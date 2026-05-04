/**
 * @file mainwindow.h
 * @brief Defines the MainWindow class used to control the main GUI, model tree,
 *        VTK renderer, and VR rendering features.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ModelPart.h"
#include "ModelPartList.h"
#include "VRRenderThread.h"

#include <QMainWindow>
#include <vtkSmartPointer.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class vtkRenderer;
class vtkGenericOpenGLRenderWindow;

/**
 * @brief Main application window for the CAD viewer.
 *
 * Handles the Qt user interface, tree view interactions, STL loading,
 * VTK rendering, context menu actions, and starting the VR renderer.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:

    /**
     * @brief Sends status messages to the user interface.
     * @param message Message to display.
     * @param timeout Time in milliseconds before the message disappears.
     */
    void statusUpdateMessage(const QString &message, int timeout);

public:

    /**
     * @brief Constructor for MainWindow.
     * @param parent Parent widget.
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~MainWindow();

public slots:

    /**
     * @brief Handles the main button action.
     */
    void handleButton1();

    /**
     * @brief Handles selection of an item in the tree view.
     * @param index Index of the clicked tree item.
     */
    void handleTreeClicked(const QModelIndex &index);

    /**
     * @brief Clears the current tree selection.
     */
    void handleClearSelection();

private slots:

    /**
     * @brief Opens the context menu for the tree view.
     * @param pos Mouse position where the menu should open.
     */
    void openContextMenu(const QPoint &pos);

    /**
     * @brief Removes the currently selected item from the tree.
     */
    void removeSelectedItem();

    /**
     * @brief Opens an STL file using the file menu action.
     */
    void on_actionOpen_File_triggered();

    /**
     * @brief Starts the VR rendering thread.
     */
    void startVR();

    /**
    * @brief Stops the VR rendering thread.
    */
    void stopVR();
    /**
    * @brief Opens a folder and loads STL files into the model tree.
    */
    void on_actionOpen_Folder_triggered();
/**
 * @brief Clears the current tree selection.
 */
public slots:
    void handleClearSelection();

private:

    /** Pointer to the generated Qt user interface. */
    Ui::MainWindow *ui;

    /** Model containing the list of CAD parts shown in the tree view. */
    ModelPartList* partList;

    /** VTK renderer used for the main 3D scene. */
    vtkSmartPointer<vtkRenderer> renderer;

    /** VTK render window embedded in the Qt interface. */
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;

    /**
     * @brief Updates the VTK render window.
     */
    void updateRender();

    /**
     * @brief Updates rendering based on the selected tree item.
     * @param index Index of the tree item to render/update.
     */
    void updateRenderFromTree(const QModelIndex& index);

    /** Root index used for the model parts tree. */
    QModelIndex partsRootIndex;

    /** Pointer to the VR rendering thread. */
    VRRenderThread* vrThread;

    /**
     * @brief Adds actors from the tree into the VR render thread.
     * @param index Starting tree index.
     * @param thread VR thread to receive actors.
     * @return Number of actors added.
     */
    int addVRActorsFromTree(const QModelIndex& index, VRRenderThread* thread);

    /**
     * @brief Loads STL files from a directory.
     * @param dirPath Directory path to search.
     * @param parentIndex Tree index where loaded files should be added.
     * @param recursive True to search subfolders as well.
     * @return Number of STL files loaded.
     */
    int loadSTLFilesFromDirectory(const QString& dirPath, QModelIndex parentIndex, bool recursive);

    /**
     * @brief Applies a parent part's properties to its child items.
     * @param parentPart Parent model part whose properties should be copied.
     */
    void applyPropertiesToChildren(ModelPart* parentPart);

    /** Indicates whether VR animation mode is enabled. */
    bool vrAnimationEnabled = false;

    /**
    * @brief Adds a model part to the active VR scene.
    * @param part Model part to be added.
    */
    void addPartToLiveVR(ModelPart* part);

    /**
    * @brief Queues updates for VR rendering based on the model tree.
    * @param index Starting index for processing model parts.
    */
    void queueVRUpdatesFromTree(const QModelIndex& index);
};

#endif // MAINWINDOW_H
