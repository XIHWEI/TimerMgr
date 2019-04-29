#ifndef X_Lock_H
#define X_Lock_H

#ifdef _MSC_VER

#include <windows.h>

#else

#include <pthread.h>

#endif //_MSC_VER

class IMyLock
{
public:
	virtual ~IMyLock() {}
	virtual void Lock() = 0;
	virtual void Unlock() = 0;
};

class XLock
{
public:
    XLock(IMyLock* plock)
	{
        m_lock = plock;
		m_lock->Lock();
	}
	~XLock()
	{
		m_lock->Unlock();
	}
private:
    IMyLock* m_lock;
};


#ifdef _MSC_VER

class XMutex : public IMyLock
{
public:
	XMutex()
	{
		m_mutex = ::CreateMutex(NULL, FALSE, NULL);	
	}
	~XMutex()
	{
		::CloseHandle(m_mutex);	
	}
	virtual void Lock()
	{
		WaitForSingleObject(m_mutex, INFINITE);	
	}
	virtual void Unlock()
	{
		::ReleaseMutex(m_mutex);
	}

private:
	HANDLE m_mutex;
};

#else
	
class XMutex : public IMyLock
{
public:
	XMutex()
	{
		pthread_mutexattr_t mattr;
		pthread_mutexattr_init( &mattr );
		pthread_mutex_init( &m_mutex, &mattr );
	}
	~XMutex()
	{
		pthread_mutex_destroy( &m_mutex );
	}
	virtual void Lock()
	{
		pthread_mutex_lock( &m_mutex );
	}
	virtual void Unlock()
	{
		pthread_mutex_unlock( &m_mutex );
	}

private:
	pthread_mutex_t m_mutex;
};

#endif //#ifdef _MSC_VER



#endif  //X_Lock_H
