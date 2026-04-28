#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include "PartPropertiesDialog.h"

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkNew.h>

#include <QMenu>
#include <QAction>
#include <vtkSphereSource.h>

#include <QDir>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Link a render window with the Qt widget
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->vtkWidget->setRenderWindow(renderWindow);

    // Add a renderer
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    // Create an object and add to renderer (temporary test geometry)
    vtkNew<vtkCylinderSource> cylinder;
    cylinder->SetResolution(8);

    vtkNew<vtkPolyDataMapper> cylinderMapper;
    cylinderMapper->SetInputConnection(cylinder->GetOutputPort());

    vtkNew<vtkActor> cylinderActor;
    cylinderActor->SetMapper(cylinderMapper);
    cylinderActor->GetProperty()->SetColor(1.0, 0.0, 0.35);
    cylinderActor->RotateX(30.0);
    cylinderActor->RotateY(45.0);

    renderer->AddActor(cylinderActor);

    // Reset camera / viewpoint
    renderer->ResetCamera();
    renderer->GetActiveCamera()->Azimuth(30);
    renderer->GetActiveCamera()->Elevation(30);
    renderer->ResetCameraClippingRange();

    connect(ui->treeView, &QTreeView::clicked,
            this, &MainWindow::handleTreeClicked);

    // Create the model
    this->partList = new ModelPartList("Parts List");

    // Attach model to tree view
    ui->treeView->setModel(this->partList);

    //Create top-level folder node
    QModelIndex rootIndex;
    partsRootIndex = partList->appendChild(
        rootIndex,
        { QString("Parts"), QString("true") }
        );

    ui->treeView->expand(partsRootIndex);
    ui->treeView->setCurrentIndex(partsRootIndex);

    // Get root item
    //ModelPart* rootItem = this->partList->getRootItem();

    // Add demo data
    //for (int i = 0; i < 3; i++) {
        //QString name = QString("TopLevel %1").arg(i);
        //QString visible("true");

        //ModelPart* childItem = new ModelPart({name, visible});
        //rootItem->appendChild(childItem);

        //for (int j = 0; j < 5; j++) {
            //QString subName = QString("Item %1,%2").arg(i).arg(j);
            //ModelPart* subItem = new ModelPart({subName, visible});
            //childItem->appendChild(subItem);
        //}
    //}

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeView, &QTreeView::customContextMenuRequested,
            this, &MainWindow::openContextMenu);

    connect(this, &MainWindow::statusUpdateMessage,
            ui->statusbar, &QStatusBar::showMessage);

    connect(ui->pushButton, &QPushButton::released,
            this, &MainWindow::handleButton1);

    connect(ui->ClearSelectionButton, &QPushButton::released,
            this, &MainWindow::handleClearSelection);

	// Initialize VR thread pointer (EXPERIMENTAL - will be used in future VR implementation)
    vrThread = nullptr;

    connect(ui->StartVRButton, &QPushButton::released,
        this, &MainWindow::startVR);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleButton1()
{
    updateRender();
    emit statusUpdateMessage("Button 1 clicked", 3000);
}

void MainWindow::handleTreeClicked(const QModelIndex &index)
{
    //Get pointer to the underlying ModelPart
    ModelPart* selectedPart =
        static_cast<ModelPart*>(index.internalPointer());

    if (!selectedPart)
        return;

    //Column 0 = name
    QString name = selectedPart->data(0).toString();

    emit statusUpdateMessage("Selected item: " + name, 0);
}

void MainWindow::on_actionOpen_File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open STL File"),
        "C:\\",
        tr("STL Files (*.stl)")
    );

    if (fileName.isEmpty())
        return;

    QFileInfo info(fileName);

    QModelIndex parentIndex = partsRootIndex;

    QModelIndex childIndex = partList->appendChild(
        parentIndex,
        { info.fileName(), QString("true") }
    );

    ModelPart* part = static_cast<ModelPart*>(childIndex.internalPointer());
    if (!part)
        return;

    part->loadSTL(fileName);

    ui->treeView->expand(parentIndex);
    ui->treeView->setCurrentIndex(childIndex);
    ui->treeView->viewport()->update();

    updateRender();

    emit statusUpdateMessage("Loaded STL file: " + info.fileName(), 3000);
}

