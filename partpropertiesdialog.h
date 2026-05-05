/**
 * @file partpropertiesdialog.h
 * @brief Defines the PartPropertiesDialog class used to edit properties
 *        of a selected ModelPart.
 */

#ifndef PARTPROPERTIESDIALOG_H
#define PARTPROPERTIESDIALOG_H

#include <QDialog>
#include <QColor>
#include "ModelPart.h"

namespace Ui {
class PartPropertiesDialog;
}

/**
 * @brief Dialog window used to modify properties of a model part.
 *
 * Allows the user to change properties such as colour and visibility
 * of a selected ModelPart.
 */
class PartPropertiesDialog : public QDialog
{
    Q_OBJECT

public:

    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit PartPropertiesDialog(QWidget *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~PartPropertiesDialog();

    /**
     * @brief Sets the model part to be edited in the dialog.
     * @param part Pointer to the ModelPart
     */
    void setModelPart(ModelPart *part);

     /**
     * @brief Checks whether the user requested removal of the model part.
     * @return True if removal was requested.
     */
    bool removalRequested() const;

protected:

    /**
     * @brief Called when the dialog is accepted (e.g. OK button pressed).
     *
     * Applies any changes made in the dialog to the selected model part.
     */
    void accept() override;

private slots:
     /**
     * @brief Slot triggered when the user requests to remove the model part.
     */
    void requestRemove();

      /**
      * @brief Opens a colour picker to select a new part colour.
      */
    void onColorButtonClicked();

private:

    /** Pointer to the UI components of the dialog */
    Ui::PartPropertiesDialog *ui;

    /** Pointer to the currently selected model part */
    ModelPart *currentPart;

    /** Stores whether the user has requested removal of the model part. */
    bool removeItemRequested;

    /** Stores the currently selected colour for the model part. */
    QColor selectedColor;

    /**
    * @brief Updates the colour button to reflect the selected colour.
    */
    void updateColorButton();
};

#endif // PARTPROPERTIESDIALOG_H
