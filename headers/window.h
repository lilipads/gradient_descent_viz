#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QComboBox>
#include <QtGui/QScreen>

#include "plot_area.h"

QT_BEGIN_NAMESPACE
class QGroupBox;
QT_END_NAMESPACE

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget *parent = nullptr);

private:
    PlotArea *plot_area;

    void setupKeyboardShortcuts();

    QGroupBox* createControlGroup();
    QPushButton *createZoomButton(int is_zoomout);
    QPushButton* createToggleAnimationButton();
    QPushButton* createRestartAnimationButton();
    QComboBox* createPlaybackSpeedBox();

    QComboBox* createFunctionSelector();
    QTabWidget* createViewTabs();

    QGroupBox* createDescentGroup(Animation* animation,
        QFormLayout* layout);
    QGroupBox* createGradientDescentGroup();
    QGroupBox* createMomentumGroup();
    QGroupBox* createAdaGradGroup();
    QGroupBox* createRMSPropGroup();
    QGroupBox* createAdamGroup();

    QLayout* createLearningRateBox(GradientDescent* descent);
    QDoubleSpinBox* createDecayBox(double& val);

};

#endif

