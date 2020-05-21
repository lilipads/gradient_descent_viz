#include <QtWidgets>
#include <math.h>
#include <string>

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
    vLayout->addWidget(new QLabel(QStringLiteral("zoom:")));
    vLayout->addWidget(createZoomSlider());

    // set gradient parameters
    vLayout->addWidget(createGradientDescentGroup());
    vLayout->addWidget(createMomentumGroup());
    vLayout->addWidget(createAdaGradGroup());
    vLayout->addWidget(createRMSPropGroup());
    vLayout->addWidget(createAdamGroup(), 1, Qt::AlignTop);
}


QPushButton *Window::createToggleAnimationButton(){
    // toggle animation button
    QPushButton *toggleAnimationButton = new QPushButton(this);

    toggleAnimationButton->setCheckable(true);
    toggleAnimationButton->setChecked(true);
    toggleAnimationButton->setText(QStringLiteral("Pause"));

    QObject::connect(toggleAnimationButton, &QPushButton::toggled,
        [=](bool is_checked){
            if (is_checked)
                toggleAnimationButton->setText(QStringLiteral("Pause"));
            else
                toggleAnimationButton->setText(QStringLiteral("Paused"));
        });

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

QSlider *Window::createZoomSlider(){
    QSlider *zoomSlider = new QSlider(Qt::Horizontal, this);
    zoomSlider->setRange(100, 1000);
    zoomSlider->setValue(120);
    plot->setCameraZoom(120);
    QObject::connect(zoomSlider, &QSlider::valueChanged, plot,
                     &Plot::setCameraZoom);
    return zoomSlider;
}


QGroupBox *Window::createDescentGroup(GradientDescent* descent,
                                      QVBoxLayout* vbox){
    QGroupBox *groupBox = new QGroupBox(tr(descent->name));
    groupBox->setCheckable(true);
    groupBox->setChecked(true);

    QObject::connect(groupBox, &QGroupBox::clicked,
        [=](){
            descent->ball->setVisible(!descent->ball->isVisible());
        });

    groupBox->setStyleSheet(QString("QGroupBox::title {font: 10pt; border-radius: 5px; background: %1;}"
                                    ).arg(descent->ball_color.name()));

    groupBox->setLayout(vbox);
    return groupBox;
}


QGroupBox *Window::createGradientDescentGroup(){
    VanillaGradientDescent* descent = plot->gradient_descent.get();

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(new QLabel(QStringLiteral("Learning Rate:")));
    vbox->addLayout(createLearningRateBox(descent));

    vbox->addStretch(1);
    return createDescentGroup(descent, vbox);
}


QGroupBox *Window::createMomentumGroup(){
    Momentum* descent = plot->momemtum.get();

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(new QLabel(QStringLiteral("Learning Rate:")));
    vbox->addLayout(createLearningRateBox(descent));

    vbox->addWidget(new QLabel(QStringLiteral("Momentum:")));
    vbox->addWidget(createMomentumBox(descent));

    vbox->addStretch(1);
    return createDescentGroup(descent, vbox);
}


QGroupBox *Window::createAdaGradGroup(){
    AdaGrad* descent = plot->ada_grad.get();

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(new QLabel(QStringLiteral("Learning Rate:")));
    vbox->addLayout(createLearningRateBox(descent));

    vbox->addStretch(1);
    return createDescentGroup(descent, vbox);
}


QGroupBox *Window::createRMSPropGroup(){
    RMSProp* descent = plot->rms_prop.get();

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(new QLabel(QStringLiteral("Learning Rate:")));
    vbox->addLayout(createLearningRateBox(descent));

    vbox->addWidget(new QLabel(QStringLiteral("Decay Rate:")));
    vbox->addWidget(createDecayBox(descent->decay_rate));

    vbox->addStretch(1);
    return createDescentGroup(descent, vbox);
}


QGroupBox *Window::createAdamGroup(){
    Adam* descent = plot->adam.get();

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(new QLabel(QStringLiteral("Learning Rate:")));
    vbox->addLayout(createLearningRateBox(descent));

    vbox->addWidget(new QLabel(QStringLiteral("Beta1 (1st order decay):")));
    vbox->addWidget(createDecayBox(descent->beta1));

    vbox->addWidget(new QLabel(QStringLiteral("Beta2 (2nd order decay):")));
    vbox->addWidget(createDecayBox(descent->beta2));

    vbox->addStretch(1);
    return createDescentGroup(descent, vbox);
}


QLayout *Window::createLearningRateBox(GradientDescent* descent){
    // learning rate spin box
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel(QStringLiteral("1e")));

    QSpinBox *learningRateBox = new QSpinBox(this);
    learningRateBox->setRange(-10, 10);
    learningRateBox->setValue(int(log(descent->learning_rate) / log(10)));
    QObject::connect(learningRateBox,
        QOverload<int>::of(&QSpinBox::valueChanged),
        [=](const int &newValue) {
            descent->learning_rate = pow(10, newValue);
        });

    hbox->addWidget(learningRateBox, 1, Qt::AlignLeft);

    return hbox;
}


QDoubleSpinBox *Window::createMomentumBox(Momentum* descent){
    QDoubleSpinBox *momentumBox = new QDoubleSpinBox(this);
    momentumBox->setDecimals(1);
    momentumBox->setRange(0.1, 2.0);
    momentumBox->setValue(descent->momentum);
    momentumBox->setSingleStep(0.1);
    QObject::connect(momentumBox,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=]( const double &newValue ) {
            descent->momentum = newValue;
        });
    return momentumBox;
}

QDoubleSpinBox *Window::createDecayBox(double& val){
    // learning rate spin box
    QDoubleSpinBox *decayRateBox = new QDoubleSpinBox(this);
    decayRateBox->setDecimals(3);
    decayRateBox->setRange(0.0, 1.0);
    decayRateBox->setValue(val);
    decayRateBox->setSingleStep(0.1);
    QObject::connect(decayRateBox,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [&](const double &newValue ) {val = newValue;});
    return decayRateBox;
}


