# ChangeLog

## [10.1.0-rc1] - 2020-11-09

### Changed
- GetThis: reduce memory usage by a factor of 3 depending on configuration
- GetThis: major refactor to allow some future optmizations
- Log: split stdout and stderr data
- Log: use spdlog log library
- Log: dump log backtrace on critical errors
- Log: use new log levels
- Log: add utc timestamps (ISO 8601)
- Improve DFIR-Orc temporary directory removal
- OrcCommand: major refactor
- README: add license section
- vcpkg: update to 2020.11-1

### Fixed
- GetThis: segfault when hash xml element is empty #69
- GetThis: do not make hash if they are not configured #67
- GetThis: segfault with MaxSampleCount=0 and reportall/hash options #8
- Log: catch early logs in log files
- vcpkg: fix shared build
- ... Check commit for a complete list of other fixes

### Added
- Outline: Add '/Outline' cli option
- Build: Add compatibility with clang and llvm tools
- Build: Add compatibility with ninja
- Build: Add compatibility with VS 2019 16.8
- Build: Add option ORC_BUILD_BOOST_STACKTRACE
- Build: Enhance tools/ci/build.ps1 script
- Add support archive hierarchies in resources
- CI: Azure: Add support for Azure Artifacts

### Removed
- Remove ImportData tool
- Remove ChakraCore component
- Remove OrcSql component
- Remove dead code

## [10.0.16] - 2020-11-09
### Added
- CI: Azure: add support for release/* branches
- SystemDetails: add 'Windows' and 'RTM' tag for pre-Win10 versions
- NtfsInfo: add SecurityDirectorySize and SecurityDirectorySignatureSize

### Fixed
- CsvFileWriter: fix memory corruption

## [10.0.15] - 2020-09-28
### Added
- New 'Location' configuration keyword: '{UserProfiles}' (get profiles directories from HKLM/SOFTWARE/Microsoft/Windows NT/CurrentVersion/ProfileList)

### Fixed
- BITS: Archive were skipped when BITS server was unavailable
- CSV: Two possible csv corruptions
