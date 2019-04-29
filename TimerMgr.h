#ifndef _TIMERMGR_H_
#define _TIMERMGR_H_

#include <map>
#include <time.h>

#include "XLock.h"

using namespace std;

#define TIMERID unsigned int

class TimerMgr;
class TimerSession
{
  friend class TimerMgr;
  public:
    TimerSession();
    virtual ~TimerSession();  
    unsigned int GetTimerID();
    void   DelaySpefySec(unsigned int second); //延迟多少秒再执行
    void   StartTimer(unsigned int second, unsigned delaytime = 0); //以秒为单位
    void   ClearTimer();
    //时钟到定时进行的后处理
    //不允许在此函数中调用 TimerMgr 的接口，这样会导致死锁的
    virtual void  HandleTimerPostProc() = 0; 

  private:
    void   HandleTimerSessn();
	double GetTimeStampMillsec();
    
  private:
    TIMERID  m_timerid;
    double   m_starttime_millsec;
    unsigned int m_invl_timesec;
};

class TimerMgr
{
 private:
   typedef map<TIMERID,TimerSession *> MapTimerSessn;
   
 public:
   static TimerMgr * instance();
   TIMERID GenTimerID();
   void    RegTimer(TIMERID timeid,TimerSession * timersessn);
   void    UnRegTimer(TIMERID timeid);
   void    HandleTimer();
   int     StartTimer();

 private:  
     TimerMgr();
     ~TimerMgr(); 

 private:
   static TimerMgr  *m_instance;
   static XMutex     m_lock;  
   timer_t           m_fade_in_timer;
   TIMERID           m_gen_timeid;
   MapTimerSessn     m_map_timersessn;
   
};
   
#endif

