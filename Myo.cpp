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
const char* emg_win = "8通道肌电图";
//string file_name[5] = { "放松.txt","握拳.txt","张开.txt","上翻.txt","下翻.txt" };
string file_name[6] = { "放松1.txt","放松2.txt","放松3.txt","握拳1.txt","握拳2.txt","握拳3.txt" };
//string file_name[2] = { "放松.txt","握拳.txt" };
ofstream data_file;
//每当用户断开Myo连接时，都会调用onUnpair()。
DataCollector::DataCollector()
{

}
	void  DataCollector::onUnpair(myo::Myo* myo, uint64_t timestamp)
	{
		emgData.fill(0);
	}
	//只要配对的Myo提供了新的EMG数据，就会调用onEmgData()，并启用EMG流。
	void DataCollector::onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg)
	{
		for (int i = 0; i < 8; i++) {
			emgData[i] = emg[i];

		}
	}
	// DeviceListener中还有其他可以在这里重写的虚函数，比如onAccelerometerData()。
	//对于这个例子，上面覆盖的函数就足够了。
	//我们定义这个函数来打印上面on…()函数更新的当前值。

	void DataCollector::print()//bool once)
	{
		//if (once) {
			moveWindow(emg_win, 0,560);
			//只执行一次
		//	once = false;
		//}
		// 打印出肌电图数据。
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
