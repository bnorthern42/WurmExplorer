# WurmTreasureHacks AI Guidelines

## TDD Rules
- When writing a feature or fixing a bug, use Red-Green Test Driven Development (TDD).
- First, write a red (failing) test that reproduces the bug or tests the missing feature.
- Run the test to confirm it fails.
- After creating the red test, **do not touch the test file again**.
- Modify only the feature code until the test passes (turns green).

## Code Structure Rules
- Keep all files under 500 lines of code. If a file gets larger, refactor it into smaller modules.
