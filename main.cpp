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

#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QMessageBox>
#include <QtGui/QScreen>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Q3DSurface *graph = new Q3DSurface();
    QWidget *container = QWidget::createWindowContainer(graph);

    if (!graph->hasContext()) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize the OpenGL context.");
        msgBox.exec();
        return -1;
    }

    QSize screenSize = graph->screen()->size();
    container->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.5));
    container->setMaximumSize(screenSize);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setFocusPolicy(Qt::StrongFocus);

    QWidget *widget = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    QVBoxLayout *vLayout = new QVBoxLayout();
    hLayout->addWidget(container, 1);
    hLayout->addLayout(vLayout);

    widget->setWindowTitle(QStringLiteral("Gradient Descent Visualization"));
    Plot *plot = new Plot(graph);

    // toggle animation button
    QPushButton *toggleAnimationButton = new QPushButton(widget);
    toggleAnimationButton->setText(QStringLiteral("Toggle animation"));
    vLayout->addWidget(toggleAnimationButton);
    QObject::connect(toggleAnimationButton, &QPushButton::clicked, plot,
                     &Plot::toggleAnimation);

    // restart animation button
    QPushButton *restartAnimationButton = new QPushButton(widget);
    restartAnimationButton->setText(QStringLiteral("Restart animation"));
    vLayout->addWidget(restartAnimationButton);
    QObject::connect(restartAnimationButton, &QPushButton::clicked, plot,
                     &Plot::restartAnimation);

    // learning rate spin box
    QDoubleSpinBox *learningRateBox = new QDoubleSpinBox(widget);
    learningRateBox->setDecimals(4);
    learningRateBox->setRange(0.0001, 1.0);
    learningRateBox->setValue(0.01);
    learningRateBox->setSingleStep(0.001);
    plot->setLearningRate(0.01);
    vLayout->addWidget(new QLabel(QStringLiteral("Learning Rate:")));
    vLayout->addWidget(learningRateBox, 1, Qt::AlignTop);
    QObject::connect(learningRateBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), plot,
                     &Plot::setLearningRate);

    widget->show();
    return app.exec();
}
