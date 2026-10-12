#ifndef SERVERSETTINGSDIALOG_H
#define SERVERSETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>

class ServerSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ServerSettingsDialog(QWidget *parent = nullptr);

    int    taskType() const;
    double A()  const;
    double B()  const;
    double dx() const;

private:
    QComboBox      *m_taskCombo;
    QDoubleSpinBox *m_aSpin;
    QDoubleSpinBox *m_bSpin;
    QDoubleSpinBox *m_dxSpin;
};

#endif // SERVERSETTINGSDIALOG_H