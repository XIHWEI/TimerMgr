
#include <stdio.h>  
#include <signal.h>  
#include <time.h>  
#include <string.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <sys/time.h>

#include "TimerMgr.h"

#define  TIMER_WAKEUP_MILLSEC 20000000  // 20ms
#define  TIMER_START_VALID_TIMESTAMP  0



TimerMgr  * TimerMgr::m_instance  = NULL;
XMutex       TimerMgr::m_lock;

TimerSession::TimerSession()
{
   m_starttime_millsec = TIMER_START_VALID_TIMESTAMP;
   m_timerid = TimerMgr::instance()->GenTimerID();
}

TimerSession::~TimerSession()
{

}

void TimerSession::StartTimer(unsigned int second, unsigned delaytime)
{
  m_invl_timesec  = second;
  m_starttime_millsec = GetTimeStampMillsec() + delaytime*1000 - m_invl_timesec*1000;
  TimerMgr::instance()->RegTimer(m_timerid,this);
}

void TimerSession::ClearTimer()
{
   TimerMgr::instance()->UnRegTimer(m_timerid);
}

void TimerSession::HandleTimerSessn()
{
   double nowtime = GetTimeStampMillsec();
   if((m_starttime_millsec + m_invl_timesec*1000) < nowtime)
   {
     //时钟已到
     m_starttime_millsec = GetTimeStampMillsec();
     HandleTimerPostProc();
   }
}

void TimerSession::DelaySpefySec(unsigned int second)
{
    m_starttime_millsec = GetTimeStampMillsec() + second*1000 - m_invl_timesec*1000;
}

double TimerSession::GetTimeStampMillsec()
{
	struct	timeval    tv;
	struct	timezone   tz;
	gettimeofday(&tv,&tz);	
	double timemillsec = tv.tv_sec *1.0*1000 + tv.tv_usec/1000;
	return timemillsec;
}

static void fade_in_callback(union sigval v)
{ 
   TimerMgr *ptr = (TimerMgr*)v.sival_ptr;
   ptr->HandleTimer();
}

TimerMgr::TimerMgr()
{
   m_gen_timeid = 0;
}

TimerMgr::~TimerMgr()
{
   
}

TimerMgr * TimerMgr::instance()
{
   if( m_instance == NULL)
   {
      XLock atlock(&m_lock);
      if( m_instance == NULL)
      {
        m_instance = new TimerMgr;
      }  
   }

   return m_instance;
}

int TimerMgr::StartTimer()
{
   struct sigevent evp; 
   memset(&evp, 0, sizeof(evp));
   evp.sigev_value.sival_ptr = this; //这里传一个参数进去，在timer的callback回调函数里面可以获得它  
   evp.sigev_notify = SIGEV_THREAD; //定时器到期后内核创建一个线程执行sigev_notify_function函数 
   evp.sigev_notify_function = fade_in_callback; //这个就是指定回调函数
   
   int ret = 0;
   ret = timer_create(CLOCK_REALTIME, &evp, &m_fade_in_timer);
   if(ret < 0)
   {
     return ret;
   }

   struct itimerspec ts;
   ts.it_interval.tv_sec = 0;
   ts.it_interval.tv_nsec = TIMER_WAKEUP_MILLSEC; 
   ts.it_value.tv_sec = 0;
   ts.it_value.tv_nsec = TIMER_WAKEUP_MILLSEC;  
   ret = timer_settime(m_fade_in_timer, TIMER_ABSTIME, &ts, NULL);
   if(ret < 0)
   {
     timer_delete(m_fade_in_timer);
     return ret;
   } 
   return 0;
}


TIMERID TimerMgr::GenTimerID()
{
   XLock atlock(&m_lock); 
   m_gen_timeid++;
   return m_gen_timeid;
}

void TimerMgr::RegTimer(TIMERID timeid,TimerSession * timersessn)
{
   XLock atlock(&m_lock); 
   MapTimerSessn::iterator iter = m_map_timersessn.find(timeid);
   if(iter == m_map_timersessn.end())
   {
     m_map_timersessn[timeid] = timersessn;
   }
}

void TimerMgr::UnRegTimer(TIMERID timeid)
{
   XLock atlock(&m_lock); 
   MapTimerSessn::iterator iter = m_map_timersessn.find(timeid);
   if(iter != m_map_timersessn.end())
   {
     m_map_timersessn.erase(iter);
   }
}

void TimerMgr::HandleTimer()
{
   //需要遍历全部timer
   XLock atlock(&m_lock); 
   MapTimerSessn::iterator iter = m_map_timersessn.begin();
   for(;iter != m_map_timersessn.end();iter++)
   {
     TimerSession * timersessn = (TimerSession *)iter->second;
     timersessn->HandleTimerSessn();
   }
}


