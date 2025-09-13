#include <windows.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QProcess>
#include <QThread>
#include <QTimer>

#include "common/common.h"
#include "dataview/mainwindow.h"
#include "storage/storage.h"

// 获取 QProcess 真实 PID
qint64 getProcessPid(const QProcess *process)
{
    if (!process->pid())
    {
        return -1;
    }
#ifdef Q_OS_WIN
    PROCESS_INFORMATION *pi = static_cast<PROCESS_INFORMATION *>(process->pid());
    return static_cast<qint64>(pi->dwProcessId);
#else
    return static_cast<qint64>(process->pid());
#endif
}

// 创建全局退出事件
HANDLE createExitEvent()
{
    HANDLE event = CreateEventW(nullptr, TRUE, FALSE, STORAGE_EXIT_EVENT_NAME);
    if (!event)
    {
        qWarning() << "创建退出事件失败";
    }
    return event;
}

HANDLE createDataEvent()
{
    HANDLE event = CreateEventW(nullptr, TRUE, FALSE, STORAGE_DATA_EVENT_NAME);
    if (!event)
    {
        qWarning() << "创建数据事件失败";
    }
    return event;
}

// 启动 storage 子进程
void startStorage(QProcess *proc, QApplication *app, bool isShuttingDown, std::function<void()> retryFunc)
{
    if (isShuttingDown || proc->state() == QProcess::Running)
    {
        qDebug() << "[storage] 已在运行，PID:" << getProcessPid(proc);
        return;
    }
    qDebug() << "[master] 启动 storage 进程...";
    QStringList args{"--storage"};
    proc->start(app->applicationFilePath(), args);
    if (!proc->waitForStarted(3000))
    {
        qWarning() << "[master] 启动 storage 失败:" << proc->errorString();
        QTimer::singleShot(2000, retryFunc);
    }
    else
    {
        qDebug() << "[master] storage 已启动，PID:" << getProcessPid(proc);
    }
}

// 连接 storage 子进程信号
void setupStorageConnections(QProcess *proc,
                             std::function<void()> restartFunc,
                             QTimer *monitorTimer,
                             bool &isShuttingDown)
{
    QObject::connect(proc,
                     static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                     [=, &isShuttingDown](int exitCode, QProcess::ExitStatus)
                     {
                         if (isShuttingDown)
                         {
                             return;
                         }
                         qDebug() << "[storage] 进程意外退出，代码:" << exitCode << "，重启...";
                         restartFunc();
                     });
    QObject::connect(proc,
                     &QProcess::errorOccurred,
                     [=, &isShuttingDown](QProcess::ProcessError error)
                     {
                         if (isShuttingDown || error != QProcess::FailedToStart)
                         {
                             return;
                         }
                         qWarning() << "[storage] 无法启动:" << error;
                         QTimer::singleShot(2000, restartFunc);
                     });
    QObject::connect(proc,
                     &QProcess::readyRead,
                     [=]()
                     {
                         qDebug().noquote() << proc->readAll();
                     });
}

// 监控 storage 子进程
void setupMonitor(QProcess *proc, QTimer *timer, std::function<void()> restartFunc, bool &isShuttingDown)
{
    QObject::connect(timer,
                     &QTimer::timeout,
                     [=, &isShuttingDown]()
                     {
                         if (isShuttingDown)
                         {
                             return;
                         }
                         if (proc->state() != QProcess::Running)
                         {
                             qDebug() << "[master] 检测到 storage 未运行，尝试重启...";
                             restartFunc();
                         }
                     });
    timer->start(5000);
}

// 优雅退出 storage
void shutdownStorage(QProcess *proc, HANDLE exitEvent, QTimer *timer, bool &isShuttingDown)
{
    qDebug() << "[master] 主进程退出，通知 storage...";
    isShuttingDown = true;
    timer->stop();
    if (proc->state() == QProcess::Running)
    {
        SetEvent(exitEvent);
        if (!proc->waitForFinished(5000))
        {
            qDebug() << "[master] storage 无响应，强制杀死...";
            proc->kill();
            proc->waitForFinished(1000);
        }
        else
        {
            qDebug() << "[master] storage 已优雅退出";
        }
    }
    if (exitEvent)
    {
        CloseHandle(exitEvent);
    }
}

// ===== main =====
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("MyApp1");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption("storage", "Run as storage worker process"));
    parser.process(app);

    HANDLE storageExitEvent = createExitEvent();
    HANDLE dataEvent = createDataEvent();

    if (parser.isSet("storage"))
    {
        Storage storage(storageExitEvent, dataEvent);
        return storage.run();
    }
    qDebug() << "[master] 主进程启动，PID:" << QCoreApplication::applicationPid();

    MainWindow mainWindow(dataEvent);
    mainWindow.show();

    QProcess storageProc;
    storageProc.setProcessChannelMode(QProcess::MergedChannels);
    QTimer monitorTimer;
    bool isShuttingDown = false;

    std::function<void()> startFunc;
    startFunc = [&]()
    {
        startStorage(&storageProc, &app, isShuttingDown, startFunc);
    };

    setupStorageConnections(&storageProc, startFunc, &monitorTimer, isShuttingDown);
    setupMonitor(&storageProc, &monitorTimer, startFunc, isShuttingDown);
    startFunc();

    QObject::connect(&app,
                     &QCoreApplication::aboutToQuit,
                     [&]()
                     {
                         shutdownStorage(&storageProc, storageExitEvent, &monitorTimer, isShuttingDown);
                     });

    return app.exec();
}
