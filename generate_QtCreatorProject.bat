@echo off

:: Build options : build_mode={Debug, Release}, kit={VC9, VC12}, BUILD_ARCH={x64}
set BUILD_MODE=Release
set KIT=VC12
set BUILD_ARCH=x64

:: ***********************************************************************
:: Create the main parent directory
:: ***********************************************************************
set project_name=GeoImageViewer
set install_dir=../%project_name%_%KIT%
set SourceDir=%project_name%_source
cd ..
mkdir %project_name%_Build_%KIT%_%BUILD_MODE%
cd %project_name%_build_%KIT%_%BUILD_MODE%


:: ***********************************************************************
:: Setup dependencies
:: ***********************************************************************
set QTCREATOR=C:\Qt\Qt5.3.2\Tools\QtCreator\bin\qtcreator

if "%KIT%"=="VC12" (
	set QT_DIR=C:\Qt\Qt5.3.2\5.3\msvc2013_64_opengl
	set OPENCV_DIR=C:\VFomin_folder\PISE_project\Storm_app\Dependencies\Win.VC12.x64\OpenCV
	set GDAL_DIR=C:\VFomin_folder\PISE_project\Pise_dependencies_compile\gdal_1.11.1_build_VC12x64
) 

if "%KIT%"=="VC9" (
	set QT_DIR=C:\VFomin_folder\PISE_project\PiseNoOTB\Dependencies\Win.VC9.x64\Qt
	set OPENCV_DIR=C:\VFomin_folder\PISE_project\Pise_dependencies_compile\opencv_2.4.6_with_3rdparty
	set GDAL_DIR=C:\VFomin_folder\PISE_project\Pise_dependencies_compile\gdal_1.10.1_build_release
)


rem ***********************************************************************
rem CMake the application module
rem ***********************************************************************

:: -------------BUILD OPTIONS---------------------------------------
::set build_options=

:: -------------LIB INCS---------------------------------------

:: ----------------------------------------------------
set CMAKE_PARAMETERS=^
-DCMAKE_INSTALL_PREFIX=%install_dir%
::%build_options%
:: ----------------------------------------------------
:: %CMAKE% %CMAKE_PARAMETERS% -G %CMAKE_GENERATOR% ../%SourceDir%


:: Put this as arguments: -DCMAKE_INSTALL_PREFIX=..\Colormaps -DCMAKE_BUILD_TYPE=Release -DOpenCV_DIR=$ENV{OPENCV_DIR}
%QTCREATOR% ../%SourceDir%/CMakeLists.txt

PAUSE








