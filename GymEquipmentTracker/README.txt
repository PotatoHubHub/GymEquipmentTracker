OPERATING SYSTEM: Windows 11
Compiler: Visual Studio
C++ 23
GUI from: FLTK v1.4.5

Build/run steps:
1: Open visual studio
2: Install VCPKG
 - open the developer powershell in your project 
 - type vcpkg integrate install
 - Go to Properties > VCPKG > Enable Manifest > Select Yes
3: Install FLTK
 - type vcpkg new --application
 - type vcpkg add port fltk
4: Run code

Requires vcpkg, cmake, and FLTK
