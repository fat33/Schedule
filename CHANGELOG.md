# Changelog

## 1.1.0

Rewrite of the 1.0 code base.

### Fixed
- Saving lost cell positions: only non-empty cells were written into a flat array, so every
  empty cell shifted the following lessons after reopening the file.
- Removing a group or a subject could crash the application: map entries were erased while
  iterating, and column removal signals were emitted after the data had already changed.
- The Russian translation was never loaded (wrong resource path).
- Window geometry was restored as a zero-sized rectangle on the first start.
- Duplicate detection used substring matching ("Math" blocked adding "Mathematics").
- Combo box editor did not preselect the current value.
- User text was inserted into the printed HTML without escaping.
- Memory leaks in printing and in the header view.

### Changed
- Build system moved from qmake to CMake; Qt 6 and Qt 5.15 are supported.
- New versioned JSON format (`*.json`); files from 1.0 (`*.conf`) can still be opened.
- Files are saved atomically with `QSaveFile`; unsaved changes are tracked.
- The bell schedule lives in one place (`timetable.h`) instead of being duplicated.
- Windows-only OS/compiler detection code (WinAPI, VBScript) replaced with `QSysInfo`.
- Unused code removed (`BlockWidget`, `TRichHeader`, commented-out code).
- Icons come from the system theme / Qt style, no missing resource files.

### Added
- Unit tests (Qt Test + `QAbstractItemModelTester`), GitHub Actions CI for Linux (Qt 5 and 6)
  and Windows with a ready-to-run build artifact.
- Command-line argument to open a file, "About Qt", keyboard shortcuts.
