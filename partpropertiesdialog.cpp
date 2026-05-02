#include "partpropertiesdialog.h"
#include "ui_partpropertiesdialog.h"
#include <QPushButton>
#include <QColorDialog>

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
    , selectedColor(Qt::white)
{
    ui->setupUi(this);

    connect(ui->removeSelected, &QPushButton::clicked,
        this, &PartPropertiesDialog::requestRemove);

    connect(ui->colorButton, &QPushButton::clicked,
        this, &PartPropertiesDialog::onColorButtonClicked);
}

void PartPropertiesDialog::updateColorButton()
{
    // Set the button background to the chosen colour so the user can see it.
    // Use black or white text depending on brightness so it stays readable.
    int brightness = (selectedColor.red() * 299 +
                      selectedColor.green() * 587 +
                      selectedColor.blue() * 114) / 1000;
    QString textColor = (brightness > 128) ? "black" : "white";

    ui->colorButton->setStyleSheet(
        QString("background-color: %1; color: %2;")
            .arg(selectedColor.name())
            .arg(textColor)
    );
    ui->colorButton->setText(selectedColor.name().toUpper());
}

void PartPropertiesDialog::onColorButtonClicked()
{
    QColor chosen = QColorDialog::getColor(selectedColor, this, "Choose Colour");
    if (chosen.isValid())
    {
        selectedColor = chosen;
        updateColorButton();
    }
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

    // Colour — load the part's current RGB into the picker button
    selectedColor = QColor(currentPart->getColourR(),
                           currentPart->getColourG(),
                           currentPart->getColourB());
    updateColorButton();

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
            static_cast<unsigned char>(selectedColor.red()),
            static_cast<unsigned char>(selectedColor.green()),
            static_cast<unsigned char>(selectedColor.blue())
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
