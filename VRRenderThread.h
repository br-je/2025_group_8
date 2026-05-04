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

    /**
    * @brief Queues a model part so its VR rendering pipeline can be updated safely.
    * @param part ModelPart requiring a VR pipeline update.
    */
    void queueVRPipelineUpdate(ModelPart* part);

    /**
    * @brief Requests the VR rendering loop to stop.
    */
    void stopVR();

    /**
    * @brief Requests the VR view/camera to be reset.
    */
    void resetView();

    /**
    * @brief Toggles the exploded view mode in VR.
    */
    void toggleExplode();


	//Animation control
   /**
   * @brief Enables or disables VR animation.
   * @param enabled True to enable animation, false to disable it.
   */
    void setAnimationEnabled(bool enabled);

   /**
   * @brief Checks whether VR animation is enabled.
   * @return True if animation is enabled.
   */
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

/** Actors currently rendered in the VR scene. */
    QList<vtkSmartPointer<vtkActor>> actors;

/** Actors waiting to be added to the live VR scene. */
    QList<vtkSmartPointer<vtkActor>> pendingActors;

/** Mutex used to protect access to pendingActors. */
    QMutex pendingActorsMutex;

/** Model parts waiting for VR pipeline updates. */
    QList<ModelPart*> pendingVRUpdates;

/** Mutex used to protect access to pendingVRUpdates. */
    QMutex pendingVRUpdatesMutex;

/** Flag used to request the VR thread to stop safely. */
    std::atomic<bool> stopRequested{ false };

/** Flag used to request a VR view reset. */
    std::atomic<bool> resetRequested{ false };

/** Flag storing whether animation is enabled. */
    std::atomic<bool> animationEnabled{ false };

/** Flag used to request toggling of exploded view mode. */
    std::atomic<bool> explodeToggleRequested{ false };

/** Original actor positions before animation or exploded view changes. */
    QList<std::array<double, 3>> originalPositions;

/** Original actor scales before animation or exploded view changes. */
    QList<std::array<double, 3>> originalScales;

/** Original actor orientations before animation or exploded view changes. */
    QList<std::array<double, 3>> originalOrientations;

    // Explode animation state (only accessed on VR thread)

/** Stores whether exploded view mode is currently active. */
    bool explodeActive = false;

/** Progress value used for the exploded view animation. */
    double explodeProgress = 0.0;

/** Target positions used when actors are moved into exploded view. */
    QList<std::array<double, 3>> explodedPositions;

/** OpenVR interactor used to handle VR input and interaction. */
    vtkSmartPointer<vtkOpenVRRenderWindowInteractor> vrInteractor;

/** OpenVR render window used for VR display output. */
    vtkSmartPointer<vtkOpenVRRenderWindow> vrRenderWindow;

};

#endif
