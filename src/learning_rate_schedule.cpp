#include "learning_rate_schedule.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <memory>

#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtWidgets>

#include "gradient_descent.h"
#include "item.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using namespace QtDataVisualization;
#endif

namespace {

enum class ScheduleType {
    Constant,
    Step,
    Cosine,
    Linear,
    InverseSqrt
};

enum class SurfaceType {
    LocalMinimum,
    GlobalMinimum,
    SaddlePoint,
    EllipticBowl,
    Hills,
    Plateau
};

enum class OptimizerType {
    GradientDescent,
    Momentum,
    QHM,
    AdaGrad,
    RMSProp,
    Adam,
    AdamW,
    QHAdam
};

SurfaceType active_surface = SurfaceType::EllipticBowl;

const std::array<ScheduleType, 5> kSchedules = {
    ScheduleType::Constant,
    ScheduleType::Step,
    ScheduleType::Cosine,
    ScheduleType::Linear,
    ScheduleType::InverseSqrt
};

const std::array<QString, 5> kScheduleNames = {
    "Constant", "Step", "Cosine", "Linear", "Inverse SQRT"
};

const std::array<QColor, 5> kScheduleColors = {
    QColor(150, 80, 200),
    QColor(220, 65, 65),
    QColor(33, 150, 243),
    QColor(40, 170, 90),
    QColor(255, 145, 30)
};

struct ScheduleParameters {
    double base_rate = 0.001;
    double end_rate = 0.00001;
    double gamma = 0.5;
    int total_steps = 500;
    int warmup_steps = 50;
    int step_size = 100;
    std::array<bool, 5> visible = {true, true, true, true, true};
};

double scheduleLearningRate(ScheduleType schedule, int step,
                            const ScheduleParameters &parameters)
{
    if (schedule != ScheduleType::Constant
            && parameters.warmup_steps > 0
            && step <= parameters.warmup_steps)
        return parameters.base_rate * step / parameters.warmup_steps;

    const int elapsed = std::max(0, step - parameters.warmup_steps);
    const int decay_steps = std::max(1, parameters.total_steps
                                     - parameters.warmup_steps);
    const double progress = std::min(1.0, elapsed
                                     / static_cast<double>(decay_steps));

    switch (schedule) {
    case ScheduleType::Constant:
        return parameters.base_rate;
    case ScheduleType::Step:
        return parameters.base_rate * std::pow(
                    parameters.gamma,
                    elapsed / std::max(1, parameters.step_size));
    case ScheduleType::Cosine:
        return parameters.end_rate
                + 0.5 * (parameters.base_rate - parameters.end_rate)
                * (1.0 + std::cos(3.14159265358979323846 * progress));
    case ScheduleType::Linear:
        return parameters.base_rate
                + (parameters.end_rate - parameters.base_rate) * progress;
    case ScheduleType::InverseSqrt:
        if (parameters.warmup_steps > 0)
            return parameters.base_rate
                    * std::sqrt(static_cast<double>(parameters.warmup_steps) / step);
        return parameters.base_rate / std::sqrt(1.0 + step);
    }
    return parameters.base_rate;
}

double comparisonSurface(double x, double z)
{
    switch (active_surface) {
    case SurfaceType::LocalMinimum:
        z *= 1.4;
        return -2 * std::exp(-((x - 1) * (x - 1) + z * z) / .2)
                - 6 * std::exp(-((x + 1) * (x + 1) + z * z) / .2)
                + x * x + z * z;
    case SurfaceType::GlobalMinimum:
        return x * x + z * z;
    case SurfaceType::SaddlePoint:
        return std::sin(x) + z * z;
    case SurfaceType::EllipticBowl:
        x /= 2.0;
        z /= 2.0;
        return -std::exp(-(x * x + 5 * z * z)) + x * x + 0.5 * z * z;
    case SurfaceType::Hills:
        z *= 1.4;
        return 2 * std::exp(-((x - 1) * (x - 1) + z * z) / .2)
                + 6 * std::exp(-((x + 1) * (x + 1) + z * z) / .2)
                - 2 * std::exp(-((x - 1) * (x - 1)
                                  + (z + 1) * (z + 1)) / .2)
                + x * x + z * z;
    case SurfaceType::Plateau: {
        x *= 10;
        z *= 10;
        const double radius = std::sqrt(x * x + z * z) + 0.01;
        return -std::sin(radius) / radius + 0.01 * radius * radius;
    }
    }
    return 0.0;
}

class LearningRateChart : public QWidget
{
public:
    explicit LearningRateChart(ScheduleParameters *parameters,
                               QWidget *parent = nullptr)
        : QWidget(parent), parameters(parameters)
    {
        setMinimumSize(420, 220);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), palette().brush(QPalette::Base));

