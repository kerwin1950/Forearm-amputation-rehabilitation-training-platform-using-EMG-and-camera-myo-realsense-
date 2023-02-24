#include "TimeMaker.h"
Timer::Timer() :start_time(time(0)), pause_time(0)
{
	is_pause = false; //初始化计时器状态
	is_stop = true;
}

bool Timer::isPause()
{
	if (is_pause)
		return true;
	else
		return false;
}

bool Timer::isStop()
{
	if (is_stop)
		return true;
	return false;
}

void Timer::Start() //开始 
{
	if (is_stop)
	{
		start_time = time(0);
		is_stop = false;
	}
	else if (is_pause)
	{
		is_pause = false;
		start_time += time(0) - pause_time; //更新开始时间：用此时的时间 - 暂停时所用的时间 + 上一次开始的时间 = 此时的开始时间 
	}
}

void Timer::Pause() //暂停 
{
	if (is_stop || is_pause) //如果处于停止/暂停状态,此动作不做任何处理，直接返回 
		return;
	else    //否则调制为暂停状态
	{
		is_pause = true;
		pause_time = time(0); //获取暂停时间 
	}
}
void Timer::Stop()  //停止 
{
	if (is_stop) //如果正处于停止状态（不是暂停状态），不做任何处理 
		return;
	else if (is_pause) //改变计时器状态 
	{
		is_pause = false;
		is_stop = true;
	}
	else if (!is_stop)
	{
		is_stop = true;
	}
}

long Timer::show(Mat image)
{
	long t = time(0) - start_time;
	char buf[80];
	sprintf_s(buf, 80, "%d:%d:%d", t / 60 / 60, t / 60, t % 60);
	//cout << t / 60 / 60 << " ," << t / 60 << " ," << t % 60 << endl;
	putText(image, buf, Point(250, 50), FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255), 3, 8, 0);
	return t;
}

