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
- Imrpove VR Rendering function from the GUI
(Currenty the VR render button only renders a cylinder - As ive only adapted the VR Demo provided in worksheet 7)
- Create installer and GitHub release

## Setup Instructions
This project needs Qt, VTK and OpenVR installed locally.
Each machine may need to edit the paths at the top of `CMakeLists.txt`:

To build this project:
1. Install Qt and VTK
2. Update the following paths in CMakeLists.txt:

```cmake
set(OpenVR_DIR "C:/OpenVR")
set(OpenVR_INCLUDE_DIR "C:/OpenVR/headers")
set(OpenVR_LIBRARY "C:/OpenVR/lib/win64/openvr_api.lib")

set(VTK_DIR "C:/VTK9.6_MSVC2026/build")
set(CMAKE_PREFIX_PATH "C:/Qt/Qt6.10.2/6.10.2/msvc2022_64") (ONLY ADD THIS ONE IF QT ERRORS SHOW DURING BUILD)

Personally i run the project via Visual Studio using x64 Release
