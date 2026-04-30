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

class VRRenderThread : public QThread
{
    Q_OBJECT

public:
    explicit VRRenderThread(QObject* parent = nullptr);

    void addActorOffline(vtkSmartPointer<vtkActor> actor);
    void stopVR();

protected:
    void run() override;

private:
    QList<vtkSmartPointer<vtkActor>> actors;

    std::atomic<bool> stopRequested{ false };
    vtkSmartPointer<vtkOpenVRRenderWindowInteractor> vrInteractor;
    vtkSmartPointer<vtkOpenVRRenderWindow> vrRenderWindow;
};

#endif