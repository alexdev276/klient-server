#include "serversettingsdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QtMath>

ServerSettingsDialog::ServerSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Параметры сервера");
    setMinimumWidth(420);

    m_taskCombo = new QComboBox(this);
    m_taskCombo->addItem("Вариант 1: площадь sin(x) при Y>0", 1);
    m_taskCombo->addItem("Вариант 4: корни cos³(x)", 4);

    m_aSpin = new QDoubleSpinBox(this);
    m_aSpin->setRange(-1e6, 1e6);
    m_aSpin->setDecimals(6);
    m_aSpin->setValue(0.0);

    m_bSpin = new QDoubleSpinBox(this);
    m_bSpin->setRange(-1e6, 1e6);
    m_bSpin->setDecimals(6);
    m_bSpin->setValue(4.0 * M_PI);

    m_dxSpin = new QDoubleSpinBox(this);
    m_dxSpin->setRange(1e-6, 1.0);
    m_dxSpin->setDecimals(6);
    m_dxSpin->setSingleStep(0.001);
    m_dxSpin->setValue(0.01);

    QFormLayout *form = new QFormLayout;
    form->addRow("Задание:",              m_taskCombo);
    form->addRow("Начало диапазона (A):", m_aSpin);
    form->addRow("Конец диапазона (B):",  m_bSpin);
    form->addRow("Шаг dx (точность):",    m_dxSpin);

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addWidget(buttons);
}

int    ServerSettingsDialog::taskType() const { return m_taskCombo->currentData().toInt(); }
double ServerSettingsDialog::A()  const { return m_aSpin->value();  }
double ServerSettingsDialog::B()  const { return m_bSpin->value();  }
double ServerSettingsDialog::dx() const { return m_dxSpin->value(); }