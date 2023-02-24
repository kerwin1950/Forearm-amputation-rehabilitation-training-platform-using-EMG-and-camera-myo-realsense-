#include "Myo.h"
#include<typeinfo>
#include <time.h>
using namespace std;
using namespace cv;
using namespace rs2;
using namespace myo;
#define X_AXIS 320
#define Y_AXIS 240


cv::Mat emg_image (Y_AXIS, X_AXIS, CV_8UC3,Scalar(0, 0, 0));

std::vector<std::vector<cv::Point>> emgpoint_c(8, std::vector<cv::Point>());
int xAxis = 0;
int emg_Channels = 0;
const char* emg_win = "8ͨ������ͼ";
//string file_name[5] = { "����.txt","��ȭ.txt","�ſ�.txt","�Ϸ�.txt","�·�.txt" };
string file_name[6] = { "����1.txt","����2.txt","����3.txt","��ȭ1.txt","��ȭ2.txt","��ȭ3.txt" };
//string file_name[2] = { "����.txt","��ȭ.txt" };
ofstream data_file;
//ÿ���û��Ͽ�Myo����ʱ���������onUnpair()��
DataCollector::DataCollector()
{

}
	void  DataCollector::onUnpair(myo::Myo* myo, uint64_t timestamp)
	{
		emgData.fill(0);
	}
	//ֻҪ��Ե�Myo�ṩ���µ�EMG���ݣ��ͻ����onEmgData()��������EMG����
	void DataCollector::onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg)
	{
		for (int i = 0; i < 8; i++) {
			emgData[i] = emg[i];

		}
	}
	// DeviceListener�л�������������������д���麯��������onAccelerometerData()��
	//����������ӣ����渲�ǵĺ������㹻�ˡ�
	//���Ƕ��������������ӡ����on��()�������µĵ�ǰֵ��

	void DataCollector::print()//bool once)
	{
		//if (once) {
			moveWindow(emg_win, 0,560);
			//ִֻ��һ��
		//	once = false;
		//}
		// ��ӡ������ͼ���ݡ�
		for (int i = 0; i < 8; i++)
		{
			emgpoint_c[i].push_back(cv::Point(xAxis, 32 * (i + 1) + emgData[i] / 6));
			
			if (abs(emgData[i]) >= 10)
			{
				emg_Channels++;
			}
		}
		for (int i = 0; i < 8; i++)
		{
			cv::polylines(emg_image, emgpoint_c[i], false, cv::Scalar(32 * i, 32 * (8 - i), 0), 1, 8, 0);
		}
		imshow(emg_win, emg_image);
		xAxis++;
		if (emg_Channels >= 3)
		{
		//	keybd_event(VK_UP, 0, 0, 0);
			emg_Channels = 0;
		}
		else {
		//	keybd_event(VK_UP, 0, KEYEVENTF_KEYUP, 0);
			emg_Channels = 0;
		}
		if (xAxis >=X_AXIS)
		{
			xAxis = 0;
			emg_image.setTo(cv::Scalar(0, 0, 0));
			for (int i = 0; i < 8; i++)
			{
				emgpoint_c[i].clear();
			}
		}
	}
	void DataCollector::collect(int n)
	{
			data_file.open(file_name[n], ios::app);
			for (int i = 0; i < 8; i++)
			{
				data_file << (float)emgData[i] << " ";
			}
			data_file << endl;
			data_file.close();
	}
