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


VRRenderThread::VRRenderThread(QObject* parent)
    : QThread(parent)
{
}

//This code is currently considered experimental
//Values should be changed to perfectly fit the VR scene to scale the STL properly
void VRRenderThread::addActorOffline(vtkSmartPointer<vtkActor> actor)
{
    if (!this->isRunning() && actor)
    {
        // Do not scale or centre actors individually here.
        // If each part is centred separately, the CAD assembly loses its correct layout.
        actors.append(actor);
    }
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

    renderer->AddActor(floorActor);

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

        if (vrRenderWindow)
        {
            vrRenderWindow->Render();
        }

        // Avoid maxing out the CPU.
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