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
        double bounds[6];
        actor->GetBounds(bounds);

        double centreX = (bounds[0] + bounds[1]) / 2.0;
        double centreY = (bounds[2] + bounds[3]) / 2.0;
        double centreZ = (bounds[4] + bounds[5]) / 2.0;

        double sizeX = bounds[1] - bounds[0];
        double sizeY = bounds[3] - bounds[2];
        double sizeZ = bounds[5] - bounds[4];

        double maxSize = std::max(sizeX, std::max(sizeY, sizeZ));

        if (maxSize > 0.0)
        {
            double scale = 1.5 / maxSize;

            actor->SetScale(scale, scale, scale);

            // Centre the model and place it roughly 2 metres in front of the user.
            actor->SetPosition(
                -centreX * scale,
                -centreY * scale,
                -centreZ * scale - 2.0
            );

            // Match the orientation used in the provided VRRenderThread example.
            actor->RotateX(-90.0);
        }

        actors.append(actor);
    }
}

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

    renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

    vtkNew<vtkOpenVRCamera> cam;
    renderer->SetActiveCamera(cam);

    vtkNew<vtkOpenVRRenderWindow> renderWindow;
    renderWindow->Initialize();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetWindowName("Group8 CAD VR");

    vtkNew<vtkOpenVRRenderWindowInteractor> renderWindowInteractor;
    renderWindowInteractor->SetRenderWindow(renderWindow);
    renderWindowInteractor->Initialize();

    renderWindow->Render();
    renderWindowInteractor->Start();
}