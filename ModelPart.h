/**
 * @file ModelPart.h
 * @brief Defines the ModelPart class used to represent CAD model parts in the tree view and VTK renderer.
 */

#ifndef VIEWER_MODELPART_H
#define VIEWER_MODELPART_H

#include <QString>
#include <QList>
#include <QVariant>

#include <vtkSmartPointer.h>
#include <vtkMapper.h>
#include <vtkActor.h>
#include <vtkSTLReader.h>
#include <vtkColor.h>

/**
 * @brief Represents one CAD model part in the application.
 *
 * Stores tree hierarchy information, display properties such as colour and
 * visibility, and the VTK objects needed to render an STL model.
 */
class ModelPart {
public:

    /**
     * @brief Creates a new model part.
     * @param data Item data such as name and visibility.
     * @param parent Parent item in the tree, or nullptr for the root item.
     */
    ModelPart(const QList<QVariant>& data, ModelPart* parent = nullptr);

    /**
     * @brief Deletes the model part and its child items.
     */
    ~ModelPart();

    /**
     * @brief Adds a child item to this model part.
     * @param item Pointer to the child item to add.
     */
    void appendChild(ModelPart* item);

    /**
     * @brief Gets the child item at a given row.
     * @param row Row index of the child item.
     * @return Pointer to the child item.
     */
    ModelPart* child(int row);

    /**
     * @brief Gets the number of child items.
     * @return Number of children below this item.
     */
    int childCount() const;

    /**
     * @brief Gets the number of data columns.
     * @return Number of columns used by the tree model.
     */
    int columnCount() const;

    /**
     * @brief Gets the data stored in a given column.
     * @param column Column index.
     * @return Data value for that column.
     */
    QVariant data(int column) const;

    /**
     * @brief Sets the data stored in a given column.
     * @param column Column index.
     * @param value New value to store.
     */
    void set(int column, const QVariant& value);

    /**
     * @brief Gets the parent item.
     * @return Pointer to the parent ModelPart.
     */
    ModelPart* parentItem();

    /**
     * @brief Gets this item's row number relative to its parent.
     * @return Row index of this item.
     */
    int row() const;

    /**
     * @brief Sets the RGB colour of the model part.
     * @param R Red value from 0 to 255.
     * @param G Green value from 0 to 255.
     * @param B Blue value from 0 to 255.
     */
    void setColour(const unsigned char R, const unsigned char G, const unsigned char B);

    /**
     * @brief Gets the red colour value.
     * @return Red value from 0 to 255.
     */
    unsigned char getColourR();

    /**
     * @brief Gets the green colour value.
     * @return Green value from 0 to 255.
     */
    unsigned char getColourG();

    /**
     * @brief Gets the blue colour value.
     * @return Blue value from 0 to 255.
     */
    unsigned char getColourB();

    /**
     * @brief Sets whether the model part is visible.
     * @param isVisible True to show the part, false to hide it.
     */
    void setVisible(bool isVisible);

    /**
     * @brief Gets whether the model part is visible.
     * @return True if visible, false otherwise.
     */
    bool visible();

    /**
     * @brief Loads an STL file for this model part.
     * @param fileName Path to the STL file.
     */
    void loadSTL(QString fileName);

    /**
     * @brief Gets the actor used for normal GUI rendering.
     * @return VTK actor for the main application renderer.
     */
    vtkSmartPointer<vtkActor> getActor();

    /**
     * @brief Creates a separate actor for VR rendering.
     * @return New VTK actor for the VR renderer.
     */
    vtkSmartPointer<vtkActor> getNewActor();

    /**
     * @brief Removes a child item at a given row.
     * @param row Row index of the child to remove.
     * @return True if the child was removed successfully.
     */
    bool removeChild(int row);

/**
 * @brief Sets the shrink filter used for rendering.
 * @param enabled True to enable the filter, false to disable it.
 * @param factor Shrink factor applied to the model.
 */
void setShrinkFilter(bool enabled, double factor);

/**
 * @brief Checks whether the shrink filter is enabled.
 * @return True if the shrink filter is enabled.
 */
bool shrinkFilterEnabled() const;

/**
 * @brief Gets the shrink filter factor.
 * @return Current shrink factor.
 */
double shrinkFactor() const;

/**
 * @brief Updates the main VTK rendering pipeline after filter/property changes.
 */
void updatePipeline();

/**
 * @brief Updates the VR rendering pipeline after filter/property changes.
 */
void updateVRPipeline();

/**
 * @brief Sets the clip filter used for rendering.
 * @param enabled True to enable the filter, false to disable it.
 * @param axis Axis used for clipping.
 * @param value Clip position along the selected axis.
 * @param invert True to invert the clipped region.
 */
void setClipFilter(bool enabled, int axis, double value, bool invert);

/**
 * @brief Checks whether the clip filter is enabled.
 * @return True if the clip filter is enabled.
 */
bool clipFilterEnabled() const;

/**
 * @brief Gets the axis used by the clip filter.
 * @return Clip axis index.
 */
int clipAxis() const;

/**
 * @brief Gets the clip filter value.
 * @return Clip position value.
 */
double clipValue() const;

/**
 * @brief Checks whether the clip filter is inverted.
 * @return True if the clipped region is inverted.
 */
bool clipInvert() const;

private:

    /** Child items stored below this model part in the tree. */
    QList<ModelPart*> m_childItems;

    /** Data shown in the tree view, such as name and visibility. */
    QList<QVariant> m_itemData;

    /** Pointer to the parent item in the tree. */
    ModelPart* m_parentItem;

    /** Stores whether the part should be shown in the renderer. */
    bool isVisible;

    /** Red colour component from 0 to 255. */
    unsigned char colourR;

    /** Green colour component from 0 to 255. */
    unsigned char colourG;

    /** Blue colour component from 0 to 255. */
    unsigned char colourB;
	
	/** VTK objects used to load and render this model part. */
    vtkSmartPointer<vtkSTLReader>               file;               /**< Datafile from which part loaded */
    vtkSmartPointer<vtkMapper>                  mapper;             /**< Mapper for rendering */
    vtkSmartPointer<vtkActor>                   actor;              /**< Actor for rendering */
    vtkSmartPointer<vtkActor>                   vrActor;            /**< Separate actor used for VR rendering */
    vtkColor3<unsigned char>                    colour;             /**< User defineable colour */

	/** Stores whether the shrink filter is enabled. */
bool applyShrinkFilter;

/** Factor used by the shrink filter. */
double shrinkFilterFactor;

/** VTK shrink filter used to reduce model geometry size. */
vtkSmartPointer<vtkShrinkFilter> shrinkFilter;

/** Stores whether the clip filter is enabled. */
bool applyClipFilter;

/** Axis used by the clip filter. */
int clipFilterAxis;

/** Position value used by the clip filter. */
double clipFilterValue;

/** Stores whether the clip filter output should be inverted. */
bool invertClipFilter;

/** VTK clip filter used to cut model geometry. */
vtkSmartPointer<vtkClipDataSet> clipFilter;

/** Plane used to define the clipping position and direction. */
vtkSmartPointer<vtkPlane> clipPlane;

/** Mapper used for filtered VR geometry. */
vtkSmartPointer<vtkDataSetMapper> vrMapper;

/** Filtered data used for VR rendering. */
vtkSmartPointer<vtkDataSet> vrFilteredData;

};  


#endif
