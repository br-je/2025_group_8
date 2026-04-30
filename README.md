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

## Documentation

Doxygen documentation is generated automatically using GitHub Actions.

Documentation website:
https://br-je.github.io/2025_group_8/index.html

The documentation includes class and file descriptions for the major components of the project.

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

## VR Usage

1. Launch SteamVR.
2. Ensure headset and controllers are active.
3. Run the application.
4. Load an STL file or STL folder.
5. Click **Start VR**.
6. Use the GUI to edit colour, visibility, shrink filter, or clip filter.
7. Changes should update live in VR.
8. Click **Stop VR** to safely stop the VR session.

### Expected Behaviour

- If no STL is loaded, a test cylinder appears.
- If STL models are loaded, the CAD assembly appears in VR.
- Colour, visibility, shrink, and clip filter changes update live in VR.
- VR can be stopped and restarted without restarting the application.

---

## Key Files

- `MainWindow` – Handles the Qt GUI, STL loading, tree view, context menu, and VR controls.
- `ModelPart` – Stores CAD part data, actor properties, filters, and VR actor links.
- `ModelPartList` – Provides the tree model structure for loaded CAD parts.
- `VRRenderThread` – Manages the OpenVR render thread, VR actors, environment, lighting, and VR loop.
- `PartPropertiesDialog` – Allows editing of part name, colour, visibility, and filters.

---

## Assets

*THIS NEEDS TO BE ADDED THIS IS A PLACEHOLDER
* Skybox assets stored in `Assets/Skybox`
* Intended for VR environment enhancement
* Ensure `Assets` folder is present alongside executable when running

---

## Next Tasks

- Add simple animation for VR demonstration
- Improve reset view behaviour
- Continue Doxygen documentation polish
- Package application into an installer
- Prepare final demo workflow
- Optional: add textured skybox or enhanced VR scenery
- Optional: investigate controller-based part movement

---

## Notes

- Do not commit machine-specific paths in `CMakeLists.txt`.
- Use Git branches for major features.
- Ensure builds work in Release mode before committing.
- Avoid committing build folders, binaries, or large dependency folders.

- ---
