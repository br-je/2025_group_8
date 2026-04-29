/**
 * @file partpropertiesdialog.h
 * @brief Defines the PartPropertiesDialog class for editing ModelPart properties.
 *
 * Provides a dialog interface that allows users to modify properties such as
 * colour and visibility of a selected model part.
 */
#include "partpropertiesdialog.h"
#include "ui_partpropertiesdialog.h"

PartPropertiesDialog::PartPropertiesDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PartPropertiesDialog)
    ,currentPart(nullptr)
{
    ui->setupUi(this);
}

PartPropertiesDialog::~PartPropertiesDialog()
{
    delete ui;
}

void PartPropertiesDialog::setModelPart(ModelPart *part)
{
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
    }

    QDialog::accept();
}
