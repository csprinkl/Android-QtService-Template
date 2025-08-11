#include <QCoreApplication>
#include <QCommandLineParser>
#include <QString>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include "timerworker.h"

#ifdef ANDROID
#include <android/log.h>
#define ANDROID_LOG(...) __android_log_print(ANDROID_LOG_INFO, "QtService", __VA_ARGS__)
#else
#define ANDROID_LOG(...)
#endif

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // Android service logging
    ANDROID_LOG("=== Qt Timer Service Starting ===");
    qWarning() << ">>> QtService main() started with args:" << QCoreApplication::arguments();

    ANDROID_LOG("Step 1: Creating command line parser");
    QCommandLineParser parser;
    // verbose mode
    QCommandLineOption verboseOption("v", "verbose output");
    parser.addOption(verboseOption);
    // default "-h" flag
    parser.addHelpOption();

    ANDROID_LOG("Step 2: Parsing command line arguments");
    // parse the command line
    parser.process(a);

    ANDROID_LOG("Step 3: Processing options");
    // toggle verbose mode
    bool verbose = false;
    if (parser.isSet(verboseOption)) {
        verbose = true;
        qWarning() << "*** Command Line ***";
        qWarning() << "Application version: " << APP_VERSION << " (" << APP_BUILD << ")";
    }

    // Create new QThreads
    QThread test1;
    QThread test2;
    QThread test3;
    QThread test4;

    ANDROID_LOG("Step 4: Creating TimerWorker objects");
    // Create TimerWorkers with different intervals
    TimerWorker* worker1 = new TimerWorker(1, 1000); // 1 second
    ANDROID_LOG("Created worker1");
    TimerWorker* worker2 = new TimerWorker(2, 2000); // 2 seconds
    ANDROID_LOG("Created worker2");
    TimerWorker* worker3 = new TimerWorker(3, 3000); // 3 seconds
    ANDROID_LOG("Created worker3");
    TimerWorker* worker4 = new TimerWorker(4, 5000); // 5 seconds
    ANDROID_LOG("Created worker4 - All workers created successfully");

    ANDROID_LOG("Step 5: Moving workers to threads");
    // Move workers to their respective threads
    worker1->moveToThread(&test1);
    worker2->moveToThread(&test2);
    worker3->moveToThread(&test3);
    worker4->moveToThread(&test4);
    ANDROID_LOG("Workers moved to threads successfully");

    ANDROID_LOG("Step 6: Connecting signals and slots");
    // Connect thread started signals to worker start slots
    QObject::connect(&test1, &QThread::started, worker1, &TimerWorker::startWork);
    QObject::connect(&test2, &QThread::started, worker2, &TimerWorker::startWork);
    QObject::connect(&test3, &QThread::started, worker3, &TimerWorker::startWork);
    QObject::connect(&test4, &QThread::started, worker4, &TimerWorker::startWork);
    ANDROID_LOG("Thread started connections made");

    // Connect application aboutToQuit to worker stop slots
    QObject::connect(&a, &QCoreApplication::aboutToQuit, worker1, &TimerWorker::stopWork);
    QObject::connect(&a, &QCoreApplication::aboutToQuit, worker2, &TimerWorker::stopWork);
    QObject::connect(&a, &QCoreApplication::aboutToQuit, worker3, &TimerWorker::stopWork);
    QObject::connect(&a, &QCoreApplication::aboutToQuit, worker4, &TimerWorker::stopWork);
    ANDROID_LOG("AboutToQuit connections made");

    // Connect worker destroyed signals to thread quit slots
    QObject::connect(worker1, &TimerWorker::destroyed, &test1, &QThread::quit);
    QObject::connect(worker2, &TimerWorker::destroyed, &test2, &QThread::quit);
    QObject::connect(worker3, &TimerWorker::destroyed, &test3, &QThread::quit);
    QObject::connect(worker4, &TimerWorker::destroyed, &test4, &QThread::quit);
    ANDROID_LOG("Destroyed connections made");

    // Optional: Connect to worker signals for verbose logging
    if (verbose) {
        QObject::connect(worker1, &TimerWorker::workerStarted,
                         [](int id) {
                             qWarning() << QString("Thread %1 worker started").arg(id);
                             ANDROID_LOG("Thread %d worker started", id);
                         });
        QObject::connect(worker2, &TimerWorker::workerStarted,
                         [](int id) {
                             qWarning() << QString("Thread %1 worker started").arg(id);
                             ANDROID_LOG("Thread %d worker started", id);
                         });
        QObject::connect(worker3, &TimerWorker::workerStarted,
                         [](int id) {
                             qWarning() << QString("Thread %1 worker started").arg(id);
                             ANDROID_LOG("Thread %d worker started", id);
                         });
        QObject::connect(worker4, &TimerWorker::workerStarted,
                         [](int id) {
                             qWarning() << QString("Thread %1 worker started").arg(id);
                             ANDROID_LOG("Thread %d worker started", id);
                         });
    }

    if(verbose) {
        qWarning() << "starting threads";
        ANDROID_LOG("Starting 4 timer threads");
    }

    ANDROID_LOG("Step 7: Starting threads");
    // Start all threads
    test1.start();
    ANDROID_LOG("Thread 1 started");
    test2.start();
    ANDROID_LOG("Thread 2 started");
    test3.start();
    ANDROID_LOG("Thread 3 started");
    test4.start();
    ANDROID_LOG("Thread 4 started - All threads started");

    qWarning() << "=========  starting event loop  ==============";
    ANDROID_LOG("Step 8: Qt Timer Service: Starting event loop - threads should start ticking now");

    // For Android services, we need to handle the event loop carefully
    QTimer::singleShot(1000, [&]() {
        ANDROID_LOG("Timer threads should be running now - check for timer tick outputs");
    });

    // Start the event loop and run until quit() is called
    int result = a.exec();  // Run the event loop

    // 🔄 Graceful shutdown
    qWarning() << "=========  shutting down threads  ==============";
    ANDROID_LOG("Shutting down Qt Timer Service");

    // Stop workers and clean up
    worker1->deleteLater();
    worker2->deleteLater();
    worker3->deleteLater();
    worker4->deleteLater();

    // Quit and wait for threads
    test1.quit();
    test2.quit();
    test3.quit();
    test4.quit();

    test1.wait(3000);
    test2.wait(3000);
    test3.wait(3000);
    test4.wait(3000);

    qWarning() << "=========  shutdown complete  ==============";
    ANDROID_LOG("Qt Timer Service shutdown complete");
    return result;
}
