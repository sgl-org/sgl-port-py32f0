@echo off

REM
REM  MIT License
REM 
REM  Copyright(c) 2023-present All contributors of SGL
REM  Document reference link: docs directory
REM 
REM  Permission is hereby granted, free of charge, to any person obtaining a copy
REM  of this software and associated documentation files (the "Software"), to deal
REM  in the Software without restriction, including without limitation the rights
REM  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
REM  copies of the Software, and to permit persons to whom the Software is
REM  furnished to do so, subject to the following conditions:
REM  The above copyright notice and this permission notice shall be included in all
REM  copies or substantial portions of the Software.
REM  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
REM  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
REM  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
REM  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
REM  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
REM  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
REM  SOFTWARE.
REM

setlocal

REM Collect options
set "options=-DENABLE_EXAMPLES=ON"
set "options=%options% -DCMAKE_BUILD_TYPE=MinSizeRel"
set "options=%options% -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
set "options=%options% -G Ninja"
set "options=%options% --toolchain cmake/toolchain-windows.cmake"

REM Run CMake configure
cmake -B build %options%

REM Get number of processors for parallel build
set "nproc=%NUMBER_OF_PROCESSORS%"

REM Run CMake build
cmake --build build -j%nproc%

endlocal
