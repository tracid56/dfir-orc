# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [10.1.0] - 2020-xx-xx

### Changed
- GetThis: reduce memory usage by a factor of 3 depending on configuration
- GetThis: major refactor to allow some future optmizations
- Log: split stdout and stderr data
- Log: use spdlog log library
- Log: dump log backtrace on critical errors
- Log: use new log levels
- Log: add utc timestamps (ISO 8601)
- OrcCommand: major refactor

### Fixed
- GetThis: segfault when hash xml element is empty #69
- GetThis: do not make hash if they are not configured #67
- GetThis: segfault with MaxSampleCount=0 and reportall/hash options #8
- Log: catch early logs in log files
- vcpkg: fix shared build
- Many other fixes

### Added
- Add '/Outline' cli option
- Add compatibility with clang and llvm tools
- Add compatibility with ninja
- Add compatibility with VS 2019 16.8

### Removed
- Remove ImportData tool
- Remove ChakraCore component
- Remove dead code

## [10.0.15] - 2020-09-28
### Added
- New 'Location' configuration keyword: '{UserProfiles}' (get profiles directories from HKLM/SOFTWARE/Microsoft/Windows NT/CurrentVersion/ProfileList)

### Fixed
- BITS: Archive were skipped when BITS server was unavailable
- CSV: Two possible csv corruptions
