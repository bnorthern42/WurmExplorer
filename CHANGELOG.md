# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Automated asset fetching script (`scripts/fetch_maps.py`) to download server map files from Google Drive.
- Decoupled Difficulty Presets data provider and spreadsheet-backed dataset for the Mechanics Grinder simulator.
- Character skill linking in Grinder widget for real-time player skill feedback.
- Dark Slate & Emerald design system and ThemeTokens (`treasure::ui::theme`).
- Community health, governance, and issue templates.
- AppImage packaging and Docker container build workflows.

### Changed
- Refactored Grinder parameter layout to structured `QFormLayout` preventing control truncation and layout bleeding.
- Restricted CI pipeline execution to trigger on releases, version tags, and manual dispatches only.
- Restricted autonomous agent execution directives to local compilation and installation.

### Fixed
- Fixed plateau dimension calculation (tile-to-corner mapping) in Bridge Dirt Pillar calculator.
- Corrected Bridge Dirt Pillar elevation grid cell overlap and enforced digging skill slope limits.
- Fixed map scale and zoom bugs, ensuring accurate coordinate projection.
- Purged large map files from repository history using `git-filter-repo`.
