#pragma once
#include <QMainWindow>

#include "../common/sharedmemory_manager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(HANDLE dataEvent, QWidget* parent = nullptr);
protected slots:
    void onButtonClicked();

private:
    HANDLE m_dataEvent;
    SharedMemoryManager* m_sharedMemory;
};
