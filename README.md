# EDK2 UEFI Firmware For HTC HD2
Attempt to create a normal EDK2 for HTC HD2.


# QUIRKS
This branch is mainly for Htc Desire Z but also adds support for MSM7x30 to the projecr

## Status 

| Function      | Notes                                            |  VISION
|---------------|--------------------------------------------------|--------|
| GPIO          | Based on cLK driver (only sysgpios not pmic ones)|   ✅   |
| SD Card       | Based on cLK driver                              |   ❌   |
| EMMC          | Driver exists in LK                              |   ❌   |
| I2C           | Based on cLK driver                              |   ?    |
| Panel         | Driver exists in Linux kernel                    |   ❌   |
| Touchscreen   | Driver exists in linux                           |   ❌   |
| Charging      | Based on cLK code                                |   ❌   |
| Battery Gauge | Supported in cLK since 1.5.x                     |   ❌   |
| USB           | Driver exists in cLK                             |   ❌   |
| Keypad        | Loosely based on cLK driver (only power button)  |   ✅   |
| SSBI          | Baed on linux kernel driver                      |   ?   |
| PMIC          | Driver exists in Linux depends on SSBI driver    |   ❌   |


## To-Do
- Port the SSBI and PMIC8058 Drivers from Linux
- Port the ACPUCLOCK_7x30 Driver from Linux
- Get the stupid qwerty keyboard to work

## Credits
 - sonic011gamer for creating msm8909Pkg
 - imbushuo for creating PrimeG2Pkg and also for the framebuffer patch
 - winocm for the iPhone4Pkg

