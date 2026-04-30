#include "partpropertiesdialog.h"
#include "ui_partpropertiesdialog.h"
#include <QPushButton>

bool PartPropertiesDialog::removalRequested() const
{
    return removeItemRequested;
}

void PartPropertiesDialog::requestRemove()
{
    removeItemRequested = true;

    // Close the dialog as accepted, but bypass PartPropertiesDialog::accept()
    // so we do not apply name/colour/filter edits before removing the item.
    QDialog::accept();
}

PartPropertiesDialog::PartPropertiesDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PartPropertiesDialog)
    , currentPart(nullptr)
    , removeItemRequested(false)
{
    ui->setupUi(this);

    connect(ui->removeSelected, &QPushButton::clicked,
        this, &PartPropertiesDialog::requestRemove);
}

PartPropertiesDialog::~PartPropertiesDialog()
{
    delete ui;
}

void PartPropertiesDialog::setModelPart(ModelPart *part)
{
    removeItemRequested = false;

    currentPart = part;
    if (!currentPart) return;

    // Column 0 = name
    ui->nameLineEdit->setText(currentPart->data(0).toString());

    // Colour
    ui->redSpinBox->setValue(currentPart->getColourR());
    ui->greenSpinBox->setValue(currentPart->getColourG());
    ui->blueSpinBox->setValue(currentPart->getColourB());

    // Visibility
    ui->visibleCheckBox->setChecked(currentPart->visible());

	//Shrink filter
    ui->shrinkCheckBox->setChecked(currentPart->shrinkFilterEnabled());
    ui->shrinkFactorSpinBox->setValue(currentPart->shrinkFactor());

	//Clip filter
    ui->clipCheckBox->setChecked(currentPart->clipFilterEnabled());
    ui->clipAxisComboBox->setCurrentIndex(currentPart->clipAxis());
    ui->clipValueSpinBox->setValue(currentPart->clipValue());
    ui->clipInvertCheckBox->setChecked(currentPart->clipInvert());
}

void PartPropertiesDialog::accept()
{
    if (currentPart)
    {
        // Name (column 0)
        currentPart->set(0, ui->nameLineEdit->text());

        // Colour
        currentPart->setColour(
            static_cast<unsigned char>(ui->redSpinBox->value()),
            static_cast<unsigned char>(ui->greenSpinBox->value()),
            static_cast<unsigned char>(ui->blueSpinBox->value())
            );

        // Visibility
        currentPart->set(
                       1,
            ui->visibleCheckBox->isChecked() ? "true" : "false");
        currentPart->setVisible(ui->visibleCheckBox->isChecked());


		//Shrink filter
        currentPart->setShrinkFilter(
            ui->shrinkCheckBox->isChecked(),
            ui->shrinkFactorSpinBox->value());


        //Clip filter
        currentPart->setClipFilter(
            ui->clipCheckBox->isChecked(),
            ui->clipAxisComboBox->currentIndex(),
            ui->clipValueSpinBox->value(),
            ui->clipInvertCheckBox->isChecked());
        
    }



    QDialog::accept();
}
