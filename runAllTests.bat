@echo off

set TESTPATH=%CD%\..\GeoImageViewer_VC12\bin

if not exist %TESTPATH% ( 
	echo Test path does not exist
	exit /B
)

echo ------------------- Run tests -------------------------
pushd %TESTPATH%
for /r %%i in (*Test.exe) do (
		echo --- Process test : %%~nxi ---
		SetLocal
		call %%i
		EndLocal
)
popd
echo ------------------- End of Run tests -------------------------
pause