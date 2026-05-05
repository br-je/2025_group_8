/**     @file partpropertiesdialog.h
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Dialog window for editing the properties of a selected
  *     model part, including colour, visibility, and filters.
  */

#ifndef PARTPROPERTIESDIALOG_H
#define PARTPROPERTIESDIALOG_H

#include <QDialog>
#include <QColor>
#include "ModelPart.h"

namespace Ui {
class PartPropertiesDialog;
}

/** Dialog window used to edit the properties of a selected model part.
  * Allows the user to change the part name, colour, visibility,
  * shrink filter, clip filter, and optionally remove the part.
  */
class PartPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    /** Constructor - sets up the dialog UI.
      * @param parent is the parent widget (usually the MainWindow)
      */
    explicit PartPropertiesDialog(QWidget *parent = nullptr);

    /** Destructor - frees the dialog UI.
      */
    ~PartPropertiesDialog();

    /** Set the model part whose properties will be shown and edited.
      * @param part is a pointer to the ModelPart to edit
      */
    void setModelPart(ModelPart *part);

    /** Check whether the user requested removal of the part.
      * @return true if the user clicked the remove button
      */
    bool removalRequested() const;

protected:
    /** Called when the dialog is accepted (i.e. the OK button is pressed).
      * Applies any changes made in the dialog back to the selected part.
      */
    void accept() override;

private slots:
    /** Called when the user clicks the remove button.
      * Sets the removal flag so the caller knows to delete the part.
      */
    void requestRemove();

    /** Opens a colour picker so the user can choose a new colour for the part.
      */
    void onColorButtonClicked();

private:
    Ui::PartPropertiesDialog *ui;  /**< Pointer to the generated Qt UI for this dialog */
    ModelPart *currentPart;        /**< Pointer to the part currently being edited */
    bool removeItemRequested;      /**< True if the user has requested removal of the part */
    QColor selectedColor;          /**< Stores the colour currently selected by the user */

    /** Update the colour button to show the currently selected colour.
      */
    void updateColorButton();
};

#endif // PARTPROPERTIESDIALOG_H
