#include "sharedmemory_manager.h"

#include <iostream>

SharedMemoryManager::SharedMemoryManager(bool create)
{
    if (create)
    {
        // 主进程：创建共享内存
        m_hMapFile = CreateFileMappingW(
            INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, SHARED_MEMORY_SIZE, SHARED_MEMORY_NAME);
        if (!m_hMapFile)
        {
            DWORD err = GetLastError();  // 立刻保存
            std::cout << "[SharedMemory] CreateFileMappingW failed, error:" << err << std::endl;
        }
        else
        {
            m_isOwner = true;
            std::cout << "[SharedMemory] create success" << std::endl;
        }
    }
    else
    {
        // 子进程：打开共享内存
        m_hMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_NAME);
        if (!m_hMapFile)
        {
            std::cerr << "[SharedMemory] open failed: " << GetLastError() << std::endl;
        }
        else
        {
            std::cout << "[SharedMemory] open success" << std::endl;
        }
    }
}

SharedMemoryManager::~SharedMemoryManager()
{
    if (m_hMapFile)
    {
        CloseHandle(m_hMapFile);
        m_hMapFile = nullptr;
    }
}

bool SharedMemoryManager::write(const std::string &data)
{
    if (!m_hMapFile)
    {
        return false;
    }

    LPVOID pBuf = MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEMORY_SIZE);
    if (!pBuf)
    {
        return false;
    }

    memset(pBuf, 0, SHARED_MEMORY_SIZE);
    memcpy(pBuf, data.c_str(), std::min((size_t)SHARED_MEMORY_SIZE - 1, data.size()));
    UnmapViewOfFile(pBuf);

    return true;
}

std::string SharedMemoryManager::read()
{
    if (!m_hMapFile)
    {
        return "";
    }

    LPVOID pBuf = MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEMORY_SIZE);
    if (!pBuf)
    {
        return "";
    }

    std::string data((char *)pBuf);
    UnmapViewOfFile(pBuf);

    return data;
}
