import unittest
import sys
import subprocess
from pathlib import Path

# Add scripts directory to path
sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "scripts"))
import fetch_maps


class TestFetchMapsFiltering(unittest.TestCase):
    def test_should_skip_historical_archives(self):
        # Must skip any folder or file from years prior to target_year
        self.assertTrue(fetch_maps.should_skip("August 2017", 2026))
        self.assertTrue(fetch_maps.should_skip("December 2017", 2026))
        self.assertTrue(fetch_maps.should_skip("Original 2020", 2026))
        self.assertTrue(fetch_maps.should_skip("February 2020", 2026))
        self.assertTrue(fetch_maps.should_skip("February 2022", 2026))
        self.assertTrue(fetch_maps.should_skip("Febuary 2022", 2026))
        self.assertTrue(fetch_maps.should_skip("January 2023", 2026))
        self.assertTrue(fetch_maps.should_skip("Elevation 2023", 2026))
        self.assertTrue(fetch_maps.should_skip("February 2024", 2026))
        self.assertTrue(fetch_maps.should_skip("February 2025", 2026))
        self.assertTrue(fetch_maps.should_skip("Pristine-classic-20240216.png", 2026))

    def test_should_not_skip_target_year(self):
        # Must NOT skip current/target year archives or files
        self.assertFalse(fetch_maps.should_skip("February 2026", 2026))
        self.assertFalse(fetch_maps.should_skip("Pristine-classic-20260224.png", 2026))
        self.assertFalse(fetch_maps.should_skip("Affliction-topo-20260224.png", 2026))

    def test_should_not_skip_base_server_folders_without_date(self):
        # Invariant: Root server folders without year in name must never be skipped
        self.assertFalse(fetch_maps.should_skip("Melody", 2026))
        self.assertFalse(fetch_maps.should_skip("Pristine", 2026))
        self.assertFalse(fetch_maps.should_skip("Affliction", 2026))
        self.assertFalse(fetch_maps.should_skip("Xanadu", 2026))

    def test_cli_target_year_argument(self):
        script_path = Path(__file__).resolve().parent.parent / "scripts" / "fetch_maps.py"
        res = subprocess.run(
            [sys.executable, str(script_path), "--help"],
            capture_output=True,
            text=True,
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("--target-year", res.stdout)


if __name__ == "__main__":
    unittest.main()
