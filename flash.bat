
REM === Configuration ===
set DEVICE=PY32F030X8
set INTERFACE=SWD
set SPEED=4000
set FIRMWARE=.\build\examples\demo\demo.hex

REM === Flashing Process ===
(
echo device %DEVICE%
echo if %INTERFACE%
echo speed %SPEED%
echo r
echo erase
echo loadfile %FIRMWARE%
echo r
echo g
echo exit
) | JLink.exe