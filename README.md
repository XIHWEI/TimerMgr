# TimerMgr
# Timer Mgr  and  Timer session

#include <iostream>
#include <string.h>

#include "TimerMgr.h"

class SendDataSession : public TimerSession
{
	public:
	SendDataSession()
	{
	}
	virtual void  HandleTimerPostProc()
	{
		printf(" timer print data  \n");
	}
};


int main()
{
	// 开启timerMgr
	int ret = TimerMgr::instance()->StartTimer();
	if(ret != 0)
	{
		printf("TimerMgr Error \n");
		return 0;
	}	
	
	// 定义 定时发送数据的类 间隔1秒
	SendDataSession sendSession(&tcpClient);
	sendSession.StartTimer(1);
	getchar();
	return 1;
}
