#ifndef PLOT_H
#define PLOT_H

#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/qscatterdataproxy.h>
#include <QtCore/QTimer>
#include <gradientdescent.h>
#include <memory>
#include <vector>

using namespace QtDataVisualization;

class Plot : public QObject
{
    Q_OBJECT
public:
    explicit Plot(Q3DSurface *surface);
    ~Plot();
    std::unique_ptr<VanillaGradientDescent> gradient_descent;
    std::unique_ptr<Momentum> momemtum;
    std::unique_ptr<AdaGrad> ada_grad;
    std::unique_ptr<RMSProp> rms_prop;
    std::unique_ptr<Adam> adam;

public Q_SLOTS:
    void toggleAnimation();
    void triggerAnimation();
    void restartAnimation();
    void setCameraZoom(float zoom);

private:
    std::unique_ptr<Q3DSurface> m_graph;
    std::vector<GradientDescent*> all_descents;
    QTimer m_timer;
    std::unique_ptr<QSurfaceDataProxy> m_surfaceProxy;
    std::unique_ptr<QSurface3DSeries> m_surfaceSeries;

    float stepX;
    float stepZ;
    void initializeSurface();
    void initializeGraph();
    void initializeBall(GradientDescent* descent);
    void setBallPosition(QCustom3DItem* ball, Point p);
};

#endif // PLOT_H
