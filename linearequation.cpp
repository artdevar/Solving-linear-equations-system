#include "linearequation.h"
#include "ui_linearequation.h"
#include "sizewindow.cpp"

#include <boost\numeric\ublas\lu.hpp>
#include <QMessageBox>
#include <QFuture>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QThread>
#include <QDebug>


LinearEquation::LinearEquation() : QMainWindow(nullptr), m_interface(new Ui::LinearEquation)
{
    m_interface->setupUi(this);
    m_interface->matrix->verticalHeader()->hide();
    m_interface->matrix->horizontalHeader()->hide();
    on_actionSettings_triggered();
}

// qtablewidget deletes items byself
LinearEquation::~LinearEquation() = default;

void LinearEquation::FillTable()
{
    for(int i = 0; i < m_interface->matrix->rowCount(); ++i)
    {
        for(int j = 0; j < m_interface->matrix->columnCount(); ++j)
        {
            if(!m_interface->matrix->item(i, j))
            {
                QTableWidgetItem * item = new QTableWidgetItem;
                item->setTextAlignment(Qt::AlignCenter);
                m_interface->matrix->setItem(i, j, item);
            }

            // choose correct background color for item in the matrix
            QTableWidgetItem & item = *m_interface->matrix->item(i, j);
            if(j == m_interface->matrix->columnCount() - 1)
            {
                item.setBackgroundColor(m_colors[VECTOR]);
            } else if(item.backgroundColor() != m_colors[MEMBERS])
            {
                item.setBackgroundColor(m_colors[MEMBERS]);
            }
        }
    }
}

LinearEquation::Errors LinearEquation::ReadTable() const
{
    if(!m_scale)
    {
        return CLEAR_TABLE;
    }
    
    bool convertSuccess = true;
    for(int32_t i = 0, j = 0; i < m_scale; ++i, j = 0)
    {
        for(; j < m_scale; ++j)
        {
            m_matrix(i, j) = m_interface->matrix->item(i, j)->data(Qt::DisplayRole).toFloat(&convertSuccess);
            if(!convertSuccess)
            {
                return BAD_DATA;
            }
        }
        m_vector[i] = m_interface->matrix->item(i, j)->data(Qt::DisplayRole).toFloat(&convertSuccess);
        if(!convertSuccess)
        {
            return BAD_DATA;
        }
    }
    return NO_ERRORS;
}

double_t LinearEquation::FindDeterminant() const
{
    auto temporaryMatrix = m_matrix;
    ublas::permutation_matrix<size_t> pivots(temporaryMatrix.size1());
    
    if(ublas::lu_factorize(temporaryMatrix, pivots))
    {
        return 0.0;
    }

    double_t det = 1.0;
    for(size_t i = 0; i < pivots.size(); ++i)
    {
        det *= (pivots(i) != i ? -1.0 : 1.0) * temporaryMatrix(i, i);
    }

    return det;
}

template <typename T>
double_t LinearEquation::FindDeterminant(T && matrix) const
{
    ublas::permutation_matrix<size_t> pivots(matrix.size1());

    if(ublas::lu_factorize(matrix, pivots))
    {
        return 0.0;
    }

    double_t det = 1.0;
    for(size_t i = 0; i < pivots.size(); ++i)
    {
        det *= (pivots(i) != i ? -1.0 : 1.0) * matrix(i, i);
    }

    return det;
}

QVector<double_t> LinearEquation::MakeSolution()
{
    auto readResult = this->ReadTable();
    if(readResult != NO_ERRORS)
    {
        throw readResult;
    }

    const double_t determinant = this->FindDeterminant();
    if(m_determinantOnly)
    {
        return {determinant};
    }
    if(!determinant)
    {
        throw ZERO_DETERMINANT;
    }

    QVector<double_t> result(m_matrix.size2() + 1);
    result[0] = determinant;

    // the half of work performs by the other thread
    auto getResult = QtConcurrent::run([this, &result, determinant]
    {
        for(size_t i = 0; i < m_matrix.size2() / 2; ++i)
        {
            result[i + 1] = this->FindDeterminant(this->SwapColumn(i)) / determinant;
        }
    });

    for(size_t i = m_matrix.size2() / 2; i < m_matrix.size2(); ++i)
    {
        result[i + 1] = this->FindDeterminant(this->SwapColumn(i)) / determinant;
    }

    getResult.waitForFinished();
    return result;
}

