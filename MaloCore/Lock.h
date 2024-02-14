#pragma once
class LockObject
{
public:
    LockObject() {};
    ~LockObject() {};

    virtual void Lock() abstract;
    virtual void UnLock() abstract;
};

class CriticalSectionObject : public LockObject
{
private:
    CRITICAL_SECTION _cs;

public:
    CriticalSectionObject() : LockObject()
    {
        InitializeCriticalSection(&_cs);
    }

    ~CriticalSectionObject()
    {
        DeleteCriticalSection(&_cs);
    }

public:
    virtual void Lock() override
    {
        EnterCriticalSection(&_cs);
    }

    virtual void UnLock() override
    {
        LeaveCriticalSection(&_cs);
    }
};

class MutexObject : public LockObject
{
private:
    HANDLE _mutex;

public:
    MutexObject() : LockObject()
    {
        _mutex = CreateMutex(
            NULL,
            FALSE,
            NULL);
    }

    ~MutexObject()
    {
        CloseHandle(_mutex);
    }

public:
    virtual void Lock() override
    {
        WaitForSingleObject(_mutex, INFINITE);
    }

    virtual void UnLock() override
    {
        ReleaseMutex(_mutex);
    }
};

class SpinLock : public LockObject
{
private:
    volatile __int64 _lock;

public:
    SpinLock() : LockObject(), _lock(0) {};
    ~SpinLock() {};

    virtual void Lock() override;
    virtual void UnLock() override;
};

inline void SpinLock::Lock()
{
    __int64 desired = true;
    __int64 expected = false;
    // T2 T3 T4
    while (InterlockedCompareExchange64(&_lock, desired, expected))
    {
        Sleep(0);
    }

    // T1
}

inline void SpinLock::UnLock()
{
    __int64 desired = false;
    __int64 expected = true;
    InterlockedCompareExchange64(&_lock, desired, expected);
}


class LockGuard
{
private:
    LockObject* _obj;

public:
    LockGuard(LockObject* obj) : _obj(obj)
    {
        _obj->Lock();
    }

    ~LockGuard()
    {
        _obj->UnLock();
    }
};

