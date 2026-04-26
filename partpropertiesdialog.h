#ifndef PARTPROPERTIESDIALOG_H
#define PARTPROPERTIESDIALOG_H

#include <QDialog>
#include "ModelPart.h"

namespace Ui {
class PartPropertiesDialog;
}

class PartPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PartPropertiesDialog(QWidget *parent = nullptr);
    ~PartPropertiesDialog();

    void setModelPart(ModelPart *part);

protected:
    void accept() override;

private:
    Ui::PartPropertiesDialog *ui;
    ModelPart *currentPart;
};

#endif // PARTPROPERTIESDIALOG_H
