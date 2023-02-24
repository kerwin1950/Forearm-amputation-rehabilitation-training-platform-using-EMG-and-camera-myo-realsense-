#include "TimeMaker.h"
Timer::Timer() :start_time(time(0)), pause_time(0)
{
	is_pause = false; //��ʼ����ʱ��״̬
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

void Timer::Start() //��ʼ 
{
	if (is_stop)
	{
		start_time = time(0);
		is_stop = false;
	}
	else if (is_pause)
	{
		is_pause = false;
		start_time += time(0) - pause_time; //���¿�ʼʱ�䣺�ô�ʱ��ʱ�� - ��ͣʱ���õ�ʱ�� + ��һ�ο�ʼ��ʱ�� = ��ʱ�Ŀ�ʼʱ�� 
	}
}

void Timer::Pause() //��ͣ 
{
	if (is_stop || is_pause) //�������ֹͣ/��ͣ״̬,�˶��������κδ���ֱ�ӷ��� 
		return;
	else    //�������Ϊ��ͣ״̬
	{
		is_pause = true;
		pause_time = time(0); //��ȡ��ͣʱ�� 
	}
}
void Timer::Stop()  //ֹͣ 
{
	if (is_stop) //���������ֹͣ״̬��������ͣ״̬���������κδ��� 
		return;
	else if (is_pause) //�ı��ʱ��״̬ 
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

