#include"SVM_train.h"
#define random(a,b) (rand()%(b-a+1)+a)
using namespace std;
using namespace cv;
using namespace cv::ml;
//string file_train_name[5] = { "放松.txt","握拳.txt","张开.txt","上翻.txt","下翻.txt" };
string file_train_name[6] = { "放松1.txt","放松2.txt","放松3.txt","握拳1.txt","握拳2.txt","握拳3.txt" };
//string file_train_name[2] = { "放松.txt","握拳.txt" };
string file_test_name[5] = { "放松.txt","握拳_2.txt","张开_2.txt","上翻_2.txt","下翻_2.txt" };
float trainTD_data[6][10][8][4];//5个动作，10个窗口，8个通道，4个特征
float testTD_data[8][4];///////////////////////////////////
float *trainTD_tmp;
float *testTD_tmp;///////////////////////////////
float trainTD_data_ten[50][32];//50个窗口的特征 8个通道4个特征
float testTD_data_ten[1][32];//1个窗口的特征 8个通道4个特征///////////////
int label[60] = { 1,1,1,1,1,1,1,1,1,1,
				   2,2,2,2,2,2,2,2,2,2,
				  3,3,3,3,3,3,3,3,3,3,
				   4,4,4,4,4,4,4,4,4,4,
				   5,5,5,5,5,5,5,5,5,5,
					6,6,6,6,6,6,6,6,6,6 };
float train_data_raw[6][8][1000];//5个动作，8个通道，1000个采样点
float test_data_raw[8][1000];////////////////////
int samp_num = 0;//多少组采样
float tmp[SAMP];//窗口暂存
int tmpnum, tmpnum2;

////////////////特征提取///////////////
float  TDextraction::Mav(float x[SAMP])
{
	vector<float> tmp(SAMP);
	float num = 0, mav = 0;
	for (int j = 0; j < SAMP; j++)
	{
		tmp[j] = fabs(x[j]);
	}
	for (int i = 0; i < tmp.size(); i++)
	{
		num = num + tmp[i];
	}
	mav = num / tmp.size();
	return mav;
}
int  TDextraction::sgn(float d) { return d < -EPS ? 1 : 0; }
int  TDextraction::sgnw(float d, float w) { return d > w ? 1 : 0; }
float  TDextraction::Zc(float x[SAMP])
{
	float zc = 0;
	for (int j = 0; j < SAMP; j++)
	{
		zc = zc + sgn(x[j] * x[j + 1]);
	}
	return zc;
}
float  TDextraction::Ssc(float x[SAMP])
{
	float ssc = 0;
	for (int j = 1; j < SAMP; j++)
	{
		ssc = ssc + sgnw((x[j] - x[j - 1])*(x[j] - x[j + 1]), 0.015);
	}
	return ssc;
}
float  TDextraction::Wl(float x[SAMP])
{
	float wl = 0;
	for (int j = 0; j < SAMP - 1; j++)
	{
		wl = wl + fabs(x[j] - x[j - 1]);
	}
	return wl;
}
float  *TD(float x[SAMP])
{
	TDextraction xx;
	float *a = new float[4];
	a[0] = xx.Ssc(x);
	a[1] = xx.Wl(x);
	a[2] = xx.Zc(x);
	a[3] = xx.Mav(x);
	return a;
}
void Load_traindata()
{
	/////////////////////////////把训练数据写入数组//////////////////////////////////
	for (size_t file_num = 0; file_num <6; file_num++)
	{
		ifstream train_file(file_train_name[file_num]);//依次读入txt
		while (train_file >> train_data_raw[file_num][0][samp_num])//读
		{
			//cout << "第" << file_num << "动作：";
			for (int i = 1; i < 8; i++)
			{
				train_file >> train_data_raw[file_num][i][samp_num];
				//cout <<" "<< train_data_raw[file_num][i][samp_num];
			}
			//cout << endl;
			samp_num++;
		}
		
		cout << "训练数据总共" << samp_num << "个点" << endl;
		samp_num = 0;
		train_file.close();
	}
}
void Extract_trainTD()
{
	/////////////////////训练数据分窗口，特征提取，写入数组///////////////////////
	for (size_t file_num = 0; file_num <6; file_num++)//5个动作文件
	{
		for (size_t win_num = 1; win_num < 11; win_num++)//分成10个窗口
		{
			for (int chan_num = 0; chan_num < 8; chan_num++)//8个通道
			{
				for (int sampoint_num = SAMP * win_num, tmp_point = 0; sampoint_num < SAMP * (win_num + 1) && tmp_point < SAMP; tmp_point++, sampoint_num++)
				{

					tmp[tmp_point] = train_data_raw[file_num][chan_num][sampoint_num];//动作0的8个通道所有采样点每隔50读入
					//line(img, Point(sampoint_num - 1, 100 + 50 * chan_num + train_data_raw[file_num][chan_num][sampoint_num - 1]), Point(sampoint_num, 100 + 50 * chan_num + train_data_raw[file_num][chan_num][sampoint_num]), Scalar(0, 0, 255), 1);

				}
				trainTD_tmp = TD(tmp);
				for (size_t fd_num = 0; fd_num < 4; fd_num++)
				{
					trainTD_data[file_num][win_num - 1][chan_num][fd_num] = trainTD_tmp[fd_num];
				}

			}

		}

	}
	//////////////训练数据变成2维/////////////////////

	for (size_t c = 0; c < 6; c++)
	{
		for (size_t i = 0; i < 10; i++)
		{
			tmpnum2 = i + c * 10;
			for (size_t n = 0; n < 8; n++)
			{
				for (size_t m = 0; m < 4; m++)
				{
					tmpnum = m + n * 4;// +i * 8;

					trainTD_data_ten[tmpnum2][tmpnum] = trainTD_data[c][i][n][m];
				}
			}
		}
	}
	for (size_t i = 0; i <60; i++)
	{
		cout << "第" << i + 1 << "组特征：";
		for (size_t j = 0; j < 32; j++)
		{
			cout << " " << trainTD_data_ten[i][j];
		}
		cout << endl;
	}
}
void Load_testdata()
{
	srand((int)time(0));
	int random_num = random(0, 4);
	cout << "测试动作：" << random_num+1 << endl;
	ifstream test_file(file_test_name[random_num]);//依次读入txt
//////////////////测试数据写入数组////////////////////////
	while (test_file >> test_data_raw[0][samp_num])//读
	{
		for (int i = 1; i < 8; i++)
		{
			test_file >> test_data_raw[i][samp_num];
		}
		samp_num++;
	}

	cout << "测试总共" << samp_num << "个点" << endl;
	samp_num = 0;
	test_file.close();
}

