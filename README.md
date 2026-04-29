# Group 8 Repository

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

* Qt GUI fully functional
* VTK render window integrated into UI
* STL files can be loaded via File/Open or toolbar
* STL folders can be loaded recursively
* Multiple STL files can be displayed simultaneously
* Tree view reflects loaded model hierarchy
* Rename functionality implemented
* Right-click context menu (properties editing)
* Colour changes applied to models in GUI
* Visibility toggle implemented recursively

* **Filters implemented in GUI:**

  * Shrink filter with adjustable factor
  * Clip filter with axis selection, percentage-based clipping and invert option
  * Filters apply recursively across model hierarchy

* **VR integration working:**

  * Start VR from GUI button
  * Loaded STL models render in VR
  * Multiple parts supported in VR
  * Model scaling and positioning corrected
  * Colour changes carry into VR
  * Controller tracking working

* **Filters in VR:**

  * Shrink filter supported
  * Clip filter supported
  * Filters applied when VR starts

* **VR environment improvements:**

  * Floor scenery implemented for spatial reference

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

* Improve VR lighting
* Add enhanced environment such as skybox or dome
* Implement live updates in VR
* Improve GUI controls for model interaction
* Package application into installer
* Prepare final demo

---

## Notes

* Do not commit machine-specific paths in `CMakeLists.txt`
* Use Git branches for major features
* Ensure builds work in Release mode before committing

---
test
