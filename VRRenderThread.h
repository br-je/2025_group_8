/**
 * @file VRRenderThread.h
 * @brief Defines the VRRenderThread class used to handle VR rendering in a separate thread.
 */

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


//StopVR
#include <atomic>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRRenderWindow.h>

#include <array>

//Experimental adding STL during live VR
#include <QMutex>

#include <vtkEventData.h>

#include "ModelPart.h"


/**
 * @brief Thread responsible for rendering actors in a VR environment.
 *
 * Runs separately from the main GUI thread to allow smooth VR rendering.
 */
class VRRenderThread : public QThread
{
    Q_OBJECT

public:

    /**
     * @brief Constructor
     * @param parent Parent QObject
     */
    explicit VRRenderThread(QObject* parent = nullptr);

    /**
     * @brief Adds an actor to be rendered in VR.
     * @param actor VTK actor to add
     */
    void addActorOffline(vtkSmartPointer<vtkActor> actor);
    void queueVRPipelineUpdate(ModelPart* part);
    void stopVR();
    void resetView();
    void toggleExplode();

	//Animation control
    void setAnimationEnabled(bool enabled);
    bool animationIsEnabled() const;


protected:

    /**
     * @brief Main thread execution function.
     *
     * Runs the VR rendering loop.
     */
    void run() override;

private:

    /** List of actors to be rendered in VR */
    QList<vtkSmartPointer<vtkActor>> actors;
    QList<vtkSmartPointer<vtkActor>> pendingActors;
    QMutex pendingActorsMutex;

    QList<ModelPart*> pendingVRUpdates;
    QMutex pendingVRUpdatesMutex;

    std::atomic<bool> stopRequested{ false };
    std::atomic<bool> resetRequested{ false };
    std::atomic<bool> animationEnabled{ false };
    std::atomic<bool> explodeToggleRequested{ false };

    QList<std::array<double, 3>> originalPositions;
    QList<std::array<double, 3>> originalScales;
    QList<std::array<double, 3>> originalOrientations;

    // Explode animation state (only accessed on VR thread)
    bool explodeActive = false;
    double explodeProgress = 0.0;
    QList<std::array<double, 3>> explodedPositions;


    vtkSmartPointer<vtkOpenVRRenderWindowInteractor> vrInteractor;
    vtkSmartPointer<vtkOpenVRRenderWindow> vrRenderWindow;

};

#endif
