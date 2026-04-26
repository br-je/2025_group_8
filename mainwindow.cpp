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
        tr("Open STL"),
        "C:\\",
        tr("STL Files (*.stl)")
        );

    if (fileName.isEmpty())
        return;

    QFileInfo info(fileName);

    QModelIndex index = ui->treeView->currentIndex();

    //Overwrite if a part is selected (and it's not the Parts folder)
    if (index.isValid() && index != partsRootIndex)
    {
        ModelPart* part = static_cast<ModelPart*>(index.internalPointer());
        if (!part)
            return;

        part->set(0, info.fileName());
        part->loadSTL(fileName);

        ui->treeView->viewport()->update();
    }
    else
    {
        //Add new STL under Parts folder
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
    }

    updateRender();

    emit statusUpdateMessage("Loaded: " + info.fileName(), 3000);
}

void MainWindow::openContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->treeView->indexAt(pos);
    if (!index.isValid()) return;

    ModelPart *part =
        static_cast<ModelPart*>(index.internalPointer());

    if (!part) return;

    QMenu menu(this);
    QAction *editAction = menu.addAction("Edit Properties");

    QAction *selected =
        menu.exec(ui->treeView->viewport()->mapToGlobal(pos));

    if (selected == editAction)
    {
        PartPropertiesDialog dialog(this);
        dialog.setModelPart(part);

        if (dialog.exec() == QDialog::Accepted)
        {
            ui->treeView->viewport()->update();
            //Render updates on each change to dialog box
            updateRender();
            emit statusUpdateMessage("Item properties updated", 3000);
        }
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