void MainWindow::on_actionOpen_Folder_triggered()
{
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        tr("Open STL Folder"),
        "C:\\"
    );

    if (dirPath.isEmpty())
        return;

    QFileInfo dirInfo(dirPath);

    QModelIndex parentIndex = partsRootIndex;

    QModelIndex folderIndex = partList->appendChild(
        parentIndex,
        { dirInfo.fileName(), QString("true") }
    );

    int fileCount = loadSTLFilesFromDirectory(dirPath, folderIndex, true);

    ui->treeView->expand(parentIndex);
    ui->treeView->expand(folderIndex);
    ui->treeView->setCurrentIndex(folderIndex);
    ui->treeView->viewport()->update();

    updateRender();

    emit statusUpdateMessage(
        "Loaded folder: " + dirInfo.fileName() +
        " (" + QString::number(fileCount) + " STL files)",
        3000
    );
}

void MainWindow::openContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->treeView->indexAt(pos);

    if (!index.isValid())
        return;

    ui->treeView->setCurrentIndex(index);

    ModelPart *part = static_cast<ModelPart*>(index.internalPointer());

    if (!part)
        return;

    QMenu menu(this);

    QAction *editAction = menu.addAction("Edit Properties");
    QAction *removeAction = menu.addAction("Remove Selected Item");

    QAction *selected =
        menu.exec(ui->treeView->viewport()->mapToGlobal(pos));

    if (selected == editAction)
    {
        PartPropertiesDialog dialog(this);
        dialog.setModelPart(part);

        if (dialog.exec() == QDialog::Accepted)
        {
            // If the selected item is a folder/group, apply its colour and visibility
            // to all child parts below it.
            applyPropertiesToChildren(part);

            ui->treeView->viewport()->update();
            updateRender();

            emit statusUpdateMessage("Item properties updated", 3000);
        }
    }
    else if (selected == removeAction)
    {
        removeSelectedItem();
    }
}

void MainWindow::applyPropertiesToChildren(ModelPart* parentPart)
{
    if (!parentPart)
        return;

    for (int i = 0; i < parentPart->childCount(); i++)
    {
        ModelPart* childPart = parentPart->child(i);

        if (!childPart)
            continue;

        // Copy visibility from parent
        childPart->set(1, parentPart->visible() ? "true" : "false");
        childPart->setVisible(parentPart->visible());

        // Copy colour from parent
        childPart->setColour(
            parentPart->getColourR(),
            parentPart->getColourG(),
            parentPart->getColourB()
        );

        // Recursively apply to children of children
        applyPropertiesToChildren(childPart);
    }
}

void MainWindow::updateRender()
{
    renderer->RemoveAllViewProps();

    // Start recursion from root
    updateRenderFromTree(QModelIndex());

    renderer->ResetCamera();
    renderWindow->Render();
}

void MainWindow::updateRenderFromTree(const QModelIndex& index)
{
    if (index.isValid())
    {
        ModelPart* part = static_cast<ModelPart*>(index.internalPointer());
        if (part && part->getActor() && part->visible())
        {
            renderer->AddActor(part->getActor());
        }
    }

    int rows = partList->rowCount(index);
    for (int i = 0; i < rows; i++)
    {
        QModelIndex childIndex = partList->index(i, 0, index);
        updateRenderFromTree(childIndex);
    }
}

void MainWindow::handleClearSelection()
{
    ui->treeView->clearSelection();
    ui->treeView->setCurrentIndex(QModelIndex()); // clears current item
    emit statusUpdateMessage("Selection cleared", 2000);
}

