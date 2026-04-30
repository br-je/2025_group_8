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
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkPNGReader.h>
#include <vtkTexture.h>
#include <vtkSphereSource.h>
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
    double vrCentreZ = 0.0;
    double vrScale = 1.0;
    bool hasVRTransform = false;

    if (hasBounds)
    {
        double centreX = (globalBounds[0] + globalBounds[1]) / 2.0;
        double centreY = (globalBounds[2] + globalBounds[3]) / 2.0;
        double centreZ = (globalBounds[4] + globalBounds[5]) / 2.0;

        double sizeX = globalBounds[1] - globalBounds[0];
        double sizeY = globalBounds[3] - globalBounds[2];
        double sizeZ = globalBounds[5] - globalBounds[4];

        double maxSize = std::max(sizeX, std::max(sizeY, sizeZ));
		// We can change the scale factor here to make the CAD assembly larger or smaller in the VR scene (currently set to 1.5)
        if (maxSize > 0.0)
        {
            double scale = 1.5 / maxSize;

            vrCentreX = centreX;
            vrCentreY = centreY;
            vrCentreZ = centreZ;
            vrScale = scale;
            hasVRTransform = true;

            for (auto actor : actors)
            {
                if (!actor)
                {
                    continue;
                }
				// Scale the actor uniformly
                actor->SetScale(scale, scale, scale);

                // Move whole assembly together, not each part separately
                actor->SetPosition(
                    -centreX * scale,
                    -centreY * scale,
                    -centreZ * scale - 2.0
                );

                // Keep the orientation correction from the example/email
                actor->RotateX(-90.0);
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

    // Add a simple floor to make the VR environment more realistic.
    // This helps avoid the scene appearing as just a plain background colour.
	// Play around with the floor position and size to best fit the CAD assembly and improve depth perception in VR.
    vtkNew<vtkPlaneSource> floorSource;

    double floorY = -1.0; // adjust height
    floorSource->SetOrigin(-3.0, floorY, -5.0);
    floorSource->SetPoint1(3.0, floorY, -5.0);
    floorSource->SetPoint2(-3.0, floorY, 5.0);
    floorSource->Update();

    vtkNew<vtkPolyDataMapper> floorMapper;
    floorMapper->SetInputConnection(floorSource->GetOutputPort());

    vtkNew<vtkActor> floorActor;
    floorActor->SetMapper(floorMapper);
    floorActor->GetProperty()->SetColor(0.35, 0.35, 0.35);
    floorActor->GetProperty()->SetOpacity(1.0);


	//Add floor to the scene before the sky to ensure it appears in front of the skybox
    renderer->AddActor(floorActor);

   // Add a simple environment sphere around the VR scene.
   // This acts as a lightweight skybox/scenery without relying on image files.
    vtkNew<vtkSphereSource> skySource;
    skySource->SetRadius(25.0);
    skySource->SetThetaResolution(64);
    skySource->SetPhiResolution(32);
    skySource->SetCenter(0.0, 0.0, -2.0);
    skySource->Update();

    vtkNew<vtkPolyDataMapper> skyMapper;
    skyMapper->SetInputConnection(skySource->GetOutputPort());

    vtkNew<vtkActor> skyActor;
    skyActor->SetMapper(skyMapper);

    // Make it look like a distant dark-blue VR environment.
    skyActor->GetProperty()->SetColor(0.04, 0.07, 0.14);
    skyActor->GetProperty()->SetAmbient(1.0);
    skyActor->GetProperty()->SetDiffuse(0.0);
    skyActor->GetProperty()->SetSpecular(0.0);

    // Prevent the sky from being selected/interacted with.
    skyActor->PickableOff();

    renderer->AddActor(skyActor);

	// Set a consistent background color for the VR scene
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
                actor->SetPosition(
                    -vrCentreX * vrScale,
                    -vrCentreY * vrScale,
                    -vrCentreZ * vrScale - 2.0
                );
                actor->RotateX(-90.0);
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