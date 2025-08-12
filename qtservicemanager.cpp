#include "qtservicemanager.h"
#include <QDebug>
#include <QTimer>

#ifdef ANDROID
#include <android/log.h>
#define ANDROID_LOG(...) __android_log_print(ANDROID_LOG_INFO, "QtServiceLib", __VA_ARGS__)
#else
#define ANDROID_LOG(...)
#endif

QtServiceManager::QtServiceManager(QObject *parent)
    : QObject(parent)
    , m_serviceRunning(false)
    , m_verboseLogging(false)
    , m_app(nullptr)
{
    ANDROID_LOG("QtServiceManager created");
}

QtServiceManager::~QtServiceManager()
{
    stopService();
    ANDROID_LOG("QtServiceManager destroyed");
}

QtServiceManager* QtServiceManager::createDefaultService()
{
    QList<TimerConfig> defaultConfigs = {
        {1, 1000},  // 1 second
        {2, 2000},  // 2 seconds
        {3, 3000},  // 3 seconds
        {4, 5000}   // 5 seconds
    };
    return createService(defaultConfigs);
}

QtServiceManager* QtServiceManager::createService(const QList<TimerConfig>& configs)
{
    QtServiceManager* manager = new QtServiceManager();

    for (const auto& config : configs) {
        manager->addTimer(config.id, config.intervalMs);
    }

    return manager;
}

void QtServiceManager::initializeAsService(int argc, char* argv[])
{
    ANDROID_LOG("Initializing Qt Service Library");

    if (!m_app) {
        m_app = new QCoreApplication(argc, argv);
    }

    // Connect application signals
    connect(m_app, &QCoreApplication::aboutToQuit, this, &QtServiceManager::stopService);

    startService();

    // Start event loop (this blocks until quit)
    QTimer::singleShot(100, [this]() {
        ANDROID_LOG("Qt Service Library initialized and running");
        emit serviceStarted();
    });
}

bool QtServiceManager::startService()
{
    if (m_serviceRunning) {
        return true;
    }

    ANDROID_LOG("Starting Qt Service Library with %d timers", m_timers.size());

    try {
        setupTimers();
        m_serviceRunning = true;
        emit serviceStarted();
        return true;
    } catch (...) {
        emit serviceError("Failed to start service");
        return false;
    }
}

void QtServiceManager::stopService()
{
    if (!m_serviceRunning) {
        return;
    }

    ANDROID_LOG("Stopping Qt Service Library");

    cleanupTimers();
    m_serviceRunning = false;
    emit serviceStopped();
}

void QtServiceManager::addTimer(int id, int intervalMs)
{
    // Check if timer already exists
    for (const auto& pair : m_timers) {
        if (pair.config.id == id) {
            qWarning() << "Timer with ID" << id << "already exists";
            return;
        }
    }

    TimerThreadPair pair;
    pair.config = {id, intervalMs};
    pair.worker = new TimerWorker(id, intervalMs);
    pair.thread = new QThread(this);

    // Move worker to thread
    pair.worker->moveToThread(pair.thread);

    // Connect signals
    connect(pair.thread, &QThread::started, pair.worker, &TimerWorker::startWork);
    connect(pair.worker, &TimerWorker::timerTick, this, &QtServiceManager::onTimerTick);
    connect(this, &QtServiceManager::serviceStopped, pair.worker, &TimerWorker::stopWork);
    connect(pair.worker, &TimerWorker::destroyed, pair.thread, &QThread::quit);

    m_timers.append(pair);

    // Start thread if service is running
    if (m_serviceRunning) {
        pair.thread->start();
    }

    ANDROID_LOG("Added timer %d with %dms interval", id, intervalMs);
}

void QtServiceManager::setupTimers()
{
    for (auto& pair : m_timers) {
        pair.thread->start();
    }
    ANDROID_LOG("All timer threads started");
}

void QtServiceManager::cleanupTimers()
{
    for (auto& pair : m_timers) {
        if (pair.worker) {
            pair.worker->deleteLater();
        }
        if (pair.thread) {
            pair.thread->quit();
            pair.thread->wait(3000);
        }
    }
    ANDROID_LOG("All timer threads stopped");
}

void QtServiceManager::onTimerTick(int workerId, int intervalMs)
{
    if (m_verboseLogging) {
        qInfo() << QString("Timer %1: Tick at %2ms interval").arg(workerId).arg(intervalMs);
    }
    emit timerTick(workerId, intervalMs);
}

bool QtServiceManager::isServiceRunning() const
{
    return m_serviceRunning;
}

void QtServiceManager::setVerboseLogging(bool enabled)
{
    m_verboseLogging = enabled;
}

void QtServiceManager::setTimerInterval(int id, int intervalMs)
{
    for (auto& pair : m_timers) {
        if (pair.config.id == id) {
            pair.worker->setInterval(intervalMs);
            pair.config.intervalMs = intervalMs;
            return;
        }
    }
    qWarning() << "Timer with ID" << id << "not found";
}

void QtServiceManager::removeTimer(int id)
{
    for (int i = 0; i < m_timers.size(); ++i) {
        if (m_timers[i].config.id == id) {
            auto& pair = m_timers[i];

            if (pair.worker) {
                pair.worker->deleteLater();
            }
            if (pair.thread) {
                pair.thread->quit();
                pair.thread->wait(3000);
            }

            m_timers.removeAt(i);
            ANDROID_LOG("Removed timer %d", id);
            return;
        }
    }
    qWarning() << "Timer with ID" << id << "not found";
}
