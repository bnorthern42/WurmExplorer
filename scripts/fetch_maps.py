#!/usr/bin/env python3
"""
scripts/fetch_maps.py - Automated Map Asset Fetcher for WurmExplorer
Downloads map images from Google Drive with large-file virus scan bypass,
extracts/organizes them into standard asset paths, and stages them for builds.
"""

import argparse
import os
import re
import shutil
import sys
import tarfile
import zipfile
from pathlib import Path

DEFAULT_DRIVE_FOLDER_ID = "0B6J_aGQ6URL8UURFN2VadWxtSWs"
DEFAULT_DEST_DIR = "assets/maps"


def extract_date_key(filename_or_path: str) -> int:
    """Extract numeric date key from filename or path for version comparison."""
    # Look for 8-digit date YYYYMMDD e.g. 20260224
    m8 = re.search(r"20\d{6}", filename_or_path)
    if m8:
        return int(m8.group(0))
    # Look for 4-digit year e.g. 2026
    m4 = re.search(r"20\d{2}", filename_or_path)
    if m4:
        return int(m4.group(0)) * 10000
    # Look for 6-digit timestamp like _170813
    m6 = re.search(r"_(\d{6})", filename_or_path)
    if m6:
        return int("20" + m6.group(1))
    return 0


def should_skip(name: str, target_year: int) -> bool:
    """
    Check if a folder or file name belongs to an archive prior to target_year.
    Returns True if a 4-digit year < target_year is found in the string.
    """
    match = re.search(r"(201\d|202\d)", name)
    if match:
        found_year = int(match.group(1))
        if found_year < target_year:
            return True
    return False


class DriveItem:
    def __init__(self, id: str, path: str, local_path: str):
        self.id = id
        self.path = path
        self.local_path = local_path


def crawl_drive_folder(
    folder_id: str,
    target_year: int,
    dest_dir: Path,
    current_rel_path: str = "",
    session=None,
    quiet: bool = False,
) -> list[DriveItem]:
    """
    Recursively crawl Google Drive folder structure, filtering out historical
    archive folders and files prior to target_year before descending.
    """
    import requests

    if session is None:
        session = requests.Session()
        session.headers.update({"User-Agent": "Mozilla/5.0"})

    url = f"https://drive.google.com/embeddedfolderview?id={folder_id}"
    try:
        resp = session.get(url, timeout=20)
        resp.raise_for_status()
    except Exception as e:
        if not quiet:
            print(f"Warning: Failed to fetch folder {folder_id} ({current_rel_path}): {e}", file=sys.stderr)
        return []

    raw_html = resp.text
    entries = []
    try:
        import bs4
        soup = bs4.BeautifulSoup(raw_html, "html.parser")
        for a in soup.find_all("a"):
            href = a.get("href", "")
            name = a.get_text(strip=True)
            if not name:
                continue
            mf = re.search(r"drive/folders/([-\w]+)", href)
            if mf:
                entries.append(("folder", mf.group(1), name))
                continue
            mfile = re.search(r"file/d/([-\w]+)", href)
            if mfile:
                entries.append(("file", mfile.group(1), name))
    except ImportError:
        for m in re.finditer(r'href="[^"]*drive/folders/([-\w]+)[^"]*"[^>]*>([^<]+)</a>', raw_html):
            entries.append(("folder", m.group(1), m.group(2).strip()))
        for m in re.finditer(r'href="[^"]*file/d/([-\w]+)[^"]*"[^>]*>([^<]+)</a>', raw_html):
            entries.append(("file", m.group(1), m.group(2).strip()))

    discovered: list[DriveItem] = []
    for item_type, item_id, name in entries:
        if should_skip(name, target_year):
            print(f"Skipping {name}")
            continue

        item_rel_path = f"{current_rel_path}/{name}" if current_rel_path else name
        if item_type == "folder":
            sub_items = crawl_drive_folder(
                folder_id=item_id,
                target_year=target_year,
                dest_dir=dest_dir,
                current_rel_path=item_rel_path,
                session=session,
                quiet=quiet,
            )
            discovered.extend(sub_items)
        else:
            local_path = dest_dir / item_rel_path
            discovered.append(DriveItem(id=item_id, path=item_rel_path, local_path=str(local_path)))

    return discovered


def download_drive_file(file_id: str, output_path: Path, quiet: bool = False) -> bool:
    """Download file from Google Drive, bypassing large-file virus scan warnings."""
    output_path.parent.mkdir(parents=True, exist_ok=True)
    try:
        import gdown
        res = gdown.download(id=file_id, output=str(output_path), quiet=quiet, resume=True)
        if res and Path(res).exists() and Path(res).stat().st_size > 0:
            return True
    except Exception:
        pass

    import requests
    session = requests.Session()
    session.headers.update({"User-Agent": "Mozilla/5.0"})
    url = "https://drive.google.com/uc"
    params = {"id": file_id, "export": "download"}
    response = session.get(url, params=params, stream=True, timeout=60)

    token = None
    for k, v in response.cookies.items():
        if k.startswith("download_warning"):
            token = v
            break
    if not token and ("confirm=" in response.text if response.encoding else False):
        m = re.search(r'confirm=([0-9A-Za-z_-]+)', response.text)
        if m:
            token = m.group(1)

    if token:
        params["confirm"] = token
        response = session.get(url, params=params, stream=True, timeout=60)

    with open(output_path, "wb") as f:
        for chunk in response.iter_content(chunk_size=65536):
            if chunk:
                f.write(chunk)
    return output_path.exists() and output_path.stat().st_size > 0


