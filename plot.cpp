#include "plot.h"

#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtCore/qmath.h>

using namespace QtDataVisualization;

const int sampleCountX = 50;
const int sampleCountZ = 50;
const float sampleMin = -8.0f;
const float sampleMax = 8.0f;
const float kCameraMoveStepSize = 0.1f;
const float kCameraZoomStepSize = 10.f;

Plot::Plot(Q3DSurface *surface)
    : gradient_descent(new VanillaGradientDescent),
      momemtum(new Momentum),
      ada_grad(new AdaGrad),
      rms_prop(new RMSProp),
      adam(new Adam),
      m_graph(surface),
      m_surfaceProxy(new QSurfaceDataProxy()),
      m_surfaceSeries(new QSurface3DSeries(m_surfaceProxy.get()))
{
    stepX = (sampleMax - sampleMin) / float(sampleCountX - 1);
    stepZ = (sampleMax - sampleMin) / float(sampleCountZ - 1);

    initializeGraph();

    all_descents.push_back(gradient_descent.get());
    all_descents.push_back(momemtum.get());
    all_descents.push_back(ada_grad.get());
    all_descents.push_back(rms_prop.get());
    all_descents.push_back(adam.get());

    for (auto& descent : all_descents)
        m_graph->addCustomItem(descent->ball.get());
    initializeArrow(gradient_descent.get());

    initializeSurface();

    QObject::connect(&m_timer, &QTimer::timeout, this,
                     &Plot::triggerAnimation);

    // restart animation from selected position on mouse click
    QObject::connect(m_surfaceSeries.get(),
                     &QSurface3DSeries::selectedPointChanged,
                     this, &Plot::restartFromNewPosition);

    toggleAnimation();
    restartAnimation();

    detailed_descent = new GradientDescentAnimation(
                m_graph.get(), &m_timer, gradient_descent.get());
    detailed_descent->prepareDetailedAnimation();
}

Plot::~Plot() {}

void Plot::initializeGraph(){
    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    m_graph->activeTheme()->setType(Q3DTheme::Theme(2));
    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFrontHigh);
    QValue3DAxis* xAxis = new QValue3DAxis;
    xAxis->setTitle("X");
    xAxis->setTitleVisible(true);
    m_graph->setAxisX(xAxis);
    QValue3DAxis* yAxis = new QValue3DAxis;
    yAxis->setTitle("Y");
    yAxis->setTitleVisible(true);
    m_graph->setAxisY(yAxis);
    QValue3DAxis* zAxis = new QValue3DAxis;
    zAxis->setTitle("Z");
    zAxis->setTitleVisible(true);
    m_graph->setAxisZ(zAxis);
}


void Plot::initializeArrow(GradientDescent* descent){  
    m_graph->addCustomItem(descent->arrowX.get());
    m_graph->addCustomItem(descent->arrowZ.get());
}


void Plot::initializeSurface() {
    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    dataArray->reserve(sampleCountZ);
    for (int i = 0 ; i < sampleCountZ ; i++) {
        QSurfaceDataRow *newRow = new QSurfaceDataRow(sampleCountX);
        // Keep values within range bounds, since just adding step can cause minor drift due
        // to the rounding errors.
        float z = qMin(sampleMax, (i * stepZ + sampleMin));
        int index = 0;
        for (int j = 0; j < sampleCountX; j++) {
            float x = qMin(sampleMax, (j * stepX + sampleMin));
            float y = gradient_descent->f(x, z);
            (*newRow)[index++].setPosition(QVector3D(x, y, z));
        }
        *dataArray << newRow;
    }

    m_surfaceProxy->resetArray(dataArray);

    // surface look
    m_surfaceSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
    m_surfaceSeries->setFlatShadingEnabled(false);
    m_surfaceSeries->setBaseColor( QColor( 100, 0, 0, 255 ));
    //gradient
    QLinearGradient gr;
    gr.setColorAt(1.0, Qt::darkGreen);
    gr.setColorAt(0.3, Qt::yellow);
    gr.setColorAt(0.1, Qt::red);
    gr.setColorAt(0.0, Qt::darkRed);
    m_surfaceSeries->setBaseGradient(gr);
    m_surfaceSeries->setColorStyle(Q3DTheme::ColorStyleRangeGradient);

    m_graph->addSeries(m_surfaceSeries.get());
}


void Plot::toggleAnimation() {
    m_timer.isActive() ? m_timer.stop() : m_timer.start(15);
}


void Plot::triggerAnimation() {
    if (timer_counter == 0){
        detailed_descent->triggerAnimation();
//        for (auto& descent : all_descents){
//            // TODO: move this to animation class
//            if (descent->isConverged()) continue;
//            Point p;
//            for (int i = 0; i < animation_speedup; i++)
//                p = descent->takeGradientStep();
//            AnimationHelper::setBallPosition(descent->ball.get(), p);
//            Point grad(descent->gradX(), descent->gradZ());
//            AnimationHelper::setArrowGeometry(descent, grad);
//        }
    }
    timer_counter = (timer_counter + 1) % animation_slowdown;
}


void Plot::restartAnimation() {
    for (auto& descent : all_descents){
        descent->resetPosition();
    }
}


void Plot::restartFromNewPosition(QPoint q_pos){
    if (q_pos == QSurface3DSeries::invalidSelectionPosition())
        return;
    // convert the 2d Qt internal point for to the 3d point on the series
    QVector3D p = m_surfaceProxy->itemAt(q_pos)->position();
    for (auto descent : all_descents){
        descent->setStartingPosition(p.x(), p.z());
    }
    restartAnimation();
}


void Plot::setCameraZoom(float zoom){
    m_graph->scene()->activeCamera()->setZoomLevel(zoom);
}

void Plot::cameraZoomIn(){
    Q3DCamera* camera = m_graph->scene()->activeCamera();
    camera->setZoomLevel(camera->zoomLevel() + kCameraZoomStepSize);
}

void Plot::cameraZoomOut(){
    Q3DCamera* camera = m_graph->scene()->activeCamera();
    camera->setZoomLevel(camera->zoomLevel() - kCameraZoomStepSize);
}


void Plot::moveCamera(int x_direction, int z_direction){
    QVector3D target = m_graph->scene()->activeCamera()->target();
    target.setX(target.x() + x_direction * kCameraMoveStepSize);
    target.setZ(target.z() + z_direction * kCameraMoveStepSize);
    m_graph->scene()->activeCamera()->setTarget(target);
}

void Plot::setAnimationSpeed(int index){
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
