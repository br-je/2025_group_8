#ifndef VRRENDERTHREAD_H
#define VRRENDERTHREAD_H

#include <QThread>
#include <QList>

#include <vtkSmartPointer.h>
#include <vtkActor.h>

//StopVR
#include <atomic>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRRenderWindow.h>

#include <array>

//Experimental adding STL during live VR
#include <QMutex>

#include <vtkEventData.h>

#include "ModelPart.h"


class VRRenderThread : public QThread
{
    Q_OBJECT

public:
    explicit VRRenderThread(QObject* parent = nullptr);

    void addActorOffline(vtkSmartPointer<vtkActor> actor);
    void queueVRPipelineUpdate(ModelPart* part);
    void stopVR();
    void resetView();

	//Animation control
    void setAnimationEnabled(bool enabled);
    bool animationIsEnabled() const;


protected:
    void run() override;

private:
    QList<vtkSmartPointer<vtkActor>> actors;
    QList<vtkSmartPointer<vtkActor>> pendingActors;
    QMutex pendingActorsMutex;

    QList<ModelPart*> pendingVRUpdates;
    QMutex pendingVRUpdatesMutex;

    std::atomic<bool> stopRequested{ false };
    std::atomic<bool> resetRequested{ false };
    std::atomic<bool> animationEnabled{ false };

    QList<std::array<double, 3>> originalPositions;
    QList<std::array<double, 3>> originalScales;
    QList<std::array<double, 3>> originalOrientations;


    vtkSmartPointer<vtkOpenVRRenderWindowInteractor> vrInteractor;
    vtkSmartPointer<vtkOpenVRRenderWindow> vrRenderWindow;

    // Controller dragging state
    vtkSmartPointer<vtkActor> draggedActor;
    double lastControllerPos[3] = { 0.0, 0.0, 0.0 };
    vtkEventDataDevice grabbingDevice = vtkEventDataDevice::Unknown;

    // Static callbacks fired by the VR interactor on the VR thread
    static void OnButton3D(vtkObject* caller, unsigned long eventId, void* clientData, void* callData);
    static void OnMove3D(vtkObject* caller, unsigned long eventId, void* clientData, void* callData);

};

#endif