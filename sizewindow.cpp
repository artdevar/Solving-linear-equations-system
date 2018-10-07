#include "sizewindow.h"
#include "ui_sizewindow.h"

SizeWindow::SizeWindow(size_t count, Qt::CheckState state, QWidget * parent) : QDialog(parent), 
                                                                               m_ui(new Ui::SizeWindow)
{
    m_ui->setupUi(this);
    this->setFixedSize(MINIMUM_WIDTH, MINIMUM_HEIGHT);
    setWindowFlags(Qt::Drawer | Qt::MSWindowsFixedSizeDialogHint);
    m_ui->variablesCount->setKeyboardTracking(false);
    m_ui->variablesCount->setMinimum(2);
    m_ui->variablesCount->setMaximum(9);

    m_ui->variablesCount->setValue(count);  
    m_ui->checkDeterminantOnly->setChecked(state == Qt::Checked);
}

SizeWindow::~SizeWindow() = default;

void SizeWindow::on_OKbutton_clicked()
{
    emit changeSize(m_ui->variablesCount->value());
    emit determinantOnly(m_ui->checkDeterminantOnly->checkState());
    close();
}

void SizeWindow::on_Cbutton_clicked()
{
    close();
}