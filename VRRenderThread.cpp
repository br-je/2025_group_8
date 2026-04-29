/**
 * @file VRRenderThread.cpp
 * @brief Implementation of the VRRenderThread class.
 *
 * Handles rendering of VTK actors in a separate thread to support
 * smooth VR performance without blocking the main GUI.
 */
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

#include <array>

//These includes have been used to scale the objects in the scene
#include <algorithm>
#include <limits>

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
    vtkNew<vtkNamedColors> colors;

    std::array<unsigned char, 4> bkg{ {26, 51, 102, 255} };
    colors->SetColor("BkgColor", bkg.data());

    vtkNew<vtkOpenVRRenderer> renderer;

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

	// Set a consistent background color for the VR scene
    renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

	// Set up the VR camera and interactor
    vtkNew<vtkOpenVRCamera> cam;
    renderer->SetActiveCamera(cam);

	// Create the VR render window and interactor, and start the rendering loop
    vtkNew<vtkOpenVRRenderWindow> renderWindow;
    renderWindow->Initialize();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetWindowName("Group8 CAD VR");

	// The interactor will handle user input and allow for real-time interaction with the VR scene
    vtkNew<vtkOpenVRRenderWindowInteractor> renderWindowInteractor;
    renderWindowInteractor->SetRenderWindow(renderWindow);
    renderWindowInteractor->Initialize();

	// Start the VR rendering loop. This will keep the VR scene responsive and allow for user interaction until the interactor is stopped.
    renderWindow->Render();
    renderWindowInteractor->Start();
}