ublas::matrix<float_t> LinearEquation::SwapColumn(int32_t column) const
{
    auto temporaryMatrix = m_matrix;

    for(size_t i = 0; i < temporaryMatrix.size1(); ++i)
    {
        temporaryMatrix(i, column) = m_vector.at(i);
    }

    return temporaryMatrix;
}

void LinearEquation::on_actionSettings_triggered()
{
    SizeWindow window(m_scale, m_determinantOnly);
    window.setModal(true);
    connect(&window, &SizeWindow::changeSize,      this, &LinearEquation::changeMatrixSize);
    connect(&window, &SizeWindow::determinantOnly, this, &LinearEquation::determinantOnlyChecked);
    window.exec();
}

void LinearEquation::changeMatrixSize(int32_t size)
{
    if(m_scale == size)
    {
        return;
    }
    if(m_determinantOnly)
    {
        this->MakeColumnWriteable(m_interface->matrix->columnCount());
    }
    m_scale = size;
    this->ResizeWindow();
    m_interface->matrix->setRowCount(m_scale);
    m_interface->matrix->setColumnCount(m_scale + 1);
    this->FillTable();
    m_matrix.resize(m_scale, m_scale);
    m_vector.resize(m_scale);
    this->on_actionClean_triggered();
}

void LinearEquation::determinantOnlyChecked(Qt::CheckState state)
{
    m_determinantOnly = state;
    if(m_determinantOnly == Qt::CheckState::Checked)
    {
        this->MakeColumnReadOnly(m_interface->matrix->columnCount());
    } else
    {
        this->MakeColumnWriteable(m_interface->matrix->columnCount());
    }
}

void LinearEquation::on_actionClean_triggered()
{
    auto matrixClean = QtConcurrent::run([this]
    {
        std::fill(m_matrix.begin1(), m_matrix.end1(), 0.0f);
        std::fill(m_matrix.begin2(), m_matrix.end2(), 0.0f);
    });
    
    QScopedPointer<QThread> vectorClean(QThread::create([this]
    {
        std::fill(m_vector.begin(), m_vector.end(), 0.0f);
    }));
    vectorClean->start(QThread::Priority::HighPriority);

    for(int i = 0; i < m_interface->matrix->rowCount(); ++i)
    {
        for(int j = 0; j < m_interface->matrix->columnCount(); ++j)
        {
            m_interface->matrix->item(i, j)->setData(Qt::DisplayRole, 0.0f);
        }
    }

    matrixClean.waitForFinished();
    if(vectorClean->isRunning())
    {
        vectorClean->wait();
    }
}

void LinearEquation::on_actionSolve_triggered()
{
    QString output;
    try
    {
        auto result = MakeSolution();
        output += "Determinant: ";
        output += QString::number(result.at(0), 'd', 3) + '\n';
        for(int32_t i = 1; i < result.size(); ++i)
        {
            output += " X";
            output.push_back(i + 0x30);
            output += " = " + QString::number(result.at(i), 'd', 3) + '\n';
        }
        QMessageBox::information(this, ("Answer"), output);
    } catch(Errors codeError)
    {
        switch(codeError)
        {
            case ZERO_DETERMINANT:
                output = "The determinant is equal to zero!"; break;
            case CLEAR_TABLE:
                output = "You should fill the table before!"; break;
            case BAD_DATA:
                output = "Incorrect input in the table!";     break;
            default:                                          break;
        }
        QMessageBox::warning(this, ("Failure"), output);
    }
}

void LinearEquation::ResizeWindow()
{
    // autoresize window according to matrix size
    auto width = MINIMUM_WIDTH + (m_scale - 2) * 80;
    auto height = MINIMUM_HEIGHT + (m_scale - 2) * 44;
    this->setMinimumSize(width, height);
    this->resize(width, height);
}

void LinearEquation::MakeColumnReadOnly(uint8_t column)
{
    for(int i = 0; i < m_interface->matrix->rowCount(); ++i)
    {
        m_interface->matrix->item(i, column - 1)->setFlags(Qt::ItemIsUserCheckable);
    }
}

void LinearEquation::MakeColumnWriteable(uint8_t column)
{
    for(int i = 0; i < m_interface->matrix->rowCount(); ++i)
    {
        m_interface->matrix->item(i, column - 1)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    }
}
