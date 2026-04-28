/**     @file ModelPart.cpp
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Template for model parts that will be added as treeview items
  *
  *     P Evans 2022
  */

#include "ModelPart.h"


/* Commented out for now, will be uncommented later when you have
 * installed the VTK library
 */
#include <vtkSmartPointer.h>
#include <vtkDataSetMapper.h>
#include <vtkSTLReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkShrinkFilter.h>
#include <vtkDataSetMapper.h>


ModelPart::ModelPart(const QList<QVariant>& data, ModelPart* parent )
    : m_itemData(data), m_parentItem(parent) {
        // default visible
    isVisible = true;
        // default white
    colourR = 255;
    colourG = 255;
    colourB = 255;
    
	    //default shrink
    applyShrinkFilter = false;
    shrinkFilterFactor = 0.8;

}


ModelPart::~ModelPart() {
    qDeleteAll(m_childItems);
}


void ModelPart::appendChild( ModelPart* item ) {
    /* Add another model part as a child of this part
     * (it will appear as a sub-branch in the treeview)
     */
    item->m_parentItem = this;
    m_childItems.append(item);
}


ModelPart* ModelPart::child( int row ) {
    /* Return pointer to child item in row below this item.
     */
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int ModelPart::childCount() const {
    /* Count number of child items
     */
    return m_childItems.count();
}


int ModelPart::columnCount() const {
    /* Count number of columns (properties) that this item has.
     */
    return m_itemData.count();
}

QVariant ModelPart::data(int column) const {
    /* Return the data associated with a column of this item 
     *  Note on the QVariant type - it is a generic placeholder type
     *  that can take on the type of most Qt classes. It allows each 
     *  column or property to store data of an arbitrary type.
     */
    if (column < 0 || column >= m_itemData.size())
        return QVariant();
    return m_itemData.at(column);
}


void ModelPart::set(int column, const QVariant &value) {
    /* Set the data associated with a column of this item 
     */
    if (column < 0 || column >= m_itemData.size())
        return;

    m_itemData.replace(column, value);
}


ModelPart* ModelPart::parentItem() {
    return m_parentItem;
}


int ModelPart::row() const {
    /* Return the row index of this item, relative to it's parent.
     */
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ModelPart*>(this));
    return 0;
}

//This function sets the colour of the part, and updates the colour of the actor if it exists.
//Formaly a placeholder, but now fully functional.
void ModelPart::setColour(const unsigned char R, const unsigned char G, const unsigned char B) {
    colourR = R;
    colourG = G;
    colourB = B;

    if (actor)
    {
        actor->GetProperty()->SetColor(R / 255.0, G / 255.0, B / 255.0);
    }
}

unsigned char ModelPart::getColourR() {
    //was placeholder
    return colourR;
}

unsigned char ModelPart::getColourG() {
    //was placeholder
    return colourG;
}


unsigned char ModelPart::getColourB() {
    //was placeholder
    return colourB;
}

//Was placeholder, changed to work with vr rendering.
void ModelPart::setVisible(bool visibleState) {
    isVisible = visibleState;

    if (actor)
    {
        actor->SetVisibility(visibleState);
		// Ensure the actor's colour is updated to reflect visibility change
        actor->GetProperty()->SetColor(colourR / 255.0, colourG / 255.0, colourB / 255.0);
        actor->SetVisibility(isVisible);
    }
}

bool ModelPart::visible() {
    //was placeholder
    return isVisible;
}

//This function updates the VTK pipeline to apply the shrink filter if enabled, and ensures that any changes are reflected in the actor for rendering.
void ModelPart::updatePipeline()
{
    if (!file || !mapper)
        return;

    if (applyShrinkFilter)
    {
        shrinkFilter = vtkSmartPointer<vtkShrinkFilter>::New();
        shrinkFilter->SetInputConnection(file->GetOutputPort());
        shrinkFilter->SetShrinkFactor(shrinkFilterFactor);
        shrinkFilter->Update();

        mapper->SetInputConnection(shrinkFilter->GetOutputPort());
    }
    else
    {
        mapper->SetInputConnection(file->GetOutputPort());
    }

    if (actor)
    {
        actor->Modified();
    }
}

