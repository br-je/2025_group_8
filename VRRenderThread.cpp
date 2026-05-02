//This is the adaptation of the VRTK Demo program.
//It currently works using the "Start VR button" in the main window
#include "VRRenderThread.h"

#include <vtkActor.h>
#include <vtkOpenVRCamera.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkLight.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>

#include <array>

//These includes have been used to scale the objects in the scene
#include <algorithm>
#include <limits>

//For skybox and floor aesthetics
#include <vtkPolyDataMapper.h>
#include <vtkTexture.h>
#include <vtkSkybox.h>
#include <vtkHDRReader.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

//Live updating VR method https://doc.qt.io/qt-6/qmutexlocker.html (Experimental, but prevents crashing currently)
#include <QMutexLocker>


VRRenderThread::VRRenderThread(QObject* parent)
    : QThread(parent)
{
}

//This code is currently considered experimental
//Values should be changed to perfectly fit the VR scene to scale the STL properly
//This function allows actors to be added to the VR scene both before and during VR runtime 
//EDIT THIS FUNCTION IF CRASHES OCCUR WHEN ADDING ACTORS DURING VR RUNTIME
void VRRenderThread::addActorOffline(vtkSmartPointer<vtkActor> actor)
{
    if (!actor)
    {
        return;
    }

    if (!this->isRunning())
    {
        // Before VR starts, actors can be added directly to the starting list.
        actors.append(actor);
    }
    else
    {
        // While VR is running, queue actors so the VR thread can add them safely.
        QMutexLocker locker(&pendingActorsMutex);
        pendingActors.append(actor);
    }
}

//Animation functions
void VRRenderThread::setAnimationEnabled(bool enabled)
{
    animationEnabled = enabled;
}
bool VRRenderThread::animationIsEnabled() const
{
    return animationEnabled;
}

void VRRenderThread::resetView()
{
    // Only request the reset here.
    // The VR thread will apply the actor transforms safely inside its render loop.
    resetRequested = true;
}

//This function signals the VR render thread to stop and performs necessary cleanup of VR resources.
void VRRenderThread::stopVR()
{
    // Do not directly call OpenVR/VTK cleanup from the GUI thread.
    // The VR thread will detect this flag and terminate itself safely.
    stopRequested = true;
}

