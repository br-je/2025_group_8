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

	    //default clip
    applyClipFilter = false;
    clipFilterAxis = 0;      // 0 = X, 1 = Y, 2 = Z
    clipFilterValue = 50.0;
    invertClipFilter = false;

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
void ModelPart::setVisible(bool visibleState)
{
    isVisible = visibleState;

    // Update GUI actor visibility.
    if (actor)
    {
        actor->SetVisibility(visibleState);
        actor->Modified();
    }

    // Update VR actor visibility if VR is currently running.
    // The VR actor is separate from the GUI actor, so visibility must be copied manually.
    if (vrActor)
    {
        vrActor->SetVisibility(visibleState);
        vrActor->Modified();
    }
}

bool ModelPart::visible() {
    //was placeholder
    return isVisible;
}

//This function updates the VTK pipeline based on the current settings for the shrink and clip filters.
// It checks if the file and mapper are valid, then applies the shrink filter if enabled,
// followed by the clip filter if enabled. Finally, it updates the mapper's input connection to reflect the changes in the pipeline,
// and marks the actor as modified to ensure it is re-rendered with the updated pipeline.
void ModelPart::updatePipeline()
{
    if (!file || !mapper)
        return;

    vtkAlgorithmOutput* currentOutput = file->GetOutputPort();

	// If the shrink filter is enabled, we create a new vtkShrinkFilter and set its input connection to the current output of the pipeline.
    if (applyShrinkFilter)
    {
        shrinkFilter = vtkSmartPointer<vtkShrinkFilter>::New();
        shrinkFilter->SetInputConnection(currentOutput);
        shrinkFilter->SetShrinkFactor(shrinkFilterFactor);
        shrinkFilter->Update();

        currentOutput = shrinkFilter->GetOutputPort();
    }

	// Similar to the shrink filter, if the clip filter is enabled, we create a new vtkPlane based on the specified axis and value,
    // and apply the vtkClipDataSet filter to the current output of the pipeline. The resulting clipped output
    // is then used as the new input for the mapper. (Adaption from provided worksheet)
    if (applyClipFilter)
    {
        clipPlane = vtkSmartPointer<vtkPlane>::New();

        if (clipFilterAxis == 0)
            clipPlane->SetNormal(1.0, 0.0, 0.0);
        else if (clipFilterAxis == 1)
            clipPlane->SetNormal(0.0, 1.0, 0.0);
        else
            clipPlane->SetNormal(0.0, 0.0, 1.0);


		//IMPORTANT THIS SECTION MAY NOT WORK PROPERLY
        if (clipFilterAxis < 0 || clipFilterAxis > 2)
        {
            clipFilterAxis = 0;
        }

        // Get bounds of the STL data
        double bounds[6];
        file->Update();
        file->GetOutput()->GetBounds(bounds);

        // Convert percentage (0–100) to actual position in model
        double minBound = bounds[clipFilterAxis * 2];
        double maxBound = bounds[clipFilterAxis * 2 + 1];

        double clipFraction = clipFilterValue / 100.0;
        double clipPosition = minBound + clipFraction * (maxBound - minBound);

        // Start from centre of model
        double origin[3] = {
            (bounds[0] + bounds[1]) / 2.0,
            (bounds[2] + bounds[3]) / 2.0,
            (bounds[4] + bounds[5]) / 2.0
        };

        // Override selected axis
        origin[clipFilterAxis] = clipPosition;

        clipPlane->SetOrigin(origin);

        clipFilter = vtkSmartPointer<vtkClipDataSet>::New();
        clipFilter->SetInputConnection(currentOutput);
        clipFilter->SetClipFunction(clipPlane);
        clipFilter->SetInsideOut(invertClipFilter);
        clipFilter->Update();

        currentOutput = clipFilter->GetOutputPort();
    }

    mapper->SetInputConnection(currentOutput);

    if (actor)
        actor->Modified();
}

// Shrink filter function
void ModelPart::setShrinkFilter(bool enabled, double factor)
{
    applyShrinkFilter = enabled;
    shrinkFilterFactor = factor;

    updatePipeline();
    // VR pipeline update is deferred — MainWindow queues it via VRRenderThread so it
    // runs on the VR thread, avoiding cross-thread VTK access.
}

bool ModelPart::shrinkFilterEnabled() const
{
    return applyShrinkFilter;
}

double ModelPart::shrinkFactor() const
{
    return shrinkFilterFactor;
}

// Clip filter function
void ModelPart::setClipFilter(bool enabled, int axis, double value, bool invert)
{
    applyClipFilter = enabled;
    clipFilterAxis = axis;
    clipFilterValue = value;
    invertClipFilter = invert;

    updatePipeline();
    // VR pipeline update is deferred — see setShrinkFilter comment.
}

bool ModelPart::clipFilterEnabled() const
{
    return applyClipFilter;
}

int ModelPart::clipAxis() const
{
    return clipFilterAxis;
}

double ModelPart::clipValue() const
{
    return clipFilterValue;
}

