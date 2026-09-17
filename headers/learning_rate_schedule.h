#ifndef LEARNING_RATE_SCHEDULE_H
#define LEARNING_RATE_SCHEDULE_H

#include <functional>

#include <QWidget>

class LearningRateScheduleWidget : public QWidget
{
public:
    explicit LearningRateScheduleWidget(QWidget *parent = nullptr);
    void setActive(bool active);

private:
    std::function<void(bool)> active_handler;
};

#endif // LEARNING_RATE_SCHEDULE_H