        const QRectF plot(78, 28, width() - 108, height() - 82);
        if (plot.width() <= 0 || plot.height() <= 0)
            return;

        double max_rate = parameters->base_rate;
        for (ScheduleType schedule : kSchedules) {
            for (int step = 0; step <= parameters->total_steps; ++step) {
                max_rate = std::max(max_rate,
                                    scheduleLearningRate(schedule, step, *parameters));
            }
        }
        max_rate = std::max(max_rate, 1e-12);

        if (parameters->warmup_steps > 0) {
            const double warmup_width = plot.width()
                    * parameters->warmup_steps / parameters->total_steps;
            painter.fillRect(QRectF(plot.left(), plot.top(), warmup_width,
                                    plot.height()), QColor(70, 130, 180, 35));
            if (warmup_width > 90) {
                painter.setPen(QColor(70, 130, 180));
                painter.drawText(QRectF(plot.left() + 8, plot.top() + 5,
                                        warmup_width - 16, 20),
                                 Qt::AlignLeft | Qt::AlignTop, "Linear warmup");
            }
        }

        painter.setPen(palette().color(QPalette::Mid));
        for (int i = 0; i <= 5; ++i) {
            const double x = plot.left() + plot.width() * i / 5.0;
            const double y = plot.bottom() - plot.height() * i / 5.0;
            painter.drawLine(QPointF(x, plot.top()), QPointF(x, plot.bottom()));
            painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));

            painter.setPen(palette().color(QPalette::Text));
            painter.drawText(QRectF(x - 35, plot.bottom() + 7, 70, 20),
                             Qt::AlignHCenter | Qt::AlignTop,
                             QString::number(parameters->total_steps * i / 5));
            painter.drawText(QRectF(4, y - 10, 66, 20),
                             Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(max_rate * i / 5.0, 'g', 3));
            painter.setPen(palette().color(QPalette::Mid));
        }

        for (size_t index = 0; index < kSchedules.size(); ++index) {
            if (!parameters->visible[index])
                continue;
            QPainterPath curve;
            for (int step = 0; step <= parameters->total_steps; ++step) {
                const double x = plot.left() + plot.width()
                        * step / parameters->total_steps;
                const double y = plot.bottom() - plot.height()
                        * scheduleLearningRate(kSchedules[index], step, *parameters)
                        / max_rate;
                if (step == 0)
                    curve.moveTo(x, y);
                else
                    curve.lineTo(x, y);
            }
            painter.setPen(QPen(kScheduleColors[index], 3));
            painter.drawPath(curve);
        }

        painter.setPen(QPen(palette().color(QPalette::Text), 1));
        painter.drawRect(plot);
        painter.drawText(QRectF(plot.left(), 2, plot.width(), 22),
                         Qt::AlignCenter, "Learning rate over training steps");
        painter.drawText(QRectF(plot.left(), plot.bottom() + 31,
                                plot.width(), 20), Qt::AlignCenter, "Step");

        painter.save();
        painter.translate(18, plot.center().y());
        painter.rotate(-90);
        painter.drawText(QRectF(-plot.height() / 2, -10, plot.height(), 20),
                         Qt::AlignCenter, "Learning rate");
        painter.restore();
    }

