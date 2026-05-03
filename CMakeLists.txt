cmake_minimum_required(VERSION 3.16)

project(Group8_CAD_VR VERSION 0.1 LANGUAGES CXX)

# ---- Local dependency paths ----
# You need to change these to your appropriate locations if copying in order to build properly
set(OpenVR_DIR "C:/OpenVR")
set(OpenVR_INCLUDE_DIR "C:/OpenVR/headers")
set(OpenVR_LIBRARY "C:/OpenVR/lib/win64/openvr_api.lib")

set(VTK_DIR "C:/VTK9.6_MSVC2026/build")

find_package(VTK REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
	ModelPart.cpp
	ModelPart.h
	ModelPartList.cpp
	ModelPartList.h
	icons.qrc
	PartPropertiesDialog.cpp
	PartPropertiesDialog.h
	PartPropertiesDialog.ui
    VRRenderThread.cpp
    VRRenderThread.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Group8_CAD_VR
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        icons.qrc
        partpropertiesdialog.h partpropertiesdialog.cpp partpropertiesdialog.ui
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Group8_CAD_VR APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Group8_CAD_VR SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Group8_CAD_VR
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Group8_CAD_VR PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${VTK_LIBRARIES})

vtk_module_autoinit(
    TARGETS Group8_CAD_VR
    MODULES ${VTK_LIBRARIES}
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Group8_CAD_VR)
endif()
set_target_properties(Group8_CAD_VR PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Group8_CAD_VR
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Group8_CAD_VR)
endif()

add_custom_target(VRBindings ALL)

add_custom_command(TARGET VRBindings PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/vrbindings
    ${CMAKE_BINARY_DIR}/vrbindings
)

add_custom_command(TARGET Group8_CAD_VR POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/vrbindings/vtk_openvr_actions.json
        $<TARGET_FILE_DIR:Group8_CAD_VR>/vtk_openvr_actions.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/vrbindings/vtk_openvr_binding_oculus_touch.json
        $<TARGET_FILE_DIR:Group8_CAD_VR>/vtk_openvr_binding_oculus_touch.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/vrbindings/vtk_openvr_binding_vive_controller.json
        $<TARGET_FILE_DIR:Group8_CAD_VR>/vtk_openvr_binding_vive_controller.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/vrbindings/vtk_openvr_binding_knuckles.json
        $<TARGET_FILE_DIR:Group8_CAD_VR>/vtk_openvr_binding_knuckles.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/vrbindings/vtk_openvr_binding_hpmotioncontroller.json
        $<TARGET_FILE_DIR:Group8_CAD_VR>/vtk_openvr_binding_hpmotioncontroller.json
)

add_custom_command(TARGET Group8_CAD_VR POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Assets
    $<TARGET_FILE_DIR:Group8_CAD_VR>/Assets
)

# ------------------------------------------------------------
# Installer / CPack setup
# ------------------------------------------------------------

include(InstallRequiredSystemLibraries)

# Local deploy folder containing the tested standalone runtime package.
# This folder should contain:
# - Group8_CAD_VR.exe
# - Qt DLLs and plugin folders
# - VTK DLLs
# - openvr_api.dll
# - vrbindings folder
#
# This path is intentionally configurable so machine-specific deploy paths
# do not need to be committed permanently.
set(GROUP8_DEPLOY_DIR "" CACHE PATH "Path to tested deploy folder for installer packaging")

if(GROUP8_DEPLOY_DIR)
    install(
        DIRECTORY "${GROUP8_DEPLOY_DIR}/"
        DESTINATION "."
    )
else()
    message(WARNING "GROUP8_DEPLOY_DIR is not set. Installer will not include deploy folder contents.")
endif()

set(CPACK_PACKAGE_NAME "Group8 CAD VR")
set(CPACK_PACKAGE_VENDOR "University of Nottingham Group 8")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Qt, VTK and OpenVR CAD viewer")
set(CPACK_PACKAGE_VERSION "1.0.0")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")

    # Make the installer per-user rather than system-wide.
    # This avoids requiring administrator privileges during the demo.
    set(CPACK_NSIS_INSTALL_ROOT "$LOCALAPPDATA")

    # Do not ask to modify the system PATH.
    # Modifying PATH can trigger admin requirements.
    set(CPACK_NSIS_MODIFY_PATH OFF)

    # Request normal user-level execution instead of admin elevation.
    set(CPACK_NSIS_DEFINES "RequestExecutionLevel user")
else()
    set(CPACK_GENERATOR "ZIP")
endif()

set(CPACK_PACKAGE_INSTALL_DIRECTORY "Group8 CAD VR")

# NSIS Start Menu shortcut settings
set(CPACK_NSIS_DISPLAY_NAME "Group8 CAD VR")
set(CPACK_NSIS_PACKAGE_NAME "Group8 CAD VR")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Group8 CAD VR.lnk' '$INSTDIR\\\\Group8_CAD_VR.exe'"
)

set(CPACK_NSIS_DELETE_ICONS_EXTRA
    "Delete '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Group8 CAD VR.lnk'"
)

include(CPack)