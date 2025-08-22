# Changelog

https://keepachangelog.com/en/1.1.0/

## [Changlog Section Template]

### Added
### Changed
### Deprecated
### Removed
### Fixed
### Security

## [Unreleased]

### Added

- Let user copy the name of the method.

### Changed
### Deprecated
### Removed
### Fixed

- Name of the array does not have the index.
- The one-line comment broke the js script.

### Security

## v0.3.0 - 2025-08-22

### Added

- Add template files as embeded resource.
- Add basic minification that remove whitespaces.
- Add name passing between HTML files.
- The section is removed when there is no content.

## v0.2.2 - 2025-08-21

### Added

- Add DLL API `CallDumpRValue`.
- Add DLL API `CallDumpRValueWithDefaultIndexFilename`.
- Add DLL API `CallDumpCInstance`.
- Add DLL API `CallDumpCInstanceWithDefaultIndexFilename`.
- Add an example project to show how the DLL API can be used.
- Add Resource.rc and resource.h.

### Removed

- Fully remove `EnableDumper`.
- Fully remove `DispbleDumper`.

## v0.2.1 - 2025-08-21

### Removed

- Remove `EnableDumper`.
- Remove `DispbleDumper`.

### Fixed

- Macro `DEFINE_DUMPING_HOOK_FUNCTION` was hard coded to the DLL version.

## v0.2.0 - 2025-08-19

### Added

- Add `DumpHookVariables` that automatically dumps all arguments of a hook (`Self`, `Other`,
  `Result`, `Arguments`).
- Add `RegisterHook` that abstracts away the hook creation.
- Add `RegisterHooks` that create multiple hooks with just one call.
- Add `EnableDumper`.
- Add `DispbleDumper`.

### Changed

- Add `dont_queue` parameter to `ToJsonObject`. When `true`, any struct found during the call
  will not be added to the `queue`.
- Export public interface functions to DLL. Not all functions has calling helper.
- All dumping related functions now obey `g_SHOULD_DUMP`. If it is `false`, dumping will be skipped.

### Deprecated
### Removed

- (BREAKING CHANGES) Overload of `DumpInstance`. All functions with default index filename, "index.html" now has suffix
  `WithDefaultIndexFilename`. E.g. `DumpRValueWithDefaultIndexFilename`. The `CInstance` version
  is also being explicitly named. E.g. `DumpCInstance`.

### Fixed

- The dumper does not dump the initial instance.
- The `visited_pointers` was not cleared when the `DumpInstance` is called multiple times
  during a session.

### Security

## v0.1.0 - 2025-08-18

An initial release.

### Added

- A `DumpInstance` function.