//This function enables or disables the shrink filter and sets the shrink factor, then updates the pipeline to reflect the changes.
void ModelPart::setShrinkFilter(bool enabled, double factor)
{
    applyShrinkFilter = enabled;
    shrinkFilterFactor = factor;

    updatePipeline();
}

bool ModelPart::shrinkFilterEnabled() const
{
    return applyShrinkFilter;
}

double ModelPart::shrinkFactor() const
{
    return shrinkFilterFactor;
}

void ModelPart::loadSTL( QString fileName ) {
    /* This is a placeholder function that you will need to modify if you want to use it */
    /* 1. Use the vtkSTLReader class to load the STL file 
     *     https://vtk.org/doc/nightly/html/classvtkSTLReader.html
     */
    /* 2. Initialise the part's vtkMapper */
    /* 3. Initialise the part's vtkActor and link to the mapper */

        file = vtkSmartPointer<vtkSTLReader>::New();
        file->SetFileName(fileName.toStdString().c_str());
        file->Update();

        mapper = vtkSmartPointer<vtkDataSetMapper>::New();
        mapper->SetInputConnection(file->GetOutputPort());

        actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(colourR / 255.0, colourG / 255.0, colourB / 255.0);
        actor->SetVisibility(isVisible);

        updatePipeline();

}


//For removing selected item from treeview
bool ModelPart::removeChild(int row)
{
    if (row < 0 || row >= m_childItems.size())
        return false;

    delete m_childItems.takeAt(row);
    return true;
}

vtkSmartPointer<vtkActor> ModelPart::getActor() {
    /* This is a placeholder function that you will need to modify if you want to use it */
    
    /* Needs to return a smart pointer to the vtkActor to allow
     * part to be rendered.
     */

    return actor;
}

vtkSmartPointer<vtkActor> ModelPart::getNewActor()
{
    if (!file || !actor)
    {
        return nullptr;
    }

    vtkSmartPointer<vtkDataSetMapper> newMapper =
        vtkSmartPointer<vtkDataSetMapper>::New();

    if (applyShrinkFilter)
    {
        vtkSmartPointer<vtkShrinkFilter> newShrinkFilter =
            vtkSmartPointer<vtkShrinkFilter>::New();

        newShrinkFilter->SetInputConnection(file->GetOutputPort());
        newShrinkFilter->SetShrinkFactor(shrinkFilterFactor);
        newShrinkFilter->Update();

        newMapper->SetInputConnection(newShrinkFilter->GetOutputPort());
    }
    else
    {
        newMapper->SetInputConnection(file->GetOutputPort());
    }

    vtkSmartPointer<vtkActor> newActor =
        vtkSmartPointer<vtkActor>::New();

    newActor->SetMapper(newMapper);

    // Share colour/material properties with GUI actor.
    newActor->SetProperty(actor->GetProperty());

    // Copy current visibility state.
    newActor->SetVisibility(actor->GetVisibility());

    return newActor;
}

//THE FOLLOWING COMMENTS ARE IN CASE THE PREVIOUS FUNCTIONS ARE NOT WORKING.
//
//vtkActor* ModelPart::getNewActor() {
    /* This is a placeholder function that you will need to modify if you want to use it
     * 
     * The default mapper/actor combination can only be used to render the part in 
     * the GUI, it CANNOT also be used to render the part in VR. This means you need
     * to create a second mapper/actor combination for use in VR - that is the role
     * of this function. */
     
     
     /* 1. Create new mapper */
     
     /* 2. Create new actor and link to mapper */
     
     /* 3. Link the vtkProperties of the original actor to the new actor. This means 
      *    if you change properties of the original part (colour, position, etc), the
      *    changes will be reflected in the GUI AND VR rendering.
      *    
      *    See the vtkActor documentation, particularly the GetProperty() and SetProperty()
      *    functions.
      */
    

    /* The new vtkActor pointer must be returned here */
//    return nullptr;
    
//}

