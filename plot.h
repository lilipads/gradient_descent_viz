#ifndef PLOT_H
#define PLOT_H

#include <memory>
#include <vector>

#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtCore/QTimer>

#include "surface.h"
#include "gradientdescent.h"
#include "animation.h"


class Plot : public QObject
{
    Q_OBJECT
public:
    explicit Plot(Surface *surface);
    ~Plot();
    std::unique_ptr<Animation> gradient_descent;
    std::unique_ptr<Animation> momentum;
    std::unique_ptr<Animation> ada_grad;
    std::unique_ptr<Animation> rms_prop;
    std::unique_ptr<Animation> adam;

public Q_SLOTS:
    void toggleAnimation();
    void triggerAnimation();
    void restartAnimation();
    void setAnimationSpeed(int index);
    void setCameraZoom(float zoom);
    void restartFromNewPosition(QPoint q_pos);
    void moveCamera(int x_direction, int z_direction);
    void cameraZoomIn();
    void cameraZoomOut();

private:

    std::vector<Animation*> all_animations;
    QTimer m_timer;
    std::unique_ptr<Surface> m_graph;
    std::unique_ptr<QSurfaceDataProxy> m_surfaceProxy;
    std::unique_ptr<QSurface3DSeries> m_surfaceSeries;
    bool detailedView = true;
    Animation* detailed_descent;

    int timer_counter = 0;
    int animation_slowdown = 1; // slow down factor
    int animation_speedup = 1;  // speed up factor

    void initializeSurface();
    void initializeGraph();
    void initializeAnimations();
};

#endif // PLOT_H