/**
This function initialises the OpenVR renderer, adds all pre-prepared actors, and continuously updates the VR scene until stopped.
 Key steps:
 - Create VR renderer and interactor
 - Add all actors generated from ModelParts
 - Compute a global bounding box to correctly scale and centre the full CAD assembly
 - Apply consistent transformations to preserve relative positioning of parts
 - Enter render loop to allow real-time interaction

 The thread runs independently from the Qt GUI to ensure responsive VR rendering.
*/
void VRRenderThread::run()
{

    stopRequested = false;

    vtkNew<vtkNamedColors> colors;

    std::array<unsigned char, 4> bkg{ {26, 51, 102, 255} };
    colors->SetColor("BkgColor", bkg.data());

    vtkNew<vtkOpenVRRenderer> renderer;

    // Add improved lighting for the VR scene.
    // A headlight keeps the model visible from the user's viewpoint.
    vtkNew<vtkLight> headLight;
    headLight->SetLightTypeToHeadlight();
    headLight->SetIntensity(0.6);
    renderer->AddLight(headLight);

    // A scene light adds stronger directional lighting and depth.
    // This follows the lighting approach suggested in the group task sheet.
    vtkNew<vtkLight> sceneLight;
    sceneLight->SetLightTypeToSceneLight();
    sceneLight->SetPosition(5.0, 5.0, 15.0);
    sceneLight->SetPositional(true);
    sceneLight->SetConeAngle(30.0);
    sceneLight->SetFocalPoint(0.0, 0.0, -2.0);
    sceneLight->SetDiffuseColor(1.0, 1.0, 1.0);
    sceneLight->SetAmbientColor(0.6, 0.6, 0.6);
    sceneLight->SetSpecularColor(1.0, 1.0, 1.0);
    sceneLight->SetIntensity(0.8);
    renderer->AddLight(sceneLight);

    for (auto actor : actors)
    {
        if (actor)
        {
            renderer->AddActor(actor);
        }
    }

    // Calculate one global bounding box for the whole CAD assembly
    double globalBounds[6] = {
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::lowest()
    };

    bool hasBounds = false;

    for (auto actor : actors)
    {
        if (!actor)
        {
            continue;
        }
        double bounds[6];
        actor->GetBounds(bounds);
        if (bounds[0] <= bounds[1] &&
            bounds[2] <= bounds[3] &&
            bounds[4] <= bounds[5])
        {
            globalBounds[0] = std::min(globalBounds[0], bounds[0]);
            globalBounds[1] = std::max(globalBounds[1], bounds[1]);
            globalBounds[2] = std::min(globalBounds[2], bounds[2]);
            globalBounds[3] = std::max(globalBounds[3], bounds[3]);
            globalBounds[4] = std::min(globalBounds[4], bounds[4]);
            globalBounds[5] = std::max(globalBounds[5], bounds[5]);

            hasBounds = true;
        }
    }

    double vrCentreX = 0.0;
    double vrCentreY = 0.0;
    double vrZMin    = 0.0;
    double vrScale   = 1.0;
    bool hasVRTransform = false;

    if (hasBounds)
    {
        double centreX = (globalBounds[0] + globalBounds[1]) / 2.0;
        double centreY = (globalBounds[2] + globalBounds[3]) / 2.0;

        double sizeX = globalBounds[1] - globalBounds[0];
        double sizeY = globalBounds[3] - globalBounds[2];
        double sizeZ = globalBounds[5] - globalBounds[4];

        // The lowest Z across the whole assembly — used to place the model
        // so its base sits exactly on the floor (world Y = 0).
        double zMin = globalBounds[4];

        double maxSize = std::max(sizeX, std::max(sizeY, sizeZ));
        if (maxSize > 0.0)
        {
            // 0.6 gives a comfortable viewing size — the longest axis of the
            // assembly becomes 0.6m, which fits fully in view at 2m distance.
            double scale = 0.6 / maxSize;

            vrCentreX = centreX;
            vrCentreY = centreY;
            vrZMin    = zMin;
            vrScale   = scale;
            hasVRTransform = true;

            for (auto actor : actors)
            {
                if (!actor)
                {
                    continue;
                }

                actor->SetScale(scale, scale, scale);

                // RotateX(-90) converts CAD Z-up to VR Y-up.
                // After rotation: object-X -> world-X,
                //                 object-Z -> world-Y,
                //                 object-Y -> world -Z.
                // X: centre on world X=0 using centreX.
                // Y: use zMin (not centreZ) so the model base lands at Y=0,
                //    not the centroid — otherwise the model is half-buried.
                // Z: use centreY so the assembly is centred in depth, then
                //    push 2m in front of the user.
                actor->RotateX(-90.0);
                actor->SetPosition(
                    -centreX * scale,
                    -zMin    * scale,
                     centreY * scale - 2.0
                );
            }
        }
    }

    // Store the initial VR transforms after scaling, positioning and orientation correction.
    // These are used by Reset View to undo animation/rotation changes.
    originalPositions.clear();
    originalScales.clear();
    originalOrientations.clear();

    for (auto actor : actors)
    {
        if (!actor)
        {
            continue;
        }

        double position[3];
        double scale[3];
        double orientation[3];

        actor->GetPosition(position);
        actor->GetScale(scale);
        actor->GetOrientation(orientation);

        originalPositions.append({ position[0], position[1], position[2] });
        originalScales.append({ scale[0], scale[1], scale[2] });
        originalOrientations.append({ orientation[0], orientation[1], orientation[2] });
    }

    // No solid floor plane needed — the HDRI garage image provides the
    // visual floor. A plain grey rectangle would block it.

    // Grid of lines sitting just above the floor under the car.
    // The car appears centred at X=0, Z=-2.0, so the grid is centred there.
    // Y=0.002 is a tiny offset above the floor to prevent z-fighting.
    {
        const double gridY      =  0.002;
        const double gridCentreZ = -2.0;
        const double halfExtent  =  1.5;   // grid reaches 1.5m either side of centre
        const double spacing     =  0.1;   // one line every 10cm

        const double xMin = -halfExtent;
        const double xMax =  halfExtent;
        const double zMin =  gridCentreZ - halfExtent;
        const double zMax =  gridCentreZ + halfExtent;

        vtkNew<vtkPolyData>  gridData;
        vtkNew<vtkPoints>    gridPoints;
        vtkNew<vtkCellArray> gridLines;

        // Lines running along Z (constant X)
        for (double x = xMin; x <= xMax + 1e-9; x += spacing)
        {
            vtkIdType p0 = gridPoints->InsertNextPoint(x, gridY, zMin);
            vtkIdType p1 = gridPoints->InsertNextPoint(x, gridY, zMax);
            vtkIdType pts[2] = { p0, p1 };
            gridLines->InsertNextCell(2, pts);
        }

        // Lines running along X (constant Z)
        for (double z = zMin; z <= zMax + 1e-9; z += spacing)
        {
            vtkIdType p0 = gridPoints->InsertNextPoint(xMin, gridY, z);
            vtkIdType p1 = gridPoints->InsertNextPoint(xMax, gridY, z);
            vtkIdType pts[2] = { p0, p1 };
            gridLines->InsertNextCell(2, pts);
        }

        gridData->SetPoints(gridPoints);
        gridData->SetLines(gridLines);

        vtkNew<vtkPolyDataMapper> gridMapper;
        gridMapper->SetInputData(gridData);

        vtkNew<vtkActor> gridActor;
        gridActor->SetMapper(gridMapper);
        gridActor->VisibilityOff();
        gridActor->PickableOff();

        renderer->AddActor(gridActor);
    }

    // Load the HDRI garage environment as a proper skybox.
    // The file is copied next to the executable by CMake at build time.
    QString hdrPath = QCoreApplication::applicationDirPath() + "/hdri_skybox/garage_4k.hdr";

    vtkNew<vtkHDRReader> hdrReader;
    hdrReader->SetFileName(hdrPath.toStdString().c_str());
    hdrReader->Update();

    vtkNew<vtkTexture> skyTexture;
    skyTexture->SetInputConnection(hdrReader->GetOutputPort());
    skyTexture->MipmapOn();
    skyTexture->InterpolateOn();

    vtkNew<vtkSkybox> skybox;
    skybox->SetTexture(skyTexture);
    skybox->SetProjection(vtkSkybox::Sphere);
    renderer->AddActor(skybox);

    // Use the HDRI to light the scene as well as the background.
    renderer->UseImageBasedLightingOn();
    renderer->SetEnvironmentTexture(skyTexture);

    // Fallback background colour in case the HDR file fails to load.
    renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

	// Set up the VR camera and interactor
    vtkNew<vtkOpenVRCamera> cam;
    renderer->SetActiveCamera(cam);

    // Create the VR render window and interactor, and start the rendering loop
    vrRenderWindow = vtkSmartPointer<vtkOpenVRRenderWindow>::New();
    vrRenderWindow->Initialize();
    vrRenderWindow->AddRenderer(renderer);
    vrRenderWindow->SetWindowName("Group8 CAD VR");

    vrInteractor = vtkSmartPointer<vtkOpenVRRenderWindowInteractor>::New();
    vrInteractor->SetRenderWindow(vrRenderWindow);
    vrInteractor->Initialize();

    // Run our own VR loop instead of using vrInteractor->Start().
    // This allows the Stop VR button to end the loop safely.
    while (!stopRequested)
    {
        if (vrInteractor)
        {
            vrInteractor->ProcessEvents();
        }

        // Add any STL actors that were loaded while VR is already running.
        // This is done inside the VR thread to avoid cross-thread VTK/OpenVR crashes.
        QList<vtkSmartPointer<vtkActor>> actorsToAdd;

        {
            QMutexLocker locker(&pendingActorsMutex);
            actorsToAdd = pendingActors;
            pendingActors.clear();
        }

        for (auto actor : actorsToAdd)
        {
            if (!actor)
            {
                continue;
            }

            // Apply the same VR transform as the original loaded assembly.
            if (hasVRTransform)
            {
                actor->SetScale(vrScale, vrScale, vrScale);
                actor->RotateX(-90.0);
                actor->SetPosition(
                    -vrCentreX * vrScale,
                    -vrZMin    * vrScale,
                     vrCentreY * vrScale - 2.0
                );
            }

            actors.append(actor);

            double position[3];
            double scale[3];
            double orientation[3];

            actor->GetPosition(position);
            actor->GetScale(scale);
            actor->GetOrientation(orientation);

            originalPositions.append({ position[0], position[1], position[2] });
            originalScales.append({ scale[0], scale[1], scale[2] });
            originalOrientations.append({ orientation[0], orientation[1], orientation[2] });

            renderer->AddActor(actor);
        }

        // Reset model actors back to their original VR transforms.
        // This is handled inside the VR thread to avoid cross-thread VTK issues.
        if (resetRequested.exchange(false))
        {
            animationEnabled = false;

            int count = std::min(
                actors.size(),
                std::min(originalPositions.size(),
                    std::min(originalScales.size(), originalOrientations.size()))
            );

            for (int i = 0; i < count; i++)
            {
                vtkActor* actor = actors[i];

                if (!actor)
                {
                    continue;
                }

                actor->SetPosition(
                    originalPositions[i][0],
                    originalPositions[i][1],
                    originalPositions[i][2]
                );

                actor->SetScale(
                    originalScales[i][0],
                    originalScales[i][1],
                    originalScales[i][2]
                );

                actor->SetOrientation(
                    originalOrientations[i][0],
                    originalOrientations[i][1],
                    originalOrientations[i][2]
                );

                actor->Modified();
            }
        }

        // Simple turntable animation for the loaded CAD model.
        // Only actors in the actors list are animated, so the floor and sky sphere stay still.
        if (animationEnabled)
        {
            for (auto actor : actors)
            {
                if (actor)
                {
                    actor->RotateY(0.6);
                    actor->Modified();
                }
            }
        }

        if (vrRenderWindow)
        {
            vrRenderWindow->Render();
        }

        QThread::msleep(10);
    }

    // Cleanup after the VR loop exits.
    if (vrInteractor)
    {
        vrInteractor->TerminateApp();
    }

    if (vrRenderWindow)
    {
        vrRenderWindow->Finalize();
    }

    vrInteractor = nullptr;
    vrRenderWindow = nullptr;

}