bool ModelPart::clipInvert() const
{
    return invertClipFilter;
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


void ModelPart::updateVRPipeline()
{
    if (!file || !vrMapper)
    {
        return;
    }

    file->Update();

    vtkSmartPointer<vtkDataSet> currentData =
        vtkDataSet::SafeDownCast(file->GetOutput());

    if (!currentData)
    {
        return;
    }

    // Apply shrink filter to VR pipeline if enabled.
    if (applyShrinkFilter)
    {
        vtkSmartPointer<vtkShrinkFilter> newShrinkFilter =
            vtkSmartPointer<vtkShrinkFilter>::New();

        newShrinkFilter->SetInputData(currentData);
        newShrinkFilter->SetShrinkFactor(shrinkFilterFactor);
        newShrinkFilter->Update();

        currentData = vtkDataSet::SafeDownCast(
            newShrinkFilter->GetOutputDataObject(0)
        );

        if (!currentData)
        {
            return;
        }
    }

    // Apply clip filter to VR pipeline if enabled.
    if (applyClipFilter)
    {
        int safeAxis = clipFilterAxis;

        if (safeAxis < 0 || safeAxis > 2)
        {
            safeAxis = 0;
        }

        vtkSmartPointer<vtkPlane> newClipPlane =
            vtkSmartPointer<vtkPlane>::New();

        if (safeAxis == 0)
        {
            newClipPlane->SetNormal(1.0, 0.0, 0.0);
        }
        else if (safeAxis == 1)
        {
            newClipPlane->SetNormal(0.0, 1.0, 0.0);
        }
        else
        {
            newClipPlane->SetNormal(0.0, 0.0, 1.0);
        }

        double bounds[6];
        currentData->GetBounds(bounds);

        double minBound = bounds[safeAxis * 2];
        double maxBound = bounds[safeAxis * 2 + 1];

        double clipFraction = clipFilterValue / 100.0;
        double clipPosition = minBound + clipFraction * (maxBound - minBound);

        double origin[3] = {
            (bounds[0] + bounds[1]) / 2.0,
            (bounds[2] + bounds[3]) / 2.0,
            (bounds[4] + bounds[5]) / 2.0
        };

        origin[safeAxis] = clipPosition;
        newClipPlane->SetOrigin(origin);

        vtkSmartPointer<vtkClipDataSet> newClipFilter =
            vtkSmartPointer<vtkClipDataSet>::New();

        newClipFilter->SetInputData(currentData);
        newClipFilter->SetClipFunction(newClipPlane);
        newClipFilter->SetInsideOut(invertClipFilter);
        newClipFilter->Update();

        currentData = vtkDataSet::SafeDownCast(
            newClipFilter->GetOutputDataObject(0)
        );

        if (!currentData)
        {
            return;
        }
    }

    // Keep the filtered data alive while VR is running.
    vrFilteredData = currentData;

    // Update the existing VR mapper instead of creating a new actor.
    vrMapper->SetInputData(vrFilteredData);
    vrMapper->Modified();

    if (vrActor)
    {
        vrActor->Modified();
    }
}

vtkSmartPointer<vtkActor> ModelPart::getNewActor()
{
    if (!file || !actor)
    {
        return nullptr;
    }

    file->Update();

    vtkSmartPointer<vtkDataSet> currentData =
        vtkDataSet::SafeDownCast(file->GetOutput());

    if (!currentData)
    {
        return nullptr;
    }

    // Apply shrink filter to VR actor if enabled
    if (applyShrinkFilter)
    {
        vtkSmartPointer<vtkShrinkFilter> newShrinkFilter =
            vtkSmartPointer<vtkShrinkFilter>::New();

        newShrinkFilter->SetInputData(currentData);
        newShrinkFilter->SetShrinkFactor(shrinkFilterFactor);
        newShrinkFilter->Update();

        currentData = vtkDataSet::SafeDownCast(
            newShrinkFilter->GetOutputDataObject(0)
        );

        if (!currentData)
        {
            return nullptr;
        }
    }

    // Apply clip filter to VR actor if enabled
    if (applyClipFilter)
    {
        int safeAxis = clipFilterAxis;
        if (safeAxis < 0 || safeAxis > 2)
        {
            safeAxis = 0;
        }

        vtkSmartPointer<vtkPlane> newClipPlane =
            vtkSmartPointer<vtkPlane>::New();

        if (safeAxis == 0)
            newClipPlane->SetNormal(1.0, 0.0, 0.0);
        else if (safeAxis == 1)
            newClipPlane->SetNormal(0.0, 1.0, 0.0);
        else
            newClipPlane->SetNormal(0.0, 0.0, 1.0);

        double bounds[6];
        currentData->GetBounds(bounds);

        double minBound = bounds[safeAxis * 2];
        double maxBound = bounds[safeAxis * 2 + 1];

        double clipFraction = clipFilterValue / 100.0;
        double clipPosition = minBound + clipFraction * (maxBound - minBound);

        double origin[3] = {
            (bounds[0] + bounds[1]) / 2.0,
            (bounds[2] + bounds[3]) / 2.0,
            (bounds[4] + bounds[5]) / 2.0
        };

        origin[safeAxis] = clipPosition;
        newClipPlane->SetOrigin(origin);

        vtkSmartPointer<vtkClipDataSet> newClipFilter =
            vtkSmartPointer<vtkClipDataSet>::New();

        newClipFilter->SetInputData(currentData);
        newClipFilter->SetClipFunction(newClipPlane);
        newClipFilter->SetInsideOut(invertClipFilter);
        newClipFilter->Update();

        currentData = vtkDataSet::SafeDownCast(
            newClipFilter->GetOutputDataObject(0)
        );

        if (!currentData)
        {
            return nullptr;
        }
    }

    // Store the final filtered data so it stays alive while VR is running.
    vrFilteredData = currentData;

    vrMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    vrMapper->SetInputData(vrFilteredData);

    vtkSmartPointer<vtkActor> newActor =
        vtkSmartPointer<vtkActor>::New();

    newActor->SetMapper(vrMapper);

    // Share the visual properties with the GUI actor.
    // This means colour changes made in the GUI are copied into VR.
    newActor->SetProperty(actor->GetProperty());

    // Copy current visibility state.
    newActor->SetVisibility(isVisible);

    // Store VR actor so later visibility changes can be applied live.
    vrActor = newActor;

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

