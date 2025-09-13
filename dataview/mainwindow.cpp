#include "mainwindow.h"

#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

MainWindow::MainWindow(HANDLE dataEvent, QWidget *parent)
    : QMainWindow(parent), m_dataEvent(dataEvent), m_sharedMemory(new SharedMemoryManager(true))  // 主进程创建共享内存
{
    setWindowTitle("主进程界面");
    resize(400, 200);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;

    QLabel *label = new QLabel("这是主进程的GUI界面", this);
    label->setAlignment(Qt::AlignCenter);

    QPushButton *btn = new QPushButton("点击无副作用", this);
    layout->addWidget(label);
    layout->addWidget(btn);

    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    connect(btn, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
}

void MainWindow::onButtonClicked()
{
    if (m_sharedMemory->write("abc"))
    {
        // 通知 storage 有新数据
        SetEvent(m_dataEvent);
        qDebug() << "[master] 已写入共享内存: abc";
    }
    else
    {
        qWarning() << "写入共享内存失败";
    }
}
