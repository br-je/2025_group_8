\# Group 8 Repository



\## Group Members

\- Bruce Jennings (20581042)

\- Member 2 (StudentID)

\- Member 3 (StudentID)

\- Member 4 (StudentID)


This repository contains the group Qt/VTK CAD viewer and VR demo project.

## Current status

- Qt GUI opens successfully
- VTK render window works
- Default cylinder renders on startup
- STL files can be loaded through File/Open or the toolbar folder icon
- Multiple STL files can be displayed together
- Tree view is connected to loaded model parts

## Build notes

This project is built using CMake, Qt, VTK and Visual Studio in Release x64 mode.

## Next tasks

- Add right-click context menu
- Add colour, visibility and rename editing
- Add shrink and clip filters
- Add VRRenderThread and start VR from the GUI
- Create installer and GitHub release

## Setup Instructions

To build this project:

1. Install Qt and VTK
2. Update the following paths in CMakeLists.txt:

```cmake
set(OpenVR_DIR "C:/OpenVR")
set(OpenVR_INCLUDE_DIR "C:/OpenVR/headers")
set(OpenVR_LIBRARY "C:/OpenVR/lib/win64/openvr_api.lib")
set(VTK_DIR "C:/VTK9.6_MSVC2026/build")

Personally i run the project via Visual Studio using x64 Release
