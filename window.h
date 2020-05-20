#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QMessageBox>
#include <QtGui/QScreen>

#include "plot.h"

QT_BEGIN_NAMESPACE
class QGroupBox;
QT_END_NAMESPACE

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget *parent = nullptr);

private:
    QGroupBox* createGradientDescentGroup();
    QGroupBox *createMomentumGroup();
    QPushButton* createToggleAnimationButton();
    QPushButton* createRestartAnimationButton();
    QDoubleSpinBox* createLearningRateBox(GradientDescent* descent);
    QDoubleSpinBox* createMomentumBox(Momentum* descent);
    Plot *plot;
};

#endif

