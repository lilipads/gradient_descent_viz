#ifndef PLOT_H
#define PLOT_H

#include <memory>
#include <vector>

#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtDataVisualization/Q3DSurface>
#include <QtCore/QTimer>

#include "gradient_descent.h"
#include "animation.h"


class PlotArea : public QObject
{
    Q_OBJECT
public:
    explicit PlotArea(Q3DSurface *surface);
    ~PlotArea();
    std::unique_ptr<Animation> gradient_descent;
    std::unique_ptr<Animation> momentum;
    std::unique_ptr<Animation> qhm;
    std::unique_ptr<Animation> ada_grad;
    std::unique_ptr<Animation> rms_prop;
    std::unique_ptr<Animation> adam;
    std::unique_ptr<Animation> qhadam;
    std::vector<Animation *> all_animations;

signals:
    void updateMessage(QString message);

public Q_SLOTS:
    void pauseAnimation();
    void playAnimation();
    void triggerAnimation();
    void resetAnimations();
    void setAnimationMode(const int& view_type);
    void setDetailedAnimation(QString descent_name);
    void setAnimationSpeed(int index);
    void restartFromClickedPosition(QPoint q_pos);
    void moveCamera(int x_direction, int z_direction);
    void cameraZoomIn();
    void cameraZoomOut();
    void setCameraZoom(float zoom);
    void setShowGradient(bool show);
    void setShowAdjustedGradient(bool show);
    void setShowMomentum(bool show);
    void setShowGradientSquared(bool show);
    void setShowPath(bool show);
    void changeSurface(QString name);


private:
    QTimer m_timer;
    std::unique_ptr<Q3DSurface> m_graph;
    std::unique_ptr<QSurfaceDataProxy> m_surfaceProxy;
    std::unique_ptr<QSurface3DSeries> m_surfaceSeries;
    bool detailedView = false;
    Animation* detailed_descent = nullptr;

    int timer_counter = 0;
    int animation_slowdown = 1; // slow down factor
    int animation_speedup = 1;  // speed up factor
    bool show_gradient = false;
    bool show_adjusted_gradient = false;
    bool show_momentum = false;
    bool show_gradient_squared = false;
    bool show_path = false;

    void initializeSurface();
    void initializeAxes();
    void initializeAnimations();
};

#endif // PLOT_H
