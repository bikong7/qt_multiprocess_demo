#include "storage.h"

#include <QCoreApplication>
#include <QThread>
#include <iostream>

#include "../common/common.h"

Storage::Storage(HANDLE exitEvent, HANDLE dataEvent)
    : m_exitEvent(exitEvent),
      m_dataEvent(dataEvent),
      m_sharedMemory(new SharedMemoryManager(false))  // 子进程打开共享内存
{
}

int Storage::run()
{
    HANDLE handles[2] = {m_exitEvent, m_dataEvent};

    while (true)
    {
        DWORD ret = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

        if (ret == WAIT_OBJECT_0)  // hExitEvent
        {
            std::cout << "[storage] 收到退出通知" << std::endl;
            break;
        }
        else if (ret == WAIT_OBJECT_0 + 1)  // m_dataEvent
        {
            std::string data = m_sharedMemory->read();
            std::cout << "[storage] 读取到共享内存数据: " << data << std::endl;

            // 重置事件，等待下次通知
            ResetEvent(m_dataEvent);
        }
    }

    return 0;
}
