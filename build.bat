@echo off
setlocal
cd /d "%~dp0"
if defined VSCMD_VER goto compile
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (echo Install Microsoft C++ Build Tools first. & exit /b 1)
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "RESULTSKIP_VS=%%i"
if not defined RESULTSKIP_VS (echo Microsoft C++ Build Tools not found. & exit /b 1)
call "%RESULTSKIP_VS%\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1
:compile
if not exist build mkdir build
if not exist dist\Mods\Plugins mkdir dist\Mods\Plugins
cl /nologo /std:c++17 /EHsc /O2 /MT /W4 /Fobuild\tests.obj tests.cpp /Fe:build\tests.exe || exit /b 1
build\tests.exe || exit /b 1
cl /nologo /std:c++17 /EHsc /O2 /MT /W4 /LD /Fobuild\ResultsSkip.obj ResultsSkip.cpp /Fe:dist\Mods\Plugins\ResultsSkip.dll /link bcrypt.lib user32.lib /IMPLIB:build\ResultsSkip.lib || exit /b 1
dumpbin /exports dist\Mods\Plugins\ResultsSkip.dll | findstr /c:" EML6_Load" >nul || (echo Missing EDF6 plugin entry point. & exit /b 1)
echo Built dist\Mods\Plugins\ResultsSkip.dll
