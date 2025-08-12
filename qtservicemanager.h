#ifndef QTSERVICEMANAGER_H
#define QTSERVICEMANAGER_H

#include "qtservice_global.h"
#include "timerworker.h"
#include <QObject>
#include <QThread>
#include <QList>
#include <QCoreApplication>

class QTSERVICE_EXPORT QtServiceManager : public QObject
{
    Q_OBJECT

public:
    struct TimerConfig {
        int id;
        int intervalMs;
    };

    explicit QtServiceManager(QObject *parent = nullptr);
    ~QtServiceManager();

    // Static factory method for easy integration
    static QtServiceManager* createService(const QList<TimerConfig>& configs);
    static QtServiceManager* createDefaultService(); // 4 default timers

    // Service control
    bool startService();
    void stopService();
    bool isServiceRunning() const;

    // Timer management
    void addTimer(int id, int intervalMs);
    void removeTimer(int id);
    void setTimerInterval(int id, int intervalMs);

    // Configuration
    void setVerboseLogging(bool enabled);

signals:
    void serviceStarted();
    void serviceStopped();
    void timerTick(int timerId, int intervalMs);
    void serviceError(const QString& error);

public slots:
    // Called by QCoreApplication for service mode
    void initializeAsService(int argc, char* argv[]);

private slots:
    void onTimerTick(int workerId, int intervalMs);

private:
    void setupTimers();
    void cleanupTimers();

    struct TimerThreadPair {
        TimerWorker* worker;
        QThread* thread;
        TimerConfig config;
    };

    QList<TimerThreadPair> m_timers;
    bool m_serviceRunning;
    bool m_verboseLogging;
    QCoreApplication* m_app;
};

#endif // QTSERVICEMANAGER_H
