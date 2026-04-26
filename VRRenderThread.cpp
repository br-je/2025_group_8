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

VRRenderThread::VRRenderThread(QObject* parent)
    : QThread(parent)
{
}

void VRRenderThread::addActorOffline(vtkSmartPointer<vtkActor> actor)
{
    actors.append(actor);
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