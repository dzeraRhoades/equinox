@rem *******************************************************************************
@rem  Copyright (c) 2000, 2021 IBM Corporation and others.
@rem 
@rem  This program and the accompanying materials
@rem  are made available under the terms of the Eclipse Public License 2.0
@rem  which accompanies this distribution, and is available at 
@rem  https://www.eclipse.org/legal/epl-2.0/
@rem
@rem SPDX-License-Identifier: EPL-2.0
@rem  
@rem  Contributors:
@rem      IBM Corporation - initial API and implementation
@rem      Kevin Cornell (Rational Software Corporation)
@rem ********************************************************************** 
@rem 
@rem  Usage: sh build.sh [<optional switches>] [clean]
@rem 
@rem    where the optional switches are:
@rem        -output <PROGRAM_OUTPUT>  - executable filename ("eclipse")
@rem        -library <PROGRAM_LIBRARY>- dll filename (eclipse.dll)
@rem        -os     <DEFAULT_OS>      - default Eclipse "-os" value (qnx) 
@rem        -arch   <DEFAULT_OS_ARCH> - default Eclipse "-arch" value (x86) 
@rem        -ws     <DEFAULT_WS>      - default Eclipse "-ws" value (photon)
@rem		-java   <JAVA_HOME>       - location of a Java SDK for JNI headers 
@rem 
@rem 
@rem     This script can also be invoked with the "clean" argument.
@rem
@rem NOTE: The C compiler needs to be setup. This script has been
@rem       tested against Microsoft Visual C and C++ Compiler 6.0.
@rem	
@rem Uncomment the lines below and edit MSVC_HOME to point to the
@rem correct root directory of the compiler installation, if you
@rem want this to be done by this script.
@rem 
@rem ******
@echo off

IF EXIST C:\BUILD\swt-builddir set LAUNCHER_BUILDDIR=C:\BUILD\swt-builddir
IF x.%LAUNCHER_BUILDDIR%==x. set LAUNCHER_BUILDDIR=S:\swt-builddir
echo LAUNCHER build dir: %LAUNCHER_BUILDDIR%

FOR /r "%ProgramFiles(x86)%\Microsoft Visual Studio" %%v IN (*vcvarsall.bat) DO (
	set SCRIPT=%%v
	GOTO X86_64
)

GOTO X86_64

:X86_64
set defaultOSArch=x86_64
set PROCESSOR_ARCHITECTURE=AMD64
IF NOT EXIST "%JAVA_HOME%" set "JAVA_HOME=%ProgramFiles%\AdoptOpenJDK\jdk-8.0.292.10-hotspot"
IF EXIST "%JAVA_HOME%" (
	echo "JAVA_HOME x64: %JAVA_HOME%"
) ELSE (
	echo "WARNING: x64 Java JDK not found. Please set JAVA_HOME to your JDK directory."
	echo "     Refer steps for SWT Windows native setup: https://www.eclipse.org/swt/swt_win_native.php"
)
set javaHome=%JAVA_HOME%
set makefile=make_win64.mak
call "%SCRIPT%" x64
GOTO MAKE

:MAKE 
rem --------------------------
rem Define default values for environment variables used in the makefiles.
rem --------------------------
set programOutput=eclipse.exe
set programLibrary=eclipse.dll
set defaultOS=win32
set defaultWS=win32
set OS=Windows

rem --------------------------
rem Parse the command line arguments and override the default values.
rem --------------------------

:processargs
SET ARG=%1
SET ARG_VAL=%2
IF DEFINED ARG (
    IF "%ARG%"=="-os" ( set defaultOS=%ARG_VAL%
    SHIFT
    GOTO shiftcommand )
    IF "%ARG%"=="-arch" ( set defaultOSArch=%ARG_VAL%
    SHIFT 
    GOTO shiftcommand )
    IF "%ARG%"=="-rt_bin" ( set rt_binaries=%ARG_VAL%
    	call set rt_binaries_new=%%rt_binaries:/=\%%
    SHIFT
    GOTO shiftcommand )
    IF "%ARG%"=="-ws" ( set defaultWS=%ARG_VAL%
    SHIFT
    GOTO shiftcommand )
    IF "%ARG%"=="-output" ( set programOutput=%ARG_VAL%
    SHIFT
    GOTO shiftcommand )
    IF "%ARG%"=="-library" ( set programLibrary=%ARG_VAL%
    SHIFT
    GOTO shiftcommand )
    IF "%ARG%"=="-java" ( set javaHome=%ARG_VAL%
	echo %javaHome%
    SHIFT
    GOTO shiftcommand )
	IF "%ARG%"=="install" ( set INSTALL=%ARG%
    GOTO shiftcommand )
	IF "%ARG%"=="clean" ( set CLEAN=%ARG%
    GOTO shiftcommand )
)
GOTO ENDLOOP
:shiftcommand
SHIFT
GOTO processargs

:ENDLOOP

rem --------------------------
rem Set up environment variables needed by the makefile.
rem --------------------------
set PROGRAM_OUTPUT=%programOutput%
set PROGRAM_LIBRARY=%programLibrary%
set DEFAULT_OS=%defaultOS%
set DEFAULT_OS_ARCH=%defaultOSArch%
set DEFAULT_WS=%defaultWS%
set EXEC_DIR=%rt_binaries_new%\org.eclipse.equinox.executable
set LAUNCHER_DIR=%rt_binaries_new%\org.eclipse.equinox.launcher
set OUTPUT_DIR=%EXEC_DIR%\bin\%defaultWS%\%defaultOS%\%defaultOSArch%
set LAUNCHER_OUTPUT_DIR=%LAUNCHER_DIR%.%defaultWS%.%defaultOS%.%defaultOSArch%
set JAVA_HOME=%javaHome%

rem --------------------------
rem Run nmake to build the executable.
rem --------------------------

echo Building %OS% launcher. Defaults: -os %DEFAULT_OS% -arch %DEFAULT_OS_ARCH% -ws %DEFAULT_WS%

nmake -f %makefile% %INSTALL% %CLEAN%
goto DONE

:DONE
