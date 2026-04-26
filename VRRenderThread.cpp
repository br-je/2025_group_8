#include "VRRenderThread.h"

#include <vtkNew.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRCamera.h>
#include <vtkOpenVRInteractorStyle.h>
#include <vtkLight.h>

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
    vtkNew<vtkOpenVRRenderer> vrRenderer;
    vtkNew<vtkOpenVRRenderWindow> vrRenderWindow;
    vtkNew<vtkOpenVRRenderWindowInteractor> vrInteractor;
    vtkNew<vtkOpenVRCamera> vrCamera;
    vtkNew<vtkOpenVRInteractorStyle> vrStyle;

    vrRenderer->SetActiveCamera(vrCamera);
    vrRenderWindow->AddRenderer(vrRenderer);
    vrInteractor->SetRenderWindow(vrRenderWindow);
    vrInteractor->SetInteractorStyle(vrStyle);

    for (auto actor : actors)
    {
        vrRenderer->AddActor(actor);
    }

    vtkNew<vtkLight> light;
    light->SetLightTypeToSceneLight();
    light->SetPosition(5, 5, 15);
    light->SetPositional(true);
    light->SetIntensity(0.8);
    vrRenderer->AddLight(light);

    vrRenderer->ResetCamera();
    vrRenderWindow->Render();
    vrInteractor->Start();
}