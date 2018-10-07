#ifndef SIZEWINDOW_H
#define SIZEWINDOW_H

#include <QDialog>
#include <QScopedPointer>

namespace Ui
{
    class SizeWindow;
}

class SizeWindow : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(SizeWindow)

public:
    explicit SizeWindow(size_t, Qt::CheckState, QWidget * parent = nullptr);
    ~SizeWindow();

private slots:
    void on_OKbutton_clicked();
    void on_Cbutton_clicked();

signals:
    void changeSize(int32_t);
    void determinantOnly(Qt::CheckState);

private:
    enum WindowSize
    {
        MINIMUM_WIDTH = 250,
        MINIMUM_HEIGHT = 150
    };

    QScopedPointer<Ui::SizeWindow> m_ui;
};

#endif // SIZEWINDOW_H