def ensure_archive_extracted(file_path: Path, dest_dir: Path) -> bool:
    """Extract file if it is a zip or tar archive."""
    if zipfile.is_zipfile(file_path):
        print(f"Extracting zip archive: {file_path} -> {dest_dir}")
        with zipfile.ZipFile(file_path, "r") as zf:
            zf.extractall(dest_dir)
        return True
    if tarfile.is_tarfile(file_path):
        print(f"Extracting tar archive: {file_path} -> {dest_dir}")
        with tarfile.open(file_path, "r:*") as tf:
            tf.extractall(dest_dir)
        return True
    return False


def link_or_copy(src: Path, dst: Path) -> None:
    """Create a relative symlink from src to dst, or copy if symlinks fail."""
    dst.parent.mkdir(parents=True, exist_ok=True)
    if dst.is_symlink() or dst.exists():
        if dst.resolve() == src.resolve():
            return
        if dst.is_file() and not dst.is_symlink() and dst.stat().st_size > 0:
            return
        dst.unlink()
    try:
        rel_path = os.path.relpath(src, dst.parent)
        dst.symlink_to(rel_path)
    except OSError:
        shutil.copy2(src, dst)


def stage_map_files(dest_dir: Path, staging_dir: Path | None = None) -> list[Path]:
    """
    Find all .png map files in dest_dir. If nested in subdirectories,
    create flat symlinks/copies at the root of dest_dir and staging_dir.
    """
    png_files = [p for p in dest_dir.rglob("*.png") if not p.is_symlink()]
    if not png_files:
        # Check if existing symlinks are present
        png_files = list(dest_dir.rglob("*.png"))

    staged = []
    for png in png_files:
        filename = png.name
        # Flat link in dest_dir if nested
        flat_dest = dest_dir / filename
        if flat_dest.resolve() != png.resolve():
            link_or_copy(png, flat_dest)
            staged.append(flat_dest)
        else:
            staged.append(png)

        # Stage to staging_dir (e.g. svrMaps) if configured
        if staging_dir:
            target_stage = staging_dir / filename
            link_or_copy(png, target_stage)

    return staged


def filter_latest_maps(files_to_download) -> list:
    """
    If multiple release dates exist for a server, filter to keep only
    the latest release version for each map type per server.
    """
    # Group by (server, map_type) e.g. ("Xanadu", "terrain")
    by_category: dict[tuple[str, str], list] = {}
    known_types = ["classic", "terrain", "topo", "topographical", "topographic", "isometric", "routes"]

    for item in files_to_download:
        p = item.path.replace("\\", "/")
        parts = p.split("/")
        server = parts[0] if parts else "Unknown"
        name_lower = Path(p).stem.lower()

        detected_type = "default"
        for kt in known_types:
            if kt in name_lower:
                detected_type = kt
                break

        key = (server, detected_type)
        by_category.setdefault(key, []).append(item)

    selected = []
    for key, items in by_category.items():
        # Sort items by date key ascending and pick the latest
        items_sorted = sorted(items, key=lambda it: extract_date_key(it.path))
        selected.append(items_sorted[-1])

    return selected


