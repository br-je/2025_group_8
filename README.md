# Group 8 Repository

## Group Members

- Bruce (20581042)
- Simon (20672184)
- James (20723145)
- Priya (20552722)

---

# Group 8 – Qt / VTK CAD Viewer with VR

This repository contains a Qt-based CAD viewer with VTK rendering and OpenVR integration for immersive CAD model viewing.

The application allows STL CAD models to be loaded, organised in a tree structure, edited through the GUI, and viewed both in the desktop VTK render window and in VR through OpenVR.

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
- Selected items can be removed from the model tree
- Clear selection and reset view controls included
- Application/window icon added
- Bottom control buttons visually improved with icons
- GUI controls provided for starting/stopping VR, resetting the view, starting animation, and triggering explode view

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
- STL models can be added to the VR scene while VR is running
- Reset view control works for GUI and VR model view
- Start/stop animation control implemented
- Explode view implemented for VR
- Controller tracking working
- Stable VR render thread used to separate VR rendering from the Qt GUI thread

### VR Environment

- Floor scenery implemented for spatial reference
- Improved VR lighting added
- Textured warehouse-style VR environment added using an environment sphere
- Environment assets are stored in the `Assets/Skybox` folder
- Environment assets are copied into the build output folder by CMake

---

## Installer / Release

A standalone Windows installer is provided through the GitHub Releases page.

Download the latest installer from:

https://github.com/br-je/2025_group_8/releases

The installer includes the runtime files required to run the application, including:

- Qt runtime DLLs and plugins
- VTK runtime DLLs
- OpenVR runtime DLL
- VR bindings
- Required asset folders
- The `Group8_CAD_VR.exe` application

This means the installed application should run without requiring Qt, VTK, or OpenVR to be added to the system PATH.

### Installing

1. Download the latest installer from the Releases page.
2. Run the installer.
3. Launch **Group8 CAD VR** from the installed location or Start Menu shortcut.
4. Launch SteamVR before starting VR mode in the application.

### Notes

- The installer is configured as a per-user installer.
- It is designed to install without requiring administrator privileges.
- The installer does not modify the system PATH.
- SteamVR must still be installed and running for VR mode.
- The installer is intended for running the application, not for building or modifying the source code.
- Developers who want to build the project from source still need local installations of Qt, VTK, OpenVR, CMake, and Visual Studio.

## Build Notes

These build notes are only required if building the project from source. Users who only want to run the application should use the installer from the Releases page.

This project uses:

- Qt 6.10.2 MSVC 2022 64-bit
- VTK 9.6
- OpenVR
- CMake
- Visual Studio
- x64 Release configuration recommended

---

## Setup Instructions

This project requires local installation of Qt, VTK, and OpenVR.

Update the paths in `CMakeLists.txt` to match your system:

    set(OpenVR_DIR "C:/OpenVR")
    set(OpenVR_INCLUDE_DIR "C:/OpenVR/headers")
    set(OpenVR_LIBRARY "C:/OpenVR/lib/win64/openvr_api.lib")

    set(VTK_DIR "C:/VTK9.6_MSVC2026/build")

    # Only required if Qt is not detected automatically:
    set(CMAKE_PREFIX_PATH "C:/Qt/Qt6.10.2/6.10.2/msvc2022_64")
   
### Build Steps

1. Open the project in Visual Studio.
2. Configure the project with CMake.
3. Ensure the configuration is set to:

       x64-Release

4. Build the project.
5. Run the executable.

---


## VR Usage

1. Launch SteamVR.
2. Ensure headset and controllers are active.
3. Run the application.
4. Load an STL file or STL folder.
5. Click **Start VR**.
6. Use the GUI to edit colour, visibility, shrink filter, or clip filter.
7. Changes should update live in VR.
8. Additional STL models can be loaded while VR is already running.
9. Use **Start Animation** / **Stop Animation** to rotate the loaded model in VR.
10. Use **Explode (VR)** to separate parts outward from the assembly centre.
11. Use **Reset View** to reset the GUI and VR model view.
12. Click **Stop VR** to safely stop the VR session.

### Expected Behaviour

- If no STL is loaded, a test cylinder appears.
- If STL models are loaded, the CAD assembly appears in VR.
- Colour, visibility, shrink, and clip filter changes update live in VR.
- New STL files can be added to the VR scene while VR is running.
- The model animation can be started and stopped from the GUI.
- The explode view can be triggered from the GUI.
- The view can be reset from the GUI.
- VR can be stopped and restarted without restarting the application.
- A textured warehouse-style environment appears around the VR scene.

---

## Key Files

- `MainWindow` – Handles the Qt GUI, STL loading, tree view, context menu, model removal, reset view, animation controls, explode controls, and VR controls.
- `ModelPart` – Stores CAD part data, actor properties, filters, visibility state, and VR actor links.
- `ModelPartList` – Provides the tree model structure for loaded CAD parts.
- `VRRenderThread` – Manages the OpenVR render thread, VR actors, live VR updates, environment, lighting, animation, explode view, and VR loop.
- `PartPropertiesDialog` – Allows editing of part name, colour, visibility, filters, and item removal.
- `CMakeLists.txt` – Configures the Qt/VTK/OpenVR build, asset copying, and installer packaging setup.
- `icons.qrc` – Stores Qt resource paths for application and GUI icons.
- `Assets/Skybox` – Stores the environment texture used by the VR scene.

---

## Assets

The VR environment includes the following assets stored in the `Assets/Skybox` folder:

- A warehouse-style panorama image used as the VR environment sphere texture
- The image is copied into the build output folder automatically by CMake

The floor plane and scene lighting are generated in code and do not require external assets.

---

## Notes

- Do not commit machine-specific paths in `CMakeLists.txt`
- Use branches for all changes
- Ensure Release build works before merging
- Avoid modifying VRRenderThread on main without VR testing
- Avoid committing build folders, binaries, or large dependency folders
- Commit IDs and final installer releases should be clearly recorded for submission