private:
    ScheduleParameters *parameters;
};

class ScheduleComparison : public QObject
{
public:
    ScheduleComparison(Q3DSurface *graph, ScheduleParameters *parameters,
                       std::function<void(int)> progress_callback,
                       QObject *parent)
        : QObject(parent), graph(graph), parameters(parameters),
          progress_callback(progress_callback),
          surface_proxy(new QSurfaceDataProxy),
          surface_series(new QSurface3DSeries(surface_proxy.get()))
    {
        initializeGraph();
        updateSurface();
        for (size_t index = 0; index < runs.size(); ++index) {
            runs[index].schedule = kSchedules[index];
            runs[index].ball.reset(new Ball(graph, kScheduleColors[index],
                                            comparisonSurface));
            runs[index].path.reset(new Line(graph, kScheduleColors[index],
                                            comparisonSurface));
        }
        rebuildOptimizers();

        QObject::connect(&timer, &QTimer::timeout, [this]() { advance(); });
        timer.setInterval(25);
        QObject::connect(surface_series.get(),
                         &QSurface3DSeries::selectedPointChanged,
                         [this](QPoint selected) {
            if (selected == QSurface3DSeries::invalidSelectionPosition())
                return;
            const QVector3D point = surface_proxy->itemAt(selected)->position();
            starting_position = Point(point.x(), point.z());
            if (starting_point_callback)
                starting_point_callback(starting_position);
            restart();
        });
        reset();
        timer.start();
    }

    void reset()
    {
        current_step = 0;
        for (Run &run : runs) {
            run.optimizer->setStartingPosition(starting_position.x,
                                               starting_position.z);
            run.optimizer->resetPositionAndComputeGradient();
            const Point position = run.optimizer->position();
            run.path->erase();
            run.path->addPoint(position);
            if (run.visible && show_paths)
                run.path->render();
            run.ball->setPositionOnSurface(position);
            run.ball->setVisible(run.visible && show_balls);
        }
        progress_callback(current_step);
    }

    void restart()
    {
        reset();
        timer.start();
    }

    void setRunning(bool running)
    {
        if (running) {
            if (current_step >= parameters->total_steps)
                reset();
            timer.start();
        } else {
            timer.stop();
        }
    }

    void setStepsPerTick(int value) { steps_per_tick = value; }

    void setOptimizer(OptimizerType optimizer)
    {
        optimizer_type = optimizer;
        rebuildOptimizers();
        restart();
    }

    void setSurface(SurfaceType surface)
    {
        active_surface = surface;
        updateSurface();
        restart();
    }

    void setStartingPosition(double x, double z)
    {
        starting_position = Point(x, z);
        restart();
    }

    void setStartingPointCallback(std::function<void(Point)> callback)
    {
        starting_point_callback = callback;
    }

    void setRunVisible(size_t index, bool visible)
    {
        runs[index].visible = visible;
        runs[index].ball->setVisible(visible && show_balls);
        runs[index].path->setVisible(visible && show_paths);
    }

    void setShowPaths(bool show)
    {
        show_paths = show;
        for (Run &run : runs)
            run.path->setVisible(run.visible && show_paths);
    }

    void setShowBalls(bool show)
    {
        show_balls = show;
        for (Run &run : runs)
            run.ball->setVisible(run.visible && show_balls);
    }

    void zoom(int direction)
    {
        Q3DCamera *camera = graph->scene()->activeCamera();
        camera->setZoomLevel(camera->zoomLevel() + direction * 10.0f);
    }

private:
    struct Run {
        ScheduleType schedule = ScheduleType::Step;
        std::unique_ptr<GradientDescent> optimizer;
        std::unique_ptr<Ball> ball;
        std::unique_ptr<Line> path;
        bool visible = true;
    };