def fetch_maps(
    folder_id: str | None,
    url: str | None,
    dest: str,
    staging: str | None = None,
    strict: bool = False,
    fetch_all: bool = False,
    path_filter: str | None = None,
    quiet: bool = False,
    target_year: int = 2026,
) -> int:
    """Main map download and staging routine."""
    try:
        import requests
    except ImportError:
        try:
            import gdown
        except ImportError:
            msg = "Error: 'requests' package is required. Install via: pip install requests"
            if strict:
                print(msg, file=sys.stderr)
                return 1
            print(f"Warning: {msg}. Skipping map asset download.", file=sys.stderr)
            return 0

    dest_dir = Path(dest).resolve()
    dest_dir.mkdir(parents=True, exist_ok=True)
    staging_dir = Path(staging).resolve() if staging else None
    if staging_dir:
        staging_dir.mkdir(parents=True, exist_ok=True)

    target_id = folder_id or os.environ.get("MAP_FOLDER_ID")
    target_url = url or os.environ.get("MAP_DRIVE_URL")

    if not target_id and not target_url:
        target_id = DEFAULT_DRIVE_FOLDER_ID

    print(f"=== Fetching Map Assets ===")
    print(f"Target: {'ID ' + target_id if target_id else target_url}")
    print(f"Destination: {dest_dir}")
    print(f"Target Year: {target_year}")
    if staging_dir:
        print(f"Staging dir: {staging_dir}")

    # Determine if link points to a single file or a folder
    is_file_link = False
    if target_url and ("/file/d/" in target_url or "uc?id=" in target_url):
        is_file_link = True

    try:
        if is_file_link:
            print("Downloading map asset archive/file...")
            out_file = dest_dir / "downloaded_map_asset"
            downloaded = download_drive_file(
                file_id=target_id or "",
                output_path=out_file,
                quiet=quiet,
            )
            if not downloaded:
                raise RuntimeError("Failed to download file from Google Drive.")
            extracted = ensure_archive_extracted(out_file, dest_dir)
            if extracted and out_file.exists():
                out_file.unlink(missing_ok=True)
        else:
            # Query folder structure with historical archive filtering
            print("Retrieving remote folder structure...")
            folder_target_id = target_id
            if not folder_target_id and target_url:
                m_folder = re.search(r"folders/([-\w]+)", target_url)
                if m_folder:
                    folder_target_id = m_folder.group(1)

            drive_files = crawl_drive_folder(
                folder_id=folder_target_id,
                target_year=target_year,
                dest_dir=dest_dir,
                quiet=quiet,
            )

            if not drive_files:
                raise RuntimeError("No files found in Google Drive folder.")

            print(f"Discovered {len(drive_files)} total items in remote folder.")

            # Filter items if needed
            to_download = drive_files
            if path_filter:
                pattern = re.compile(path_filter, re.IGNORECASE)
                to_download = [f for f in to_download if pattern.search(f.path)]
                print(f"Filter '{path_filter}' matched {len(to_download)} files.")
            elif not fetch_all:
                to_download = filter_latest_maps(drive_files)
                print(f"Selected {len(to_download)} latest map assets across all servers.")

            # Download each filtered file (or resume existing)
            failed_items = []
            for idx, item in enumerate(to_download, 1):
                out_path = Path(item.local_path)
                out_path.parent.mkdir(parents=True, exist_ok=True)
                if out_path.exists() and out_path.stat().st_size > 0:
                    continue  # Already downloaded

                if not quiet:
                    print(f"[{idx}/{len(to_download)}] Downloading: {item.path}")

                try:
                    success = download_drive_file(item.id, out_path, quiet=quiet)
                    if not success:
                        raise RuntimeError("Download produced empty or missing file.")
                except Exception as file_err:
                    print(f"Warning: Failed to download {item.path}: {file_err}", file=sys.stderr)
                    failed_items.append(item.path)

            if failed_items:
                print(f"Notice: {len(failed_items)} non-essential/unreachable file(s) skipped.", file=sys.stderr)

        # Stage and organize downloaded map files
        staged_files = stage_map_files(dest_dir, staging_dir)
        print(f"Successfully processed and staged {len(staged_files)} map files.")

        if not staged_files and strict:
            print("Error: No map files were downloaded or staged!", file=sys.stderr)
            return 1

        return 0

    except Exception as exc:
        msg = f"Failed to fetch maps from Google Drive: {exc}"
        if strict:
            print(f"Error (strict mode): {msg}", file=sys.stderr)
            return 1
        print(f"Warning: {msg}. Continuing without new map downloads.", file=sys.stderr)
        # Even if network download failed, stage any existing files in dest_dir or svrMaps
        stage_map_files(dest_dir, staging_dir)
        return 0


def main():
    parser = argparse.ArgumentParser(description="Fetch Wurm Online map assets from Google Drive")
    parser.add_argument("--folder-id", "-f", default=None, help="Google Drive folder ID")
    parser.add_argument("--url", "-u", default=None, help="Google Drive public link URL")
    parser.add_argument(
        "--dest", "-d",
        default=os.environ.get("MAP_DEST_DIR", DEFAULT_DEST_DIR),
        help=f"Destination directory for maps (default: {DEFAULT_DEST_DIR})",
    )
    parser.add_argument(
        "--staging-dir", "-s",
        default="svrMaps" if Path("svrMaps").exists() else None,
        help="Optional secondary staging directory (e.g., svrMaps)",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        default=bool(os.environ.get("CI")),
        help="Enforce strict failure (exit non-zero) if download fails. Defaults to true in CI.",
    )
    parser.add_argument(
        "--all",
        action="store_true",
        default=bool(os.environ.get("MAP_FETCH_ALL")),
        help="Fetch all historical map versions (default: fetch latest release per server)",
    )
    parser.add_argument(
        "--target-year",
        type=int,
        default=int(os.environ.get("MAP_TARGET_YEAR", 2026)),
        help="Only download maps for this year or newer (default: 2026)",
    )
    parser.add_argument(
        "--filter",
        default=None,
        help="Regex pattern to filter map file paths to download",
    )
    parser.add_argument(
        "--quiet", "-q",
        action="store_true",
        help="Suppress download progress output",
    )

    args = parser.parse_args()
    rc = fetch_maps(
        folder_id=args.folder_id,
        url=args.url,
        dest=args.dest,
        staging=args.staging_dir,
        strict=args.strict,
        fetch_all=args.all,
        path_filter=args.filter,
        quiet=args.quiet,
        target_year=args.target_year,
    )
    sys.exit(rc)


if __name__ == "__main__":
    main()
