/**     @file mainwindow.h
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Main application window handling the GUI, model tree,
  *     VTK renderer, and VR rendering controls.
  */

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

/** Main application window for the CAD viewer.
  * Handles the Qt GUI, tree view, STL loading, VTK rendering,
  * context menu, and VR controls.
  */
class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    /** Sends a status message to the status bar.
      * @param message is the text to display in the status bar
      * @param timeout is how long in milliseconds before it disappears (0 = stays)
      */
    void statusUpdateMessage(const QString &message, int timeout);

public:
    /** Constructor - sets up the UI, renderer, tree view, and button connections.
      * @param parent is the parent widget (usually nullptr for main window)
      */
    MainWindow(QWidget *parent = nullptr);

    /** Destructor - frees the UI.
      */
    ~MainWindow();

public slots:
    /** Called when the user clicks an item in the tree view.
      * @param index is the index of the item that was clicked
      */
    void handleTreeClicked(const QModelIndex &index);

    /** Clears the current selection in the tree view.
      */
    void handleClearSelection();

private slots:
    /** Opens a right-click context menu at the given position in the tree view.
      * @param pos is the position where the menu should appear
      */
    void openContextMenu(const QPoint &pos);

    /** Removes the currently selected item from the model tree.
      */
    void removeSelectedItem();

    /** Opens a file dialog so the user can load a single STL file.
      */
    void on_actionOpen_File_triggered();

    /** Starts the VR rendering thread with the currently loaded parts.
      */
    void startVR();

    /** Stops the VR rendering thread safely.
      */
    void stopVR();

    /** Opens a folder dialog and loads all STL files found inside it recursively.
      */
    void on_actionOpen_Folder_triggered();

    /** Toggles VR animation on or off and updates the button text accordingly.
      */
    void toggleVRAnimation();

    /** Resets the GUI and VR camera back to the default view.
      */
    void resetModelView();

    /** Toggles the explode view animation in VR on or off.
      */
    void toggleExplode();

private:
    Ui::MainWindow *ui;                                         /**< Pointer to the generated Qt UI */
    ModelPartList* partList;                                    /**< List of CAD parts shown in the tree view */
    vtkSmartPointer<vtkRenderer> renderer;                      /**< VTK renderer for the main 3D scene */
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow; /**< VTK render window embedded in the Qt widget */

    /** Clears and rebuilds the VTK render window from the current model tree.
      */
    void updateRender();

    /** Walks the model tree from the given index and adds visible actors to the renderer.
      * @param index is the tree index to start from
      */
    void updateRenderFromTree(const QModelIndex& index);

    QModelIndex partsRootIndex;  /**< Root index of the parts tree (the top-level "Parts" node) */
    VRRenderThread* vrThread;    /**< Pointer to the VR rendering thread, nullptr if VR is not running */

    /** Walks the model tree and adds each part's actor to the VR render thread.
      * @param index is the starting tree index
      * @param thread is the VR thread to add actors to
      * @return number of actors added
      */
    int addVRActorsFromTree(const QModelIndex& index, VRRenderThread* thread);

    /** Opens a directory and loads all STL files found inside, adding them to the tree.
      * @param dirPath is the path of the directory to search
      * @param parentIndex is the tree node to add loaded files under
      * @param recursive is true to also search subdirectories
      * @return number of STL files successfully loaded
      */
    int loadSTLFilesFromDirectory(const QString& dirPath, QModelIndex parentIndex, bool recursive);

    /** Copies the colour, visibility, and filter settings from a parent part down to all its children.
      * @param parentPart is the part whose properties should be copied down
      */
    void applyPropertiesToChildren(ModelPart* parentPart);

    bool vrAnimationEnabled = false;  /**< Tracks whether VR animation is currently running */

    /** Adds a single part's actor to the live VR scene if VR is already running.
      * @param part is the model part to add to VR
      */
    void addPartToLiveVR(ModelPart* part);

    /** Walks the model tree and queues VR pipeline updates for each part.
      * @param index is the starting tree index to process from
      */
    void queueVRUpdatesFromTree(const QModelIndex& index);
};
#endif // MAINWINDOW_H
