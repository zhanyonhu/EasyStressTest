@echo off

cd %~dp0

if /i "%1"=="help" goto help
if /i "%1"=="--help" goto help
if /i "%1"=="-help" goto help
if /i "%1"=="/help" goto help
if /i "%1"=="?" goto help
if /i "%1"=="-?" goto help
if /i "%1"=="--?" goto help
if /i "%1"=="/?" goto help

@rem Process arguments.
set config=
set target=Build
set noprojgen=
set nobuild=
set run=
set target_arch=ia32
set vs_toolset=x86
set platform=WIN32
set library=executable

:next-arg
if "%1"=="" goto args-done
if /i "%1"=="debug"        set config=Debug&goto arg-ok
if /i "%1"=="release"      set config=Release&goto arg-ok
if /i "%1"=="test"         set run=run-tests.exe&goto arg-ok
if /i "%1"=="bench"        set run=run-benchmarks.exe&goto arg-ok
if /i "%1"=="clean"        set target=Clean&goto arg-ok
if /i "%1"=="noprojgen"    set noprojgen=1&goto arg-ok
if /i "%1"=="nobuild"      set nobuild=1&goto arg-ok
if /i "%1"=="x86"          set target_arch=ia32&set platform=WIN32&set vs_toolset=x86&goto arg-ok
if /i "%1"=="ia32"         set target_arch=ia32&set platform=WIN32&set vs_toolset=x86&goto arg-ok
if /i "%1"=="x64"          set target_arch=x64&set platform=x64&set vs_toolset=x64&goto arg-ok
if /i "%1"=="shared"       set library=shared_library&goto arg-ok
if /i "%1"=="static"       set library=static_library&goto arg-ok
if /i "%1"=="update"       goto arg-update
:arg-ok
shift
goto next-arg
:args-done

if defined WindowsSDKDir goto select-target
if defined VCINSTALLDIR goto select-target

@rem Look for Visual Studio 2013
if not defined VS120COMNTOOLS goto vc-set-2012
if not exist "%VS120COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2012
call "%VS120COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2013
goto select-target

:vc-set-2012
@rem Look for Visual Studio 2012
if not defined VS110COMNTOOLS goto vc-set-2010
if not exist "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2010
call "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2012
goto select-target

:vc-set-2010
@rem Look for Visual Studio 2010
if not defined VS100COMNTOOLS goto vc-set-2008
if not exist "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2008
call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2010
goto select-target

:vc-set-2008
@rem Look for Visual Studio 2008
if not defined VS90COMNTOOLS goto vc-set-notfound
if not exist "%VS90COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-notfound
call "%VS90COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2008
goto select-target

:vc-set-notfound
echo Warning: Visual Studio not found

:select-target
if not "%config%"=="" goto project-tools-download
if "%run%"=="run-tests.exe" set config=Debug& goto project-tools-download
if "%run%"=="run-benchmarks.exe" set config=Release& goto project-tools-download
set config=Debug

:project-tools-download
if exist third\tcc goto project-gen
echo download tinyCC tools.
wget "http://download.savannah.gnu.org/releases/tinycc/tcc-0.9.26-win32-bin.zip" -Othird\tcc.zip
cd third
7z.exe x tcc.zip
del tcc.zip
cd %~dp0

:project-gen
@rem Skip project generation if requested.
if defined noprojgen goto msbuild

@rem Build libuv.
if exist third\libuv goto have_libuv
echo git clone https://github.com/joyent/libuv.git third/libuv
git clone https://github.com/joyent/libuv.git third/libuv
if errorlevel 1 goto libuv_install_failed
goto have_libuv

:libuv_install_failed
echo Failed to download libuv. Make sure you have git installed, or
echo manually install libuv into %~dp0third/libuv.
exit /b 1

:have_libuv
cd %~dp0third/libuv
call "%~dp0third\libuv\vcbuild.bat" %*
cd %~dp0

if not defined PYTHON set PYTHON=python
"%PYTHON%" gyp_stresstest.py -Dtarget_arch=%target_arch% -Dstresstest_library=%library%
if errorlevel 1 goto create-msvs-files-failed
if not exist stresstest.sln goto create-msvs-files-failed
echo Project files generated.

:msbuild
@rem Skip project generation if requested.
if defined nobuild goto run

@rem Check if VS build env is available
if defined VCINSTALLDIR goto msbuild-found
if defined WindowsSDKDir goto msbuild-found
echo Build skipped. To build, this file needs to run from VS cmd prompt.
goto run

@rem Build the sln with msbuild.
:msbuild-found
msbuild stresstest.sln /t:%target% /p:Configuration=%config% /p:Platform="%platform%" /clp:NoSummary;NoItemAndPropertyList;Verbosity=minimal /nologo
if errorlevel 1 exit /b 1

:run
@rem Run tests if requested.
if "%run%"=="" goto exit
if not exist %config%\%run% goto exit
echo running '%config%\%run%'
%config%\%run%
goto exit

:create-msvs-files-failed
echo Failed to create vc project files.
exit /b 1

:arg-update
@rem update source by git.
echo 'git pull easystresstest'
git pull origin master
if exist third/libuv (
	cd third/libuv 
	echo 'git pull libuv'
	git pull origin master
	if exist build/gyp	(
		cd build/gyp
		echo 'git pull gyp'
		git pull origin master
		cd ../../
	)
	cd ../../
)
goto exit

:help
echo vcbuild.bat [debug/release] [test/bench] [clean] [noprojgen] [nobuild] [x86/x64] [static/shared]
echo Examples:
echo   vcbuild.bat              : builds debug build
echo   vcbuild.bat test         : builds debug build and runs tests
echo   vcbuild.bat release bench: builds release build and runs benchmarks
goto exit

:exit

