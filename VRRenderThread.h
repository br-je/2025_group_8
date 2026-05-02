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
    void toggleExplode();
    void undoDrag();
    void redoDrag();

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
    std::atomic<bool> explodeToggleRequested{ false };
    std::atomic<bool> undoRequested{ false };
    std::atomic<bool> redoRequested{ false };

    // Undo/redo history for part dragging
    struct PartMove
    {
        vtkSmartPointer<vtkActor> actor;
        std::array<double, 3> from;
        std::array<double, 3> to;
    };
    QList<PartMove> undoStack;
    QList<PartMove> redoStack;
    std::array<double, 3> dragStartPosition = { 0.0, 0.0, 0.0 };

    QList<std::array<double, 3>> originalPositions;
    QList<std::array<double, 3>> originalScales;
    QList<std::array<double, 3>> originalOrientations;

    // Explode animation state (only accessed on VR thread)
    bool explodeActive = false;
    double explodeProgress = 0.0;
    QList<std::array<double, 3>> explodedPositions;


    vtkSmartPointer<vtkOpenVRRenderWindowInteractor> vrInteractor;
    vtkSmartPointer<vtkOpenVRRenderWindow> vrRenderWindow;

    // Controller dragging state
    vtkSmartPointer<vtkActor> draggedActor;
    double lastControllerPos[3] = { 0.0, 0.0, 0.0 };
    vtkEventDataDevice grabbingDevice = vtkEventDataDevice::Unknown;

    // Hover highlight state — the actor currently glowing, and its original colour
    vtkSmartPointer<vtkActor> hoveredActor;
    double hoveredOriginalColor[3] = { 1.0, 1.0, 1.0 };

    // Static callbacks fired by the VR interactor on the VR thread
    static void OnButton3D(vtkObject* caller, unsigned long eventId, void* clientData, void* callData);
    static void OnMove3D(vtkObject* caller, unsigned long eventId, void* clientData, void* callData);

};

#endif