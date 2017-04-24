# Onyx
## How to build
### Requirements
Visual Studio 2013 or later must be installed
### Build process
1. Download https://github.com/MegaMilkX/Aurora/archive/master.zip and build it using build batch script inside:
2. while in project dir type into cmd "build lib > out.txt"
3. see out.txt for build result
4. Download lua https://www.lua.org/download.html and build it. Or use a prebuilt package (https://sourceforge.net/projects/luabinaries/files/5.3.3/Windows%20Libraries/Static/)
5. Download https://github.com/MegaMilkX/Onyx/archive/master.zip 
6. Check include and library directories in build.bat
```
set INCLUDE_PATHS=/I "%~dp0\..\Aurora\include" ^
/I "%~dp0\..\lib\lua\include"
set LIB_PATHS=/LIBPATH:"%~dp0\..\Aurora\lib" ^
/LIBPATH:"%~dp0\..\lib\lua\lib"
```
7. Change them according to your directory setup
8. Change liblua.lib according to your lua lib name
```
set LIBRARIES=kernel32.lib ^
user32.lib ^
gdi32.lib ^
winspool.lib ^
comdlg32.lib ^
advapi32.lib ^
shell32.lib ^
ole32.lib ^
oleaut32.lib ^
uuid.lib ^
odbc32.lib ^
odbccp32.lib ^
winmm.lib ^
Shlwapi.lib ^
legacy_stdio_definitions.lib ^
Opengl32.lib ^
aurora.lib ^
liblua.lib                                        <== HERE IT IS
```
9. Now you can run "build exe > out.txt"
10. See out.txt for any errors
  
