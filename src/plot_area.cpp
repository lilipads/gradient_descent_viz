#include "plot_area.h"

#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtCore/qmath.h>

using namespace QtDataVisualization;

const int sampleCountX = 51;
const int sampleCountZ = 51;
const float kCameraMoveStepSize = 0.1f;
const float kCameraZoomStepSize = 10.f;
const float minX = -2.0f;
const float maxX = 2.0f;
const float minZ = -2.0f;
const float maxZ = 2.0f;

PlotArea::PlotArea(Q3DSurface *surface)
    : m_graph(surface),
      m_surfaceProxy(new QSurfaceDataProxy()),
      m_surfaceSeries(new QSurface3DSeries(m_surfaceProxy.get()))
{
    initializeAxes();
    initializeAnimations();
    // should be called after animations are initialized because it needs
    // to reset the starting points
    initializeSurface();

    QObject::connect(&m_timer, &QTimer::timeout, this,
                     &PlotArea::triggerAnimation);

    // restart animation from selected position on mouse click
    QObject::connect(m_surfaceSeries.get(),
                     &QSurface3DSeries::selectedPointChanged,
                     this, &PlotArea::restartFromClickedPosition);
    resetAnimations();
    playAnimation();
}

PlotArea::~PlotArea(){}


void PlotArea::initializeAxes(){
    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    m_graph->activeTheme()->setType(Q3DTheme::Theme(2));
    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFrontHigh);
    QValue3DAxis* xAxis = new QValue3DAxis;
    xAxis->setTitle("X");
    xAxis->setTitleVisible(true);
    m_graph->setAxisX(xAxis);
    QValue3DAxis* yAxis = new QValue3DAxis;
    yAxis->setTitle("Z");
    yAxis->setTitleVisible(true);
    m_graph->setAxisY(yAxis);
    QValue3DAxis* zAxis = new QValue3DAxis;
    zAxis->setTitle("Y");
    zAxis->setTitleVisible(true);
    m_graph->setAxisZ(zAxis);

    // surface look
    m_surfaceSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
    m_surfaceSeries->setFlatShadingEnabled(false);
    m_surfaceSeries->setBaseColor(QColor( 100, 0, 0, 255 ));
    // surface color gradient
    QLinearGradient gr;
    gr.setColorAt(1.0, Qt::darkGreen);
    gr.setColorAt(0.3, Qt::yellow);
    gr.setColorAt(0.1, Qt::red);
    gr.setColorAt(0.0, Qt::darkRed);
    m_surfaceSeries->setBaseGradient(gr);
    m_surfaceSeries->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
    m_graph->addSeries(m_surfaceSeries.get());
}


void PlotArea::initializeAnimations(){
    gradient_descent = std::unique_ptr<GradientDescentAnimation>(
                new GradientDescentAnimation(m_graph.get(), &m_timer));
    momentum = std::unique_ptr<Animation>(
                new MomentumAnimation(m_graph.get(), &m_timer));
    ada_grad = std::unique_ptr<Animation>(
                new AdaGradAnimation(m_graph.get(), &m_timer));
    rms_prop = std::unique_ptr<Animation>(
                new RMSPropAnimation(m_graph.get(), &m_timer));
    adam = std::unique_ptr<Animation>(
                new AdamAnimation(m_graph.get(), &m_timer));
    all_animations = {
        gradient_descent.get(),
        momentum.get(),
        ada_grad.get(),
        rms_prop.get(),
        adam.get()
    };
}


void PlotArea::initializeSurface() {
    float stepX = (maxX - minX) / float(sampleCountX - 1);
    float stepZ = (maxZ - minZ) / float(sampleCountZ - 1);

    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    dataArray->reserve(sampleCountZ);
    for (int i = 0 ; i < sampleCountZ ; i++) {
        QSurfaceDataRow *newRow = new QSurfaceDataRow(sampleCountX);
        // Keep values within range bounds, since just adding step can cause minor drift due
        // to the rounding errors.
        float z = qMin(maxZ, (i * stepZ + minZ));
        int index = 0;
        for (int j = 0; j < sampleCountX; j++) {
            float x = qMin(maxX, (j * stepX + minX));
            float y = GradientDescent::f(x, z);
            (*newRow)[index++].setPosition(QVector3D(x, y, z));
        }
        *dataArray << newRow;
    }
    m_surfaceProxy->resetArray(dataArray);

    // make sure starting point is within view port
    for (auto animation : all_animations){
        animation->descent->setStartingPosition(
                    (7 * maxX + minX) / 8, (7 * maxZ + minZ) / 8);
    }
}


void PlotArea::pauseAnimation() {m_timer.stop();}

void PlotArea::playAnimation(){
    if (!m_timer.isActive()) m_timer.start(15);
}


