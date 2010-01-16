@echo off

REM --------------------------------------------------------------
REM   "LogMX.exe" should be used to start LogMX.
REM 
REM   This file can be used if "LogMX.exe" doesn't work for you
REM   (it may happen when your JDK/JRE installation is corrupted)
REM --------------------------------------------------------------

REM ** If you want to use a specific JRE, add its absolute path here:
SET SPECIFIC_JRE_PATH=

REM ** If your parser needs additional JARs, add their path here (absolute or relative to LogMX directory):
SET LOGMX_ADDITIONAL_CLASSPATH=

REM ** If you want to use a configuration file not located in LogMX "config/" directory:
REM SET CONFIG_FILE_PATH=-Dconfig.file=\\YourRemoteHost\YourPath\logmx.properties (another simple example could be C:\LogMX\config.txt)

echo.
echo Starting LogMX...  (you should use 'LogMX.exe' instead)
echo.

REM Searching folder containing this BAT file
REM -----------------------------------------

REM -- If batch file started from its directory --
set BATCHNAME=%0
if not exist %BATCHNAME% set BATCHNAME=%0.bat
if not exist %BATCHNAME% goto SearchInPath
for %%i in (%BATCHNAME%) do set BATCHNAME=%%~dfsi
goto GetLogMXDir

:SearchInPath
REM -- If batch file started using %PATH% --
rem If PATH contains a command
for %%i in (%0) do set BATCHNAME=%%~dfs$PATH:i
if not "%BATCHNAME%"=="" goto GetLogMXDir
for %%i in (%0.bat) do set BATCHNAME=%%~dfs$PATH:i
if not "%BATCHNAME%"=="" goto GetLogMXDir
echo Unable to find LogMX directory
PAUSE
goto End

:GetLogMXDir
for %%i in (%BATCHNAME%) do set LOGMX_HOME=%%~dpsi

REM Starting LogMX
REM --------------
SET LOGMX_LIB_PATH=%LOGMX_HOME%\lib
SET LOGMX_CLASSPATH=%LOGMX_HOME%\classes;%LOGMX_HOME%\parsers\classes;%LOGMX_HOME%\managers\classes;%LOGMX_HOME%\jar\logmx.jar
SET LOGMX_CLASSPATH=%LOGMX_CLASSPATH%;%LOGMX_LIB_PATH%\xtlnf.jar;%LOGMX_LIB_PATH%\jsch.jar;%LOGMX_LIB_PATH%\activation.jar;%LOGMX_LIB_PATH%\mailapi.jar;%LOGMX_LIB_PATH%\smtp.jar;%LOGMX_LIB_PATH%\jcommon-1.0.14.jar;%LOGMX_LIB_PATH%\jfreechart-1.0.11.jar
SET LOGMX_CLASSPATH=%LOGMX_CLASSPATH%;%LOGMX_ADDITIONAL_CLASSPATH%
SET LOGMX_MAIN=com.lightysoft.logmx.LogMX
SET JVM_OPTIONS=-Xmx302m %CONFIG_FILE_PATH%
SET PATH=%SPECIFIC_JRE_PATH%;%PATH%


java %JVM_OPTIONS% -cp %LOGMX_CLASSPATH% %LOGMX_MAIN% %*

IF %ERRORLEVEL% NEQ 0 PAUSE

:End