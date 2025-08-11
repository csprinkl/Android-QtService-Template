#include "TimerWorker.h"
#include <QDebug>

TimerWorker::TimerWorker(int id, int intervalMs, QObject* parent)
    : QObject(parent)
    , timer(new QTimer(this))
    , workerId(id)
    , interval(intervalMs)
{
    // Connect timer timeout to our slot
    connect(timer, &QTimer::timeout, this, &TimerWorker::onTimeout);

    qDebug() << QString("TimerWorker %1 created with %2ms interval").arg(workerId).arg(interval);
}

TimerWorker::~TimerWorker()
{
    if (timer && timer->isActive()) {
        timer->stop();
    }
    qDebug() << QString("TimerWorker %1 destroyed").arg(workerId);
}

bool TimerWorker::isRunning() const
{
    return timer && timer->isActive();
}

void TimerWorker::startWork()
{
    if (!timer) {
        qWarning() << QString("TimerWorker %1: Cannot start - timer is null").arg(workerId);
        return;
    }

    qInfo() << QString("Worker %1 starting with %2ms interval").arg(workerId).arg(interval);
    timer->start(interval);
    emit workerStarted(workerId);
}

void TimerWorker::stopWork()
{
    if (!timer) {
        qWarning() << QString("TimerWorker %1: Cannot stop - timer is null").arg(workerId);
        return;
    }

    qInfo() << QString("Worker %1 stopping").arg(workerId);
    timer->stop();
    emit workerStopped(workerId);
}

void TimerWorker::setInterval(int newIntervalMs)
{
    if (newIntervalMs <= 0) {
        qWarning() << QString("TimerWorker %1: Invalid interval %2ms").arg(workerId).arg(newIntervalMs);
        return;
    }

    bool wasRunning = isRunning();
    if (wasRunning) {
        timer->stop();
    }

    interval = newIntervalMs;
    qInfo() << QString("Worker %1 interval changed to %2ms").arg(workerId).arg(interval);

    if (wasRunning) {
        timer->start(interval);
    }
}

void TimerWorker::onTimeout()
{
    qInfo() << QString("Timer %1: Tick at %2ms interval").arg(workerId).arg(interval);
    emit timerTick(workerId, interval);
}
