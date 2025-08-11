#ifndef TIMERWORKER_H
#define TIMERWORKER_H

#include <QObject>
#include <QTimer>

class TimerWorker : public QObject {
    Q_OBJECT

public:
    explicit TimerWorker(int id, int intervalMs, QObject* parent = nullptr);
    ~TimerWorker();

    int getWorkerId() const { return workerId; }
    int getInterval() const { return interval; }
    bool isRunning() const;

public slots:
    void startWork();
    void stopWork();
    void setInterval(int newIntervalMs);

signals:
    void timerTick(int workerId, int intervalMs);
    void workerStarted(int workerId);
    void workerStopped(int workerId);

private slots:
    void onTimeout();

private:
    QTimer* timer;
    int workerId;
    int interval;
};

#endif // TIMERWORKER_H
