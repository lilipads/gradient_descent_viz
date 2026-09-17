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
class QScrollArea;
QT_END_NAMESPACE

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget *parent = nullptr);

private:
    PlotArea *plot_area;
    QWidget *graph_container;
    QGroupBox *control_group;
    QScrollArea *settings_scroll_area;
    QTabWidget *workspace_tabs;
    Qt::WindowStates previous_window_state;
    bool visualization_fullscreen = false;

    void setupKeyboardShortcuts();
    void toggleVisualizationFullscreen();

    QGroupBox* createControlGroup();
    QPushButton *createZoomButton(int is_zoomout);
    QPushButton *createFullscreenButton();
    QPushButton* createToggleAnimationButton();
    QPushButton* createRestartAnimationButton();
    QComboBox* createPlaybackSpeedBox();

    QComboBox* createFunctionSelector();
    QTabWidget* createViewTabs();

    QGroupBox* createDescentGroup(Animation* animation,
        QFormLayout* layout);
    QGroupBox* createGradientDescentGroup();
    QGroupBox* createMomentumGroup();
    QGroupBox *createQHMGroup();
    QGroupBox *createAdaGradGroup();
    QGroupBox* createRMSPropGroup();
    QGroupBox* createAdamGroup();
    QGroupBox* createAdamWGroup();
    QGroupBox *createQHAdamGroup();

    QLayout* createLearningRateBox(GradientDescent* descent);
    QDoubleSpinBox* createDecayBox(double& val);

};

#endif
