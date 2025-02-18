@echo off

:: This batch file builds Jolt Physics from source code using MSVC (cl.exe) for Release.

:: Set the script's current directory
set "SCRIPT_DIR=%~dp0"

:: Set build directories relative to where the script is executed
set "BUILD_DIR=%SCRIPT_DIR%build-release"
set "INSTALL_DIR=%SCRIPT_DIR%install-release"

:: Set Jolt's actual source location (CMakeLists.txt is inside the Build folder)
set "JOLT_SOURCE=%SCRIPT_DIR%Build"

:: Ensure Visual Studio environment is set up correctly
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" || (
    echo [ERROR] Failed to set up Visual Studio environment.
    exit /b 1
)

:: Record start time
for /f "tokens=1-4 delims=:.," %%a in ("%TIME%") do set "TIME_START=%%a:%%b:%%c"

:: Create the build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%" || (
    echo [ERROR] Failed to access build directory: %BUILD_DIR%
    exit /b 1
)

:: Configure Jolt with CMake (using the Build folder for CMakeLists.txt)
echo [INFO] Configuring Jolt with CMake...
cmake -G "Ninja" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_CXX_COMPILER=cl ^
  -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDLL" ^
  -DCMAKE_CXX_FLAGS="/O2 /GL /DNDEBUG /EHsc /MT" ^
  -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
  -DBUILD_SHARED_LIBS=ON ^
  "%JOLT_SOURCE%" || (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)

:: Build Jolt with Ninja
echo [INFO] Building Jolt...
ninja -j%NUMBER_OF_PROCESSORS% || (
    echo [ERROR] Build process failed.
    exit /b 1
)

:: Install Jolt
echo [INFO] Installing Jolt...
ninja install || (
    echo [ERROR] Install process failed.
    exit /b 1
)
:: Record end time
for /f "tokens=1-4 delims=:.," %%a in ("%TIME%") do set "TIME_END=%%a:%%b:%%c"

:: Success message
echo [SUCCESS] Jolt built and installed successfully.
echo ---------------------------------------------
echo Static library: %INSTALL_DIR%\lib\jolt.lib
echo Include headers: %INSTALL_DIR%\include\jolt
echo Build duration: %TIME_START% - %TIME_END%
echo ---------------------------------------------

pause
exit /b 0
