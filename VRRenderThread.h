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
};

#endif
