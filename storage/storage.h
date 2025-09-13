#pragma once
#include <windows.h>

#include <atomic>

#include "../common/sharedmemory_manager.h"

class Storage
{
public:
    explicit Storage(HANDLE exitEvent, HANDLE dataEvent);
    int run();

private:
    HANDLE m_exitEvent;
    HANDLE m_dataEvent;

    SharedMemoryManager* m_sharedMemory;
};
