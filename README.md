# PY32F0xx SGL PORT TEMPLATE

This project demonstrates the operation of sgl on the PY32F0xx platform.

## How to compile

```
# apt install gcc-arm-none-eabi
# ./build.sh
```

## Build under windows environment
### Prerequists
- Hardware
  - `JLink` 
  - `PuYa PY32F030K28U6TR Core Evaluation Board`
- Software
  - Install `CMake` 
  - Install `Ninja` 
    ``` 
    PS > choco install cmake ninja -y
    ```
  - Install [GNU Arm Embedded Toolchain](https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-win32.exe?rev=29bb46cfa0434fbda93abb33c1d480e6&hash=B2C5AAE07841929A0D0BF460896D6E52) with the default installation settings.
- PATH Environment Variable
  - JFlash.exe executable binary folder e.g: `C:\Program Files\SEGGER\JLink`.
  - arm-none-eabi-gcc cross compiler toolchain executable folder e.g: `C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin`
### Build
```
> build.bat
Memory region         Used Size  Region Size  %age Used
             RAM:        2680 B         4 KB     65.43%
           FLASH:       21380 B        32 KB     65.25%
```
### Flash with Jlink
```
# Make sure you have the correct SWD connection between your target boad and Jlink.
# Add the specific chip support to Jlink devices database.

> mkdir  %APPDATA%\SEGGER\JLinkDevices\Devices\Puya\PY32\
> copy /Y drivers\Misc\PY32F030xx_64.FLM %APPDATA%\SEGGER\JLinkDevices\Devices\Puya\PY32\
> copy /Y drivers\Misc\Devices.xml %APPDATA%\SEGGER\JLinkDevices\Devices\Puya\PY32\

# Flash into the target board.

> flash.bat
```

## Run benchmark

Flash the generated image `build/examples/demo/demo.hex` to py32 chip.

```
Render: 51fps
Render: 47fps
Render: 45fps
Render: 52fps
...
```
