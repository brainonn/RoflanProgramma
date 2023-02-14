#ifndef PIERCESTATSETTINGS_H
#define PIERCESTATSETTINGS_H

#include <QDialog>

namespace Ui {
class PierceStatSettings;
}

class PierceStatSettings : public QDialog
{
    Q_OBJECT

public:
    explicit PierceStatSettings(QWidget *parent = nullptr);
    ~PierceStatSettings();

private:
    Ui::PierceStatSettings *ui;
};

#endif // PIERCESTATSETTINGS_H
