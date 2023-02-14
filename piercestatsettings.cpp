#include "piercestatsettings.h"
#include "ui_piercestatsettings.h"

PierceStatSettings::PierceStatSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PierceStatSettings)
{
    ui->setupUi(this);
}

PierceStatSettings::~PierceStatSettings()
{
    delete ui;
}
