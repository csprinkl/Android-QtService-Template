#include <QCoreApplication>
#include <QApplication>
#include <QCommandLineParser>
#include <QString>
#include <QThread>
#include <QDebug>
#include <timerworker.h>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    qInfo() << ">>> QtService main() started with args:" << QCoreApplication::arguments();

    QCommandLineParser parser;
    // verbose mode
    QCommandLineOption verboseOption("v", "verbose output");
    parser.addOption(verboseOption);
    // service mode
    QCommandLineOption serviceOption("a", "start as Android service");
    parser.addOption(serviceOption);
    // default "-h" flag
    parser.addHelpOption();
    // parse the command line
    parser.process(a);

    // toggle verbose mode
    bool verbose = false;
    if (parser.isSet(verboseOption)) {
        verbose = true;
        qDebug() << "*** Command Line ***";
        qDebug() << "Application version: " << APP_VERSION << " (" << APP_BUILD << ")";
    }

    // Create new QThreads
    QThread test1;
    QThread test2;
    QThread test3;
    QThread test4;

    // Create TimerWorkers with different intervals
    TimerWorker* worker1 = new TimerWorker(1, 1000); // 1 second
    TimerWorker* worker2 = new TimerWorker(2, 2000); // 2 seconds
    TimerWorker* worker3 = new TimerWorker(3, 3000); // 3 seconds
    TimerWorker* worker4 = new TimerWorker(4, 5000); // 5 seconds

    // Move workers to their respective threads
    worker1->moveToThread(&test1);
    worker2->moveToThread(&test2);
    worker3->moveToThread(&test3);
    worker4->moveToThread(&test4);

    // Connect thread started signals to worker start slots
    QObject::connect(&test1, &QThread::started, worker1, &TimerWorker::startWork);
    QObject::connect(&test2, &QThread::started, worker2, &TimerWorker::startWork);
    QObject::connect(&test3, &QThread::started, worker3, &TimerWorker::startWork);
    QObject::connect(&test4, &QThread::started, worker4, &TimerWorker::startWork);

    // Connect application aboutToQuit to worker stop slots
    QObject::connect(&a, &QCoreApplication::aboutToQuit, worker1, &TimerWorker::stopWork);
    QObject::connect(&a, &QCoreApplication::aboutToQuit, worker2, &TimerWorker::stopWork);
    QObject::connect(&a, &QCoreApplication::aboutToQuit, worker3, &TimerWorker::stopWork);
    QObject::connect(&a, &QCoreApplication::aboutToQuit, worker4, &TimerWorker::stopWork);

    // Connect worker destroyed signals to thread quit slots
    QObject::connect(worker1, &TimerWorker::destroyed, &test1, &QThread::quit);
    QObject::connect(worker2, &TimerWorker::destroyed, &test2, &QThread::quit);
    QObject::connect(worker3, &TimerWorker::destroyed, &test3, &QThread::quit);
    QObject::connect(worker4, &TimerWorker::destroyed, &test4, &QThread::quit);

    // Optional: Connect to worker signals for verbose logging
    if (verbose) {
        QObject::connect(worker1, &TimerWorker::workerStarted,
                         [](int id) { qDebug() << QString("Thread %1 worker started").arg(id); });
        QObject::connect(worker2, &TimerWorker::workerStarted,
                         [](int id) { qDebug() << QString("Thread %1 worker started").arg(id); });
        QObject::connect(worker3, &TimerWorker::workerStarted,
                         [](int id) { qDebug() << QString("Thread %1 worker started").arg(id); });
        QObject::connect(worker4, &TimerWorker::workerStarted,
                         [](int id) { qDebug() << QString("Thread %1 worker started").arg(id); });
    }

    if(verbose) { qDebug() << "starting threads"; }

    // Start all threads
    test1.start();
    test2.start();
    test3.start();
    test4.start();

    qDebug() << "=========  starting event loop  ==============";

    // Start the event loop and run until quit() is called
    int result = a.exec();  // Run the event loop

    // 🔄 Graceful shutdown
    qDebug() << "=========  shutting down threads  ==============";

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

    qDebug() << "=========  shutdown complete  ==============";
    return result;
}
