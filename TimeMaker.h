#pragma once
#include"Common.h"
using namespace std;
using namespace cv;
using namespace rs2;
using namespace myo;

class TimeMaker
{
public:
	TimeMaker();
	~TimeMaker();
};
class Timer
{
private:
	long start_time;
	long pause_time;
	//����boolֵ�������״̬ 
	bool is_pause; //��¼��ʱ����״̬ ���Ƿ�����ͣ״̬��
	bool is_stop;//�Ƿ���ֹͣ״̬ 
public:
	Timer();
	bool isPause(); //���ؼ�ʱ��״̬ 
	bool isStop();
	//��ʱ�������ֶ��������ܣ� 
	void Start();
	void Pause();
	void Stop();
	inline long getStartTime() { return start_time; }
	long show(Mat image);
};


