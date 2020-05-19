/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Data Visualization module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "plot.h"
#include <QtDataVisualization/qscatterdataproxy.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/qscatter3dseries.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QtDataVisualization/QCustom3DItem>
#include <QtCore/qmath.h>

using namespace QtDataVisualization;

const int sampleCountX = 50;
const int sampleCountZ = 50;
const float sampleMin = -8.0f;
const float sampleMax = 8.0f;

Plot::Plot(Q3DSurface *surface)
    : m_graph(surface),
      m_point(new QCustom3DItem),
      m_surfaceProxy(new QSurfaceDataProxy()),
      m_surfaceSeries(new QSurface3DSeries(m_surfaceProxy.get()))
{
    initializeGraph();
    initializePoint();
    initializeSurface();

    QObject::connect(&m_rotationTimer, &QTimer::timeout, this,
                     &Plot::triggerAnimation);

    toggleAnimation();
}

Plot::~Plot() {}

void Plot::initializeGraph(){
    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);
    m_graph->setAxisX(new QValue3DAxis);
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->setAxisZ(new QValue3DAxis);
}

void Plot::initializePoint(){
    m_point->setScaling(QVector3D(0.01f, 0.01f, 0.01f));
    m_point->setMeshFile(QStringLiteral(":/mesh/largesphere.obj"));
    QImage pointColor = QImage(2, 2, QImage::Format_RGB32);
    pointColor.fill(QColor(0xff, 0xbb, 0x00));
    m_point->setTextureImage(pointColor);
    m_point->setPosition(QVector3D(0, 4, 4));
    m_graph->addCustomItem(m_point.get());
}

void Plot::initializeSurface()
{
    float stepX = (sampleMax - sampleMin) / float(sampleCountX - 1);
    float stepZ = (sampleMax - sampleMin) / float(sampleCountZ - 1);

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
            float y = func.f(x, z);
            (*newRow)[index++].setPosition(QVector3D(x, y, z));
        }
        *dataArray << newRow;
    }

    m_surfaceProxy->resetArray(dataArray);

    // surface look
    m_surfaceSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
    m_surfaceSeries->setFlatShadingEnabled(true);
    m_surfaceSeries->setBaseColor( QColor( 100, 0, 0, 255 ));
    //gradient
    QLinearGradient gr;
    gr.setColorAt(1.0, Qt::darkGreen);
    gr.setColorAt(0.5, Qt::yellow);
    gr.setColorAt(0.2, Qt::red);
    gr.setColorAt(0.0, Qt::darkRed);

    m_surfaceSeries->setBaseGradient(gr);
    m_surfaceSeries->setColorStyle(Q3DTheme::ColorStyleRangeGradient);

    m_graph->addSeries(m_surfaceSeries.get());
}


void Plot::triggerAnimation()
{
    Point p = func.gradientStep();
    m_point->setPosition(QVector3D(p.x, func.f(p.x, p.z), p.z));
}

void Plot::toggleAnimation()
{
    if (m_rotationTimer.isActive())
        m_rotationTimer.stop();
    else
        m_rotationTimer.start(15);
}
