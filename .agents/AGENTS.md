# WurmExplorer AI Guidelines


## TDD Rules
- When writing a feature or fixing a bug, use Red-Green Test Driven Development (TDD).
- First, write a red (failing) test that reproduces the bug or tests the missing feature.
- Run the test to confirm it fails.
- After creating the red test, **do not touch the test file again**.
- Modify only the feature code until the test passes (turns green).

## Code Structure Rules
- Keep all files under 500 lines of code. If a file gets larger, refactor it into smaller modules.

## UI Theme & Design System Rules
- All new features and UI widgets must strictly adhere to the Dark Slate & Emerald design system.
- Colors must be sourced from `src/ui/ThemeTokens.hpp` (`treasure::ui::theme`).
- Base Backgrounds: Dark Slate (`#18181b`), Deep Charcoal Sidebar (`#121214`), Surface Dark (`#202124`), Surface Card (`#27282d`), Hover (`#32343b`).
- Accents: Vibrant Emerald Green (`#04b97f`), Mint Highlight (`#37efba`), Tinted Active (`#0d3829`), Pressed (`#02875b`).
- Borders: Subtle Slate (`#383a42`), Active/Focus Border (`#04b97f`).
- Status: Danger Red (`#ef4444`), Warning Amber (`#f59e0b`), Success Green (`#10b981`).
- Absolutely no blue, navy, or purple shades (no legacy Catppuccin Macchiato `#8aadf4`, `#313244`, `#1e1e2e`, etc.).

## Execution & Verification Rules
- **DO NOT** run `scripts/fetch_maps.py` or any Docker builds autonomously.
- Never run `scripts/fetch_maps.py` or execute `docker build` commands autonomously.
- **DEFAULT VERIFICATION:** For post-task verification, only perform a fast local compile (e.g., `ninja -C builddir`) and run `./install.sh`, unless the user explicitly requests a full Docker build or map fetch.
- Execute heavy scripts or tests only if explicitly requested in the prompt.