    std::unique_ptr<GradientDescent> createOptimizer() const
    {
        switch (optimizer_type) {
        case OptimizerType::GradientDescent:
            return std::unique_ptr<GradientDescent>(new VanillaGradientDescent);
        case OptimizerType::Momentum:
            return std::unique_ptr<GradientDescent>(new Momentum);
        case OptimizerType::QHM:
            return std::unique_ptr<GradientDescent>(new QHM);
        case OptimizerType::AdaGrad:
            return std::unique_ptr<GradientDescent>(new AdaGrad);
        case OptimizerType::RMSProp:
            return std::unique_ptr<GradientDescent>(new RMSProp);
        case OptimizerType::Adam:
            return std::unique_ptr<GradientDescent>(new Adam);
        case OptimizerType::AdamW:
            return std::unique_ptr<GradientDescent>(new AdamW);
        case OptimizerType::QHAdam:
            return std::unique_ptr<GradientDescent>(new QHAdam);
        }
        return std::unique_ptr<GradientDescent>(new VanillaGradientDescent);
    }

    void rebuildOptimizers()
    {
        for (Run &run : runs) {
            run.optimizer = createOptimizer();
            run.optimizer->setObjective(comparisonSurface);
        }
    }

    void initializeGraph()
    {
        graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
        graph->activeTheme()->setType(Q3DTheme::Theme(2));
        graph->scene()->activeCamera()->setCameraPreset(
                    Q3DCamera::CameraPresetFrontHigh);

        QValue3DAxis *x_axis = new QValue3DAxis;
        x_axis->setTitle("X");
        x_axis->setTitleVisible(true);
        graph->setAxisX(x_axis);
        QValue3DAxis *y_axis = new QValue3DAxis;
        y_axis->setTitle("Loss");
        y_axis->setTitleVisible(true);
        graph->setAxisY(y_axis);
        QValue3DAxis *z_axis = new QValue3DAxis;
        z_axis->setTitle("Y");
        z_axis->setTitleVisible(true);
        graph->setAxisZ(z_axis);

        surface_series->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
        surface_series->setFlatShadingEnabled(false);
        QLinearGradient gradient;
        gradient.setColorAt(1.0, Qt::darkGreen);
        gradient.setColorAt(0.35, Qt::yellow);
        gradient.setColorAt(0.0, Qt::darkRed);
        surface_series->setBaseGradient(gradient);
        surface_series->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
        graph->addSeries(surface_series.get());

    }

    void updateSurface()
    {
        const int sample_count = 51;
        const double minimum = -2.0;
        const double maximum = 2.0;
        QSurfaceDataArray *data = new QSurfaceDataArray;
        data->reserve(sample_count);
        for (int z_index = 0; z_index < sample_count; ++z_index) {
            QSurfaceDataRow *row = new QSurfaceDataRow(sample_count);
            const double z = minimum + (maximum - minimum)
                    * z_index / (sample_count - 1);
            for (int x_index = 0; x_index < sample_count; ++x_index) {
                const double x = minimum + (maximum - minimum)
                        * x_index / (sample_count - 1);
                (*row)[x_index].setPosition(
                            QVector3D(x, comparisonSurface(x, z), z));
            }
            *data << row;
        }
        surface_proxy->resetArray(data);
    }

    void advance()
    {
        for (int iteration = 0; iteration < steps_per_tick; ++iteration) {
            if (current_step >= parameters->total_steps) {
                timer.stop();
                return;
            }
            ++current_step;
            for (Run &run : runs) {
                const double rate = scheduleLearningRate(
                            run.schedule, current_step, *parameters);
                run.optimizer->learning_rate = rate;
                const Point position = run.optimizer->takeGradientStep();
                run.path->addPoint(position);
                if (run.visible && show_paths)
                    run.path->render();
                run.ball->setPositionOnSurface(position);
            }
        }
        progress_callback(current_step);
    }

