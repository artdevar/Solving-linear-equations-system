#ifndef LINEAREQUATION_H
#define LINEAREQUATION_H

#include <boost\numeric\ublas\matrix.hpp>
#include <QVector>
#include <QScopedPointer>
#include <QMainWindow>

using namespace boost::numeric;

namespace Ui 
{
    class LinearEquation;
}

class LinearEquation : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(LinearEquation)

public:
    explicit LinearEquation();
    ~LinearEquation();

private slots:
    void on_actionSettings_triggered();
    void on_actionClean_triggered();
    void on_actionSolve_triggered();
    void changeMatrixSize(int32_t);
    void determinantOnlyChecked(Qt::CheckState);

private:
    enum Colors
    {
        MEMBERS,
        VECTOR,
        DEFAULT,
        COLORS_COUNT
    };

    enum Errors
    {
        NO_ERRORS,
        ZERO_DETERMINANT,
        CLEAR_TABLE,
        BAD_DATA,
    };

    enum WindowSize
    {
        MINIMUM_WIDTH = 262,
        MINIMUM_HEIGHT = 157
    };

    QScopedPointer<Ui::LinearEquation> m_interface; // no parent

    int32_t m_scale = 0;
    Qt::CheckState m_determinantOnly = Qt::Unchecked;

    mutable ublas::matrix<float_t> m_matrix;
    mutable QVector<float_t> m_vector;

    const QColor m_colors[COLORS_COUNT] = {{50, 50, 65}, {65, 50, 50}, {50, 50, 50}}; // rgb

// Private functions
    void FillTable();
    Errors ReadTable() const;

    void MakeColumnReadOnly(uint8_t);
    void MakeColumnWriteable(uint8_t);

    QVector<double_t> MakeSolution();
    double_t FindDeterminant() const;

    template <typename T>
    double_t FindDeterminant(T &&) const; // universal reference 

    ublas::matrix<float_t> SwapColumn(int32_t) const;

    void ResizeWindow();
};

#endif // LINEAREQUATION_H