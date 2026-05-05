/**     @file VRRenderThread.h
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Manages VR rendering in a separate thread so that the
  *     main GUI remains responsive during VR operation.
  */

#ifndef VRRENDERTHREAD_H
#define VRRENDERTHREAD_H

#include <QThread>
#include <QList>

#include <vtkSmartPointer.h>
#include <vtkActor.h>

#include <atomic>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRRenderWindow.h>

#include <array>

#include <QMutex>

#include <vtkEventData.h>

#include "ModelPart.h"

/** Thread class that handles all VR rendering separately from the Qt GUI.
  * Runs the OpenVR render loop, manages actors, handles live updates
  * from the GUI, and controls animation and explode view.
  */
class VRRenderThread : public QThread
{
    Q_OBJECT

public:
    /** Constructor
      * @param parent is the parent QObject (usually the MainWindow)
      */
    explicit VRRenderThread(QObject* parent = nullptr);

    /** Add an actor to be rendered in VR.
      * Must be called before the thread starts (use queueVRPipelineUpdate for live updates).
      * @param actor is the VTK actor to add to the VR scene
      */
    void addActorOffline(vtkSmartPointer<vtkActor> actor);

    /** Queue a model part so its VR rendering pipeline gets updated on the next frame.
      * Safe to call while VR is running.
      * @param part is the ModelPart needing a pipeline update
      */
    void queueVRPipelineUpdate(ModelPart* part);

    /** Request the VR rendering loop to stop.
      * The thread will finish its current frame before stopping.
      */
    void stopVR();

    /** Request the VR camera and actor positions to be reset back to defaults.
      */
    void resetView();

    /** Toggle the explode view animation on or off.
      * If currently collapsed, parts will animate outward, and vice versa.
      */
    void toggleExplode();

    /** Enable or disable the VR rotation animation.
      * @param enabled is true to start animation, false to stop it
      */
    void setAnimationEnabled(bool enabled);

    /** Check whether VR animation is currently enabled.
      * @return true if animation is running, false if not
      */
    bool animationIsEnabled() const;

protected:
    /** Main thread function - runs the VR render loop until stopVR() is called.
      */
    void run() override;

private:
    QList<vtkSmartPointer<vtkActor>> actors;          /**< Actors currently in the VR scene */
    QList<vtkSmartPointer<vtkActor>> pendingActors;   /**< Actors waiting to be added to the live VR scene */
    QMutex pendingActorsMutex;                         /**< Protects pendingActors from concurrent access */

    QList<ModelPart*> pendingVRUpdates;               /**< Parts waiting for their VR pipeline to be updated */
    QMutex pendingVRUpdatesMutex;                      /**< Protects pendingVRUpdates from concurrent access */

    std::atomic<bool> stopRequested{ false };          /**< Set to true when the VR loop should stop */
    std::atomic<bool> resetRequested{ false };         /**< Set to true when the view should be reset */
    std::atomic<bool> animationEnabled{ false };       /**< True if the rotation animation is active */
    std::atomic<bool> explodeToggleRequested{ false }; /**< Set to true when explode view should be toggled */

    QList<std::array<double, 3>> originalPositions;    /**< Original actor positions before explode or animation */
    QList<std::array<double, 3>> originalScales;       /**< Original actor scales before explode or animation */
    QList<std::array<double, 3>> originalOrientations; /**< Original actor orientations before explode or animation */

    bool explodeActive = false;                        /**< True if explode view is currently active */
    double explodeProgress = 0.0;                      /**< Current progress of the explode animation (0.0 to 1.0) */
    QList<std::array<double, 3>> explodedPositions;    /**< Target positions for actors in exploded view */

    vtkSmartPointer<vtkOpenVRRenderWindowInteractor> vrInteractor;  /**< OpenVR interactor for handling VR input */
    vtkSmartPointer<vtkOpenVRRenderWindow> vrRenderWindow;          /**< OpenVR render window for VR display output */
};

#endif
