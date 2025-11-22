#pragma once

#include <QDialog>

class QLineEdit;

class AppearanceDialog : public QDialog {
    Q_OBJECT
public:
    explicit AppearanceDialog(QWidget* parent = nullptr);

private slots:
    void chooseLoginBackground();
    void chooseMainBackground();
    void resetLoginBackground();
    void resetMainBackground();

private:
    void updatePreviewFields();
    QLineEdit* loginPathEdit;
    QLineEdit* mainPathEdit;
};