    Q3DSurface *graph;
    ScheduleParameters *parameters;
    std::function<void(int)> progress_callback;
    std::unique_ptr<QSurfaceDataProxy> surface_proxy;
    std::unique_ptr<QSurface3DSeries> surface_series;
    std::array<Run, 5> runs;
    QTimer timer;
    Point starting_position = Point(1.6, 1.2);
    std::function<void(Point)> starting_point_callback;
    int current_step = 0;
    int steps_per_tick = 1;
    bool show_paths = true;
    bool show_balls = true;
    OptimizerType optimizer_type = OptimizerType::GradientDescent;
};

} // namespace

LearningRateScheduleWidget::LearningRateScheduleWidget(QWidget *parent)
    : QWidget(parent)
{
    ScheduleParameters *parameters = new ScheduleParameters;
    QObject::connect(this, &QObject::destroyed,
                     [parameters]() { delete parameters; });
    LearningRateChart *chart = new LearningRateChart(parameters);

    QLabel *progress = new QLabel;
    progress->setAlignment(Qt::AlignCenter);

    Q3DSurface *graph = new Q3DSurface;
    ScheduleComparison *comparison = new ScheduleComparison(
                graph, parameters,
                [=](int step) {
        progress->setText(QString("Training step %1 / %2")
                          .arg(step).arg(parameters->total_steps));
    }, this);
    QWidget *graph_container = QWidget::createWindowContainer(graph);
    graph_container->setMinimumSize(420, 300);
    graph_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    graph_container->setToolTip(
                "Click the surface to move the shared starting point");

    QWidget *visualizations = new QWidget;
    QVBoxLayout *visualization_layout = new QVBoxLayout(visualizations);
    visualization_layout->setContentsMargins(0, 0, 0, 0);
    visualization_layout->addWidget(graph_container, 3);
    visualization_layout->addWidget(progress);
    visualization_layout->addWidget(chart, 2);

    QDoubleSpinBox *base_rate = new QDoubleSpinBox;
    base_rate->setDecimals(6);
    base_rate->setRange(0.000001, 0.19);
    base_rate->setValue(parameters->base_rate);
    base_rate->setSingleStep(0.005);

    QDoubleSpinBox *end_rate = new QDoubleSpinBox;
    end_rate->setDecimals(6);
    end_rate->setRange(0.0, 0.19);
    end_rate->setValue(parameters->end_rate);
    end_rate->setSingleStep(0.001);

    QSpinBox *total_steps = new QSpinBox;
    total_steps->setRange(10, 10000);
    total_steps->setValue(parameters->total_steps);
    total_steps->setSingleStep(50);

    QCheckBox *warmup_enabled = new QCheckBox("Enable linear warmup");
    warmup_enabled->setChecked(true);
    QSpinBox *warmup_steps = new QSpinBox;
    warmup_steps->setRange(0, parameters->total_steps - 1);
    warmup_steps->setValue(parameters->warmup_steps);

    QSpinBox *step_size = new QSpinBox;
    step_size->setRange(1, parameters->total_steps);
    step_size->setValue(parameters->step_size);

    QDoubleSpinBox *gamma = new QDoubleSpinBox;
    gamma->setDecimals(3);
    gamma->setRange(0.001, 1.0);
    gamma->setValue(parameters->gamma);
    gamma->setSingleStep(0.1);

    QPushButton *play = new QPushButton("Pause");
    play->setCheckable(true);
    play->setChecked(true);
    QPushButton *restart = new QPushButton("Restart comparison");
    QComboBox *speed = new QComboBox;
    speed->addItems({"1x", "5x", "10x"});

    QComboBox *surface = new QComboBox;
    surface->addItems({"Local Minimum", "Global Minimum", "Saddle Point",
                       "Elliptic Bowl", "Hills", "Plateau"});
    surface->setCurrentIndex(static_cast<int>(SurfaceType::EllipticBowl));

    QComboBox *optimizer = new QComboBox;
    optimizer->addItems({"Gradient Descent", "Momentum", "QHM", "AdaGrad",
                         "RMSProp", "Adam", "AdamW", "QHAdam"});

    QDoubleSpinBox *starting_x = new QDoubleSpinBox;
    starting_x->setRange(-2.0, 2.0);
    starting_x->setDecimals(3);
    starting_x->setSingleStep(0.1);
    starting_x->setValue(1.6);
    QDoubleSpinBox *starting_y = new QDoubleSpinBox;
    starting_y->setRange(-2.0, 2.0);
    starting_y->setDecimals(3);
    starting_y->setSingleStep(0.1);
    starting_y->setValue(1.2);

    QCheckBox *show_paths = new QCheckBox("Paths");
    show_paths->setChecked(true);
    QCheckBox *show_balls = new QCheckBox("Current positions");
    show_balls->setChecked(true);
    QWidget *view_options = new QWidget;
    QHBoxLayout *view_layout = new QHBoxLayout(view_options);
    view_layout->setContentsMargins(0, 0, 0, 0);
    view_layout->addWidget(show_paths);
    view_layout->addWidget(show_balls);

    QPushButton *zoom_out = new QPushButton("Zoom out");
    QPushButton *zoom_in = new QPushButton("Zoom in");

    QFormLayout *form = new QFormLayout;
    form->addRow(play, restart);
    form->addRow("Animation speed:", speed);
    form->addRow("Optimizer:", optimizer);
    form->addRow("Surface:", surface);
    form->addRow("Starting X:", starting_x);
    form->addRow("Starting Y:", starting_y);
    form->addRow("Show:", view_options);
    form->addRow(zoom_out, zoom_in);
    form->addRow("Base learning rate:", base_rate);
    form->addRow("Ending rate (cosine/linear):", end_rate);
    form->addRow("Total steps:", total_steps);
    form->addRow(warmup_enabled);
    form->addRow("Warmup steps:", warmup_steps);
    form->addRow("Step interval:", step_size);
    form->addRow("Step gamma:", gamma);

    QGroupBox *legend = new QGroupBox("Schedules and colors");
    QVBoxLayout *legend_layout = new QVBoxLayout(legend);
    for (size_t index = 0; index < kScheduleNames.size(); ++index) {
        QWidget *schedule_row = new QWidget;
        QHBoxLayout *schedule_layout = new QHBoxLayout(schedule_row);
        schedule_layout->setContentsMargins(0, 0, 0, 0);

        QFrame *swatch = new QFrame;
        swatch->setFixedSize(20, 20);
        swatch->setFrameShape(QFrame::Box);
        swatch->setAutoFillBackground(true);
        QPalette swatch_palette = swatch->palette();
        swatch_palette.setColor(QPalette::Window, kScheduleColors[index]);
        swatch->setPalette(swatch_palette);

        QCheckBox *visible = new QCheckBox(kScheduleNames[index]);
        visible->setChecked(true);
        QObject::connect(visible, &QCheckBox::toggled,
                         [=](bool checked) {
            parameters->visible[index] = checked;
            comparison->setRunVisible(index, checked);
            chart->update();
        });
        schedule_layout->addWidget(swatch);
        schedule_layout->addWidget(visible, 1);
        legend_layout->addWidget(schedule_row);
    }
    form->addRow(legend);

    QLabel *description = new QLabel(
        "All five runs use the selected optimizer from the same starting point "
        "on the selected loss surface. Only their learning-rate schedules differ. "
        "You can also click directly on the surface to choose a starting point.");
    description->setWordWrap(true);
    form->addRow(description);

    QWidget *controls = new QWidget;
    controls->setLayout(form);
    QScrollArea *controls_scroll = new QScrollArea;
    controls_scroll->setWidget(controls);
    controls_scroll->setWidgetResizable(true);
    controls_scroll->setFrameShape(QFrame::NoFrame);
    controls_scroll->setMinimumWidth(300);
    controls_scroll->setMaximumWidth(420);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(visualizations);
    splitter->addWidget(controls_scroll);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(splitter);

    auto refresh = [=]() {
        chart->update();
        comparison->restart();
        play->setChecked(true);
        play->setText("Pause");
    };

    QObject::connect(play, &QPushButton::toggled, [=](bool running) {
        play->setText(running ? "Pause" : "Play");
        comparison->setRunning(running);
    });
    QObject::connect(restart, &QPushButton::clicked, refresh);
    QObject::connect(speed,
                     QOverload<int>::of(&QComboBox::currentIndexChanged),
                     [=](int index) {
        const int speeds[] = {1, 5, 10};
        comparison->setStepsPerTick(speeds[index]);
    });
    QObject::connect(optimizer,
                     QOverload<int>::of(&QComboBox::currentIndexChanged),
                     [=](int index) {
        comparison->setOptimizer(static_cast<OptimizerType>(index));
        play->setChecked(true);
        play->setText("Pause");
    });
    QObject::connect(surface,
                     QOverload<int>::of(&QComboBox::currentIndexChanged),
                     [=](int index) {
        comparison->setSurface(static_cast<SurfaceType>(index));
        play->setChecked(true);
        play->setText("Pause");
    });
    auto update_starting_position = [=]() {
        comparison->setStartingPosition(starting_x->value(), starting_y->value());
        play->setChecked(true);
        play->setText("Pause");
    };
    QObject::connect(starting_x,
                     QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     update_starting_position);
    QObject::connect(starting_y,
                     QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     update_starting_position);
    comparison->setStartingPointCallback([=](Point point) {
        const QSignalBlocker block_x(starting_x);
        const QSignalBlocker block_y(starting_y);
        starting_x->setValue(point.x);
        starting_y->setValue(point.z);
        play->setChecked(true);
        play->setText("Pause");
    });
    QObject::connect(show_paths, &QCheckBox::toggled,
                     [=](bool checked) { comparison->setShowPaths(checked); });
    QObject::connect(show_balls, &QCheckBox::toggled,
                     [=](bool checked) { comparison->setShowBalls(checked); });
    QObject::connect(zoom_out, &QPushButton::clicked,
                     [=]() { comparison->zoom(-1); });
    QObject::connect(zoom_in, &QPushButton::clicked,
                     [=]() { comparison->zoom(1); });
    QObject::connect(base_rate,
                     QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     [=](double value) { parameters->base_rate = value; refresh(); });
    QObject::connect(end_rate,
                     QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     [=](double value) { parameters->end_rate = value; refresh(); });
    QObject::connect(total_steps,
                     QOverload<int>::of(&QSpinBox::valueChanged),
                     [=](int value) {
        parameters->total_steps = value;
        warmup_steps->setMaximum(value - 1);
        step_size->setMaximum(value);
        refresh();
    });
    QObject::connect(warmup_enabled, &QCheckBox::toggled,
                     [=](bool enabled) {
        warmup_steps->setEnabled(enabled);
        parameters->warmup_steps = enabled ? warmup_steps->value() : 0;
        refresh();
    });
    QObject::connect(warmup_steps,
                     QOverload<int>::of(&QSpinBox::valueChanged),
                     [=](int value) {
        if (warmup_enabled->isChecked()) {
            parameters->warmup_steps = value;
            refresh();
        }
    });
    QObject::connect(step_size,
                     QOverload<int>::of(&QSpinBox::valueChanged),
                     [=](int value) { parameters->step_size = value; refresh(); });
    QObject::connect(gamma,
                     QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     [=](double value) { parameters->gamma = value; refresh(); });

    active_handler = [=](bool active) {
        comparison->setRunning(active && play->isChecked());
    };
    splitter->setSizes({900, 350});
}

void LearningRateScheduleWidget::setActive(bool active)
{
    if (active_handler)
        active_handler(active);
}