void PlotArea::triggerAnimation() {
    if (timer_counter == 0){
        if (detailedView){
            QString message = detailed_descent->triggerDetailedAnimation(animation_speedup);
            emit updateMessage(message);
        } else{
            for (auto animation : all_animations)
                animation->triggerSimpleAnimation(animation_speedup,
                    show_gradient, show_adjusted_gradient, show_momentum,
                    show_gradient_squared, show_path);
        }
    }
    timer_counter = (timer_counter + 1) % animation_slowdown;
}


void PlotArea::resetAnimations() {
    if (detailedView){
        detailed_descent->resetAnimation();
    } else{
        for (auto& animation : all_animations)
            animation->resetAnimation();
    }
}


void PlotArea::restartFromClickedPosition(QPoint q_pos){
    if (q_pos == QSurface3DSeries::invalidSelectionPosition())
        return;
    // convert the 2d Qt internal point for to the 3d point on the series
    QVector3D p = m_surfaceProxy->itemAt(q_pos)->position();
    for (auto animation : all_animations){
        animation->descent->setStartingPosition(p.x(), p.z());
    }
    resetAnimations();
}


void PlotArea::setCameraZoom(float zoom){
    m_graph->scene()->activeCamera()->setZoomLevel(zoom);
}

void PlotArea::cameraZoomIn(){
    Q3DCamera* camera = m_graph->scene()->activeCamera();
    camera->setZoomLevel(camera->zoomLevel() + kCameraZoomStepSize);
}

void PlotArea::cameraZoomOut(){
    Q3DCamera* camera = m_graph->scene()->activeCamera();
    camera->setZoomLevel(camera->zoomLevel() - kCameraZoomStepSize);
}


void PlotArea::moveCamera(int x_direction, int z_direction){
    QVector3D target = m_graph->scene()->activeCamera()->target();
    float x_rotation = m_graph->scene()->activeCamera()->xRotation();
    x_rotation = qDegreesToRadians(x_rotation);
    target.setX(target.x()
                + x_direction * qCos(x_rotation) * kCameraMoveStepSize
                + z_direction * qSin(x_rotation) * kCameraMoveStepSize);
    target.setZ(target.z()
                - x_direction * qSin(x_rotation) * kCameraMoveStepSize
                + z_direction * qCos(x_rotation) * kCameraMoveStepSize);
    m_graph->scene()->activeCamera()->setTarget(target);
}

void PlotArea::setAnimationSpeed(int index){
    animation_slowdown = 1;
    animation_speedup = 1;
    switch (index) {
        case 0: animation_slowdown = 10; break;
        case 1: animation_slowdown = 5; break;
        case 2: break;
        case 3: animation_speedup = 5; break;
        case 4: animation_speedup = 10; break;
    }
}


void PlotArea::setShowGradient(bool show){
    if (show == show_gradient) return;
    show_gradient = show;
    if (!show){
        for (auto animation : all_animations)
            animation->cleanupGradient();
    }
}


void PlotArea::setShowAdjustedGradient(bool show){
    if (show == show_adjusted_gradient) return;
    show_adjusted_gradient = show;
    if (!show){
        for (auto animation : all_animations)
            animation->cleanupAdjustedGradient();
    }
}


void PlotArea::setShowMomentum(bool show){
    if (show == show_momentum) return;
    show_momentum = show;
    if (!show){
        for (auto animation : all_animations)
            animation->cleanupMomentum();
    }
}


void PlotArea::setShowGradientSquared(bool show){
    if (show == show_gradient_squared) return;
    show_gradient_squared = show;
    if (!show){
        for (auto animation : all_animations)
            animation->cleanupGradientSquared();
    }
}


void PlotArea::setShowPath(bool show){
    if (show == show_path) return;
    show_path = show;
    if (!show){
        for (auto animation : all_animations)
            animation->cleanupPath();
    }
}


void PlotArea::setDetailedAnimation(QString descent_name){
    emit updateMessage("");
    for (auto animation : all_animations){
        if (animation->name == descent_name){
            detailed_descent = animation;
            detailed_descent->resetAnimation();
            for (auto animation : all_animations){
                if (animation != detailed_descent)
                    animation->cleanupAll();
            }
            detailedView = true;
            return;
        }
    }
}


void PlotArea::setAnimationMode(const int& view_type){
    // switch to overview mode
    m_timer.stop();
    if (view_type == 0){
        if (detailed_descent != nullptr)
            detailed_descent->cleanupAll();
        detailedView = false;
        resetAnimations();
    }
    m_timer.start(15);
}


void PlotArea::changeSurface(QString name){
    Function::FunctionName function_name;
    if (name == "Local Minimum"){
        function_name = Function::local_minimum;
    } else if (name == "Global Minimum"){
        function_name = Function::global_minimum;
    } else if (name == "Saddle Point"){
        function_name = Function::saddle_point;
    } else if (name == "Ecliptic Bowl"){
        function_name = Function::ecliptic_bowl;
    } else if (name == "Hills"){
        function_name = Function::hills;
    } else if (name == "Plateau"){
        function_name = Function::plateau;
    }else{
        return;
    }

    GradientDescent::function_name = function_name;
    initializeSurface();
    resetAnimations();
}
