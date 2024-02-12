# EDK2 UEFI Firmware For HTC HD2

## Status 

| Function      | Notes                                            | Status |
|---------------|--------------------------------------------------|--------|
| GPIO          | Based on cLK driver                              |   ✅   |
| SD Card       | Based on cLK driver                              |   ✅   |
| NAND          | Driver exists in cLK (probably won't be added)   |   ❌   |
| I2C           | Based on cLK driver                              |   ✅   |
| Panel         | Driver exists in cLK                             |   ❌   |
| Touchscreen   | Driver exists in linux                           |   ❌   |
| Charging      | Based on cLK code                                |   ✅   |
| Battery Gauge | Supported in cLK since 1.5.x                     |   ❌   |
| USB           | Driver exists in cLK                             |   ❌   |
| Keypad        | Loosely based on cLK driver                      |   ✅   |
| Light sensor  | Based on cLK code                                |   ✅   |

## Loading
UEFI can be either chainloaded as a kernel from cLK, as well as flashed to boot directly from HSPL.

## Credits
 - Cotulla and DFT for the work on HD2
 - cedesmith for creating the lk port for Leo, kokotas on further work
 - n0d3 for the sdcard driver in cLK
 - imbushuo for creating PrimeG2Pkg
 - ivoszbg for Msm8916Pkg
 - winocm for the iPhone4Pkg
 - feherneoh for all the helpfull ideas and shared knowledge

## License
All code except drivers in `GplDrivers` directory is licensed under BSD 2-Clause. 
GPL Drivers are licensed under GPLv2 license.