void Extract_testTD()
{
	srand((int)time(0));
	int random_num = random(1, 10);
	cout << "选取第" << random_num * 50 << "到第" << (random_num + 1) * 50 << "个点的特征" << endl;
	///////////////////测试数据分窗口，特征提取，写入数组////////////
	for (int chan_num = 0; chan_num < 8; chan_num++)//8个通道
	{
		for (int sampoint_num = SAMP* random_num, tmp_point = 0; sampoint_num < SAMP* (random_num+1) && tmp_point < SAMP; tmp_point++, sampoint_num++)
		{

			tmp[tmp_point] = test_data_raw[chan_num][sampoint_num];//动作0的8个通道所有采样点每隔50读入

		}
		testTD_tmp = TD(tmp);
		for (size_t fd_num = 0; fd_num < 4; fd_num++)
		{
			testTD_data[chan_num][fd_num] = testTD_tmp[fd_num];
		}

	}

	//////////////测试数据变成2维/////////////////////
	for (size_t n = 0; n < 8; n++)
	{
		for (size_t m = 0; m < 4; m++)
		{

			testTD_data_ten[0][m + n * 4] = testTD_data[n][m];
		}
	}
	cout << "测试动作组特征:" << 0;
	for (size_t j = 0; j < 32; j++)
	{
		cout << " " << testTD_data_ten[0][j];
	}
	cout << endl;
}
//////////////////////SVM训练/////////////////////
void SVMtrain()
{
	//Mat img = cv::Mat::zeros(600, 640, CV_8UC3);
	Load_traindata();
	Extract_trainTD();
	//////////////////svm训练/////////////////
	Mat labelMat(60, 1, CV_32SC1, label);
	Mat traindataMat(60, 32, CV_32FC1, trainTD_data_ten);
	Ptr<SVM> svm = SVM::create();
	svm->setType(SVM::C_SVC);
	svm->setKernel(SVM::LINEAR);
	svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));
	///////////////////////////////////////////////
	svm->train(traindataMat, ROW_SAMPLE, labelMat);
	svm->save("svm.xml");
	//waitKey(1);
	//system("pause");
}
void SVMtest()
{
	Load_testdata();
	Extract_testTD();
	Mat result;
	Mat testdataMat(1, 32, CV_32FC1, testTD_data_ten);
	////////////////////////////////////////////////
	Ptr<SVM> svm = StatModel::load<SVM>("svm.xml");
	svm->predict(testdataMat, result);
	std::cout << "分类结果为动作：" << endl;
	std::cout << result << endl;
	system("pause");
}
int SVM_realtimetest(float x[8][50])
{
	for (int chan_num = 0; chan_num < 8; chan_num++)//8个通道
	{
		for (int tmp_point = 0; tmp_point < SAMP; tmp_point++)
		{

			tmp[tmp_point] = x[chan_num][tmp_point];//动作0的8个通道所有采样点每隔50读入

		}
		testTD_tmp = TD(tmp);
		for (size_t fd_num = 0; fd_num < 4; fd_num++)
		{
			testTD_data[chan_num][fd_num] = testTD_tmp[fd_num];
		}

	}
	//////////////测试数据变成2维/////////////////////
	for (size_t n = 0; n < 8; n++)
	{
		for (size_t m = 0; m < 4; m++)
		{

			testTD_data_ten[0][m + n * 4] = testTD_data[n][m];
		}
	}
	//cout << "测试动作组特征:" << 0;
	/*for (size_t j = 0; j < 32; j++)
	{
		cout << " " << testTD_data_ten[0][j];
	}
	cout << endl;*/
	int result;
	Mat testdataMat(1, 32, CV_32FC1, testTD_data_ten);
	////////////////////////////////////////////////
	Ptr<SVM> svm = StatModel::load<SVM>("svm.xml");
	result=svm->predict(testdataMat);
	std::cout << "分类结果为动作：" << endl;
	std::cout << result << endl;
	return result;
}