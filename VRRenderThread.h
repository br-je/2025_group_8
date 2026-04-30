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

	//Animation control
    void setAnimationEnabled(bool enabled);
    bool animationIsEnabled() const;

protected:
    void run() override;

private:
    QList<vtkSmartPointer<vtkActor>> actors;

    std::atomic<bool> stopRequested{ false };
    std::atomic<bool> animationEnabled{ false };
    vtkSmartPointer<vtkOpenVRRenderWindowInteractor> vrInteractor;
    vtkSmartPointer<vtkOpenVRRenderWindow> vrRenderWindow;

};

#endif