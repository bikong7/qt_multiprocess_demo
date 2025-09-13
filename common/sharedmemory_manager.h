#pragma once
#include <windows.h>

#include <string>

#define SHARED_MEMORY_NAME L"MyApp1_SharedMemory"
#define SHARED_MEMORY_SIZE 1024

class SharedMemoryManager
{
public:
    SharedMemoryManager(bool create);
    ~SharedMemoryManager();

    bool write(const std::string &data);
    std::string read();

private:
    HANDLE m_hMapFile = nullptr;
    bool m_isOwner = false;
};
