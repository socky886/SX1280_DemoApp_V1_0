#change log

## Note on formating
Extracted from http://keepachangelog.com/en/0.3.0/

Date format is YYYY-MM-DD
A new entry is added for each release plus one entry on top to track unreleased change
Classification is:
- Added for new feature
- Changed for change in existing functionnality
- Deprecated for feature to be removed in next release
- Removed for feature removed by the current release
- Fixed for bug fixes
- Security for correction and/or feature improving system/user security

## [v1.3.3] 2018-09-04

### Added
 - Add StopAutoTx command (#8)
 - Add methods to enable/disable/set manual gain for LNA (#5)
 - Add YAML continuous integration script

### Changed
 - Modify the BLE connection state names to fit datahseet v2.2 (#9)
 - Modify the IRQ handler to include handling of Tx IRQ while in Rx mode for AutoTx operations (#7)
 - Rename the GFS tokens to GFSK (#4)

### Fixed
 - Fix hal write buffer offset usage
 - Fix typo in name of function SX1280HalWakeup

## [v1.3.2] 2017-10-05

### Fixed
 - Fix wrong hal size in WriteBuffer command
 - Fix other error in BLE GetPacketStatus

## [v1.3.1] 2017-09-26

### Fixed
 - Fix bug with wrong size returned by GetRxBufferStatus for BLE packet type

## [v1.3] 2017-09-18

### Added
 - Add a function to set the Access Address for BLE packets
 - Add support for DMA

### Fixed
 - Coding style

## [v1.2] 2017-08-04

### Fixed
 - Fix wrong register address for radio firmware version

## [v1.1] 2017-07-04

### Changed
 - Change name for AutoFs mode

### Removed
 - Remove two unused variables for ContinuousMode and SingleMode
 - Remove unecessary initialization of some registers at init step

### Fixed
 - Fix for coding style

## [v1.0.1] 2017-06-23

### Fixed
- Correct wrong value of SingleMode

## [v1.0] 2017-05-30

### Added
- First commit synchronized with Mbed
- Add changeLog, readme, gitignore files

### Changed


### Deprecated


### Removed


### Fixed


### Security

