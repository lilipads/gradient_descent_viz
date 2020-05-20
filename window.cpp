#include <QtWidgets>

#include "window.h"

Window::Window(QWidget *parent)
 : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Gradient Descent Visualization"));

    Q3DSurface *graph = new Q3DSurface();
    QWidget *container = QWidget::createWindowContainer(graph);

    QSize screenSize = graph->screen()->size();
    container->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.5));
    container->setMaximumSize(screenSize);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setFocusPolicy(Qt::StrongFocus);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    QVBoxLayout *vLayout = new QVBoxLayout();
    hLayout->addWidget(container, 1);
    hLayout->addLayout(vLayout);

    plot = new Plot(graph);

    vLayout->addWidget(createToggleAnimationButton());
    vLayout->addWidget(createRestartAnimationButton());
    vLayout->addWidget(createGradientDescentGroup(), 1, Qt::AlignTop);
//    vLayout->addWidget(createLearningRateBox(), 1, Qt::AlignTop);
}


QPushButton *Window::createToggleAnimationButton(){
    // toggle animation button
    QPushButton *toggleAnimationButton = new QPushButton(this);

    toggleAnimationButton->setText(QStringLiteral("Toggle animation"));
//    toggleAnimationButton->setCheckable(true);
//    toggleAnimationButton->setChecked((true));
    QObject::connect(toggleAnimationButton, &QPushButton::clicked, plot,
                     &Plot::toggleAnimation);
    return toggleAnimationButton;
}


QPushButton *Window::createRestartAnimationButton(){
    // restart animation button
    QPushButton *restartAnimationButton = new QPushButton(this);
    restartAnimationButton->setText(QStringLiteral("Restart"));
    QObject::connect(restartAnimationButton, &QPushButton::clicked, plot,
                     &Plot::restartAnimation);
    return restartAnimationButton;
}


QGroupBox *Window::createGradientDescentGroup(){
    QGroupBox *groupBox = new QGroupBox(tr("&Gradient Descent"));
    groupBox->setCheckable(true);
    groupBox->setChecked(true);

    GradientDescent* descent = plot->gradient_descent.get();

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(new QLabel(QStringLiteral("Learning Rate:")));
    vbox->addWidget(createLearningRateBox(descent));
    vbox->addStretch(1);
    groupBox->setLayout(vbox);

    return groupBox;
}

QDoubleSpinBox *Window::createLearningRateBox(GradientDescent* descent){
    // learning rate spin box
    QDoubleSpinBox *learningRateBox = new QDoubleSpinBox(this);
    learningRateBox->setDecimals(4);
    learningRateBox->setRange(0.0001, 1.0);
    learningRateBox->setValue(0.01);
    learningRateBox->setSingleStep(0.001);
    descent->learning_rate = 0.01;
    QObject::connect(learningRateBox,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=]( const double &newValue ) {
            descent->learning_rate = newValue;
        });
    return learningRateBox;
}


