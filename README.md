# Group 8 Repository
Pages link: https://br-je.github.io/2025_group_8/index.html
## Group Members

- Bruce Jennings (20581042)
- Member 2 (StudentID)
- Member 3 (StudentID)
- Member 4 (StudentID)

---

# Group 8 – Qt / VTK CAD Viewer with VR

This repository contains a Qt-based CAD viewer with VTK rendering and an OpenVR integration for immersive model viewing.

---

## Current Status

### GUI / Core Features

- Qt GUI fully functional
- VTK render window integrated into UI
- STL files can be loaded via File/Open or toolbar
- STL folders can be loaded recursively
- Multiple STL files can be displayed simultaneously
- Tree view reflects loaded model hierarchy
- STL part names display correctly in the tree view
- Rename functionality implemented
- Right-click context menu for model property editing
- Colour changes applied to models in GUI
- Visibility toggle implemented recursively
- Clear selection and reset view controls included

### Filters

- Shrink filter with adjustable factor
- Clip filter with axis selection, percentage-based clipping and invert option
- Filters apply recursively across the model hierarchy
- Filters work in both GUI and VR
- Live filter updates in VR implemented

### VR Integration

- VR starts from GUI button
- VR can be stopped and restarted safely from GUI
- Loaded STL models render correctly in VR
- Multiple parts supported in VR
- Model scaling and positioning corrected
- Colour changes update live in VR
- Visibility changes update live in VR, including recursively
- Shrink and clip filters update live in VR
- Controller tracking working

### VR Environment

- Floor scenery implemented for spatial reference
- Improved VR lighting added
- Simple environment sphere added to improve immersion

---
## Build Notes

This project uses:

* Qt 6.10.2 (MSVC 2022 64-bit)
* VTK 9.6
* OpenVR
* CMake
* Visual Studio (x64 Release recommended)

---

## Setup Instructions

This project requires local installation of Qt, VTK, and OpenVR.

Update the paths in `CMakeLists.txt` to match your system:

```cmake
set(OpenVR_DIR "C:/OpenVR")
set(OpenVR_INCLUDE_DIR "C:/OpenVR/headers")
set(OpenVR_LIBRARY "C:/OpenVR/lib/win64/openvr_api.lib")

set(VTK_DIR "C:/VTK9.6_MSVC2026/build")

# Only required if Qt is not detected automatically:
set(CMAKE_PREFIX_PATH "C:/Qt/Qt6.10.2/6.10.2/msvc2022_64")
```

### Build Steps

1. Open the project in Visual Studio
2. Ensure configuration is set to:

   ```text
   x64-Release
   ```

3. Build the project
4. Run the executable

---

## VR Usage

1. Launch SteamVR
2. Ensure headset and controllers are active
3. Run the application
4. Load an STL model or folder
5. Click **Start VR**

Expected behaviour:

* If no STL is loaded → test cylinder appears
* If STL is loaded → model appears in VR space
* Filters applied in GUI will be reflected when VR starts

---

## Assets

*THIS NEEDS TO BE ADDED THIS IS A PLACEHOLDER
* Skybox assets stored in `Assets/Skybox`
* Intended for VR environment enhancement
* Ensure `Assets` folder is present alongside executable when running

---

## Next Tasks

* Improve GUI controls for model interaction
* Add animation
* Add ability to move STL parts with controller
* Package application into installer
* Prepare final demo

---

## Notes

* Do not commit machine-specific paths in `CMakeLists.txt`
* Use Git branches for major features
* Ensure builds work in Release mode before committing

---
