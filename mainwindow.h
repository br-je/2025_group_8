#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "ModelPart.h"
#include "ModelPartList.h"
#include <QMainWindow>
#include <vtkSmartPointer.h>
#include "VRRenderThread.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class vtkRenderer;
class vtkGenericOpenGLRenderWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void statusUpdateMessage(const QString &message, int timeout);

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void handleButton1();

public slots:
    void handleTreeClicked(const QModelIndex &index);
    

private slots:
    void openContextMenu(const QPoint &pos);
    void removeSelectedItem();
    void on_actionOpen_File_triggered();
    void startVR();
    void stopVR();
    void on_actionOpen_Folder_triggered();
    void toggleVRAnimation();
    void resetModelView();

public slots:
    void handleClearSelection();

private:
    Ui::MainWindow *ui;
    ModelPartList* partList;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    void updateRender();
    void updateRenderFromTree(const QModelIndex& index);
    QModelIndex partsRootIndex;
    VRRenderThread* vrThread;
    int addVRActorsFromTree(const QModelIndex& index, VRRenderThread* thread);
    int loadSTLFilesFromDirectory(const QString& dirPath, QModelIndex parentIndex, bool recursive);
    void applyPropertiesToChildren(ModelPart* parentPart);
    bool vrAnimationEnabled = false;
    void addPartToLiveVR(ModelPart* part);
};
#endif // MAINWINDOW_H
