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
 * @brief Represents one CAD model part in the application (TEST!!!!).
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

    /** Reader used to load the STL file. */
    vtkSmartPointer<vtkSTLReader> file;

    /** Mapper used to connect STL data to the actor. */
    vtkSmartPointer<vtkMapper> mapper;

    /** Actor used to display the model part in the scene. */
    vtkSmartPointer<vtkActor> actor;

    /** VTK colour value used for rendering the part. */
    vtkColor3<unsigned char> colour;
};

#endif