void MainWindow::removeSelectedItem()
{
    QModelIndex index = ui->treeView->currentIndex();

    if (!index.isValid())
    {
        emit statusUpdateMessage("No item selected to remove", 3000);
        return;
    }

    if (index == partsRootIndex)
    {
        emit statusUpdateMessage("Cannot remove the main Parts folder", 3000);
        return;
    }

    QModelIndex parentIndex = index.parent();
    int row = index.row();

    bool removed = partList->removeRow(row, parentIndex);

    if (removed)
    {
        updateRender();
        emit statusUpdateMessage("Selected item removed", 3000);
    }
    else
    {
        emit statusUpdateMessage("Failed to remove selected item", 3000);
    }
}

//MAIN VR FUNCTION (EXPERIMENTAL - will be used in future VR implementation)
int MainWindow::addVRActorsFromTree(const QModelIndex& index, VRRenderThread* thread)
{
    int actorCount = 0;

    if (index.isValid())
    {
        ModelPart* part = static_cast<ModelPart*>(index.internalPointer());

        if (part && part->visible())
        {
            vtkSmartPointer<vtkActor> vrActor = part->getNewActor();

            if (vrActor)
            {
                thread->addActorOffline(vrActor);
                actorCount++;
            }
        }
    }

    int rows = partList->rowCount(index);

    for (int i = 0; i < rows; i++)
    {
        QModelIndex childIndex = partList->index(i, 0, index);
        actorCount += addVRActorsFromTree(childIndex, thread);
    }

    return actorCount;
}

void MainWindow::startVR()
{
    if (vrThread && vrThread->isRunning())
    {
        emit statusUpdateMessage("VR is already running", 3000);
        return;
    }

    vrThread = new VRRenderThread(this);

    int actorCount = addVRActorsFromTree(QModelIndex(), vrThread);

    // Fallback test cylinder if no STL files have been loaded yet.
    if (actorCount == 0)
    {
        vtkNew<vtkCylinderSource> cylinder;
        cylinder->SetResolution(8);

        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(cylinder->GetOutputPort());

        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(1.0, 0.0, 0.35);
        actor->RotateX(30.0);
        actor->RotateY(45.0);

        vrThread->addActorOffline(actor);

        emit statusUpdateMessage("No STL loaded, showing test cylinder in VR", 3000);
    }
    else
    {
        emit statusUpdateMessage("Starting VR with loaded STL model", 3000);
    }

    connect(vrThread, &QThread::finished, vrThread, &QObject::deleteLater);

    connect(vrThread, &QThread::finished, this, [this]() {
        vrThread = nullptr;
        });

    vrThread->start();
}

// Recursively loads STL files from the specified directory and adds them to the model tree under the given parent index.
int MainWindow::loadSTLFilesFromDirectory(const QString& dirPath, QModelIndex parentIndex, bool recursive)
{
    QDir dir(dirPath);
    int loadedCount = 0;

    QStringList stlFilters;
    stlFilters << "*.stl" << "*.STL";

    QFileInfoList stlFiles = dir.entryInfoList(
        stlFilters,
        QDir::Files,
        QDir::Name
    );

    for (const QFileInfo& fileInfo : stlFiles)
    {
        QModelIndex childIndex = partList->appendChild(
            parentIndex,
            { fileInfo.fileName(), QString("true") }
        );

        ModelPart* part = static_cast<ModelPart*>(childIndex.internalPointer());

        if (part)
        {
            part->loadSTL(fileInfo.absoluteFilePath());
            loadedCount++;
        }
    }

    if (recursive)
    {
        QFileInfoList subDirs = dir.entryInfoList(
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name
        );

        for (const QFileInfo& subDirInfo : subDirs)
        {
            QModelIndex subFolderIndex = partList->appendChild(
                parentIndex,
                { subDirInfo.fileName(), QString("true") }
            );

            loadedCount += loadSTLFilesFromDirectory(
                subDirInfo.absoluteFilePath(),
                subFolderIndex,
                true
            );
        }
    }

    return loadedCount;
}