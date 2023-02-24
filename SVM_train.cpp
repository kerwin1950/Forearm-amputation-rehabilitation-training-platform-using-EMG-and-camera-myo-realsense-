#include"SVM_train.h"
#define random(a,b) (rand()%(b-a+1)+a)
using namespace std;
using namespace cv;
using namespace cv::ml;
//string file_train_name[5] = { "����.txt","��ȭ.txt","�ſ�.txt","�Ϸ�.txt","�·�.txt" };
string file_train_name[6] = { "����1.txt","����2.txt","����3.txt","��ȭ1.txt","��ȭ2.txt","��ȭ3.txt" };
//string file_train_name[2] = { "����.txt","��ȭ.txt" };
string file_test_name[5] = { "����.txt","��ȭ_2.txt","�ſ�_2.txt","�Ϸ�_2.txt","�·�_2.txt" };
float trainTD_data[6][10][8][4];//5��������10�����ڣ�8��ͨ����4������
float testTD_data[8][4];///////////////////////////////////
float *trainTD_tmp;
float *testTD_tmp;///////////////////////////////
float trainTD_data_ten[50][32];//50�����ڵ����� 8��ͨ��4������
float testTD_data_ten[1][32];//1�����ڵ����� 8��ͨ��4������///////////////
int label[60] = { 1,1,1,1,1,1,1,1,1,1,
				   2,2,2,2,2,2,2,2,2,2,
				  3,3,3,3,3,3,3,3,3,3,
				   4,4,4,4,4,4,4,4,4,4,
				   5,5,5,5,5,5,5,5,5,5,
					6,6,6,6,6,6,6,6,6,6 };
float train_data_raw[6][8][1000];//5��������8��ͨ����1000��������
float test_data_raw[8][1000];////////////////////
int samp_num = 0;//���������
float tmp[SAMP];//�����ݴ�
int tmpnum, tmpnum2;

////////////////������ȡ///////////////
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
	/////////////////////////////��ѵ������д������//////////////////////////////////
	for (size_t file_num = 0; file_num <6; file_num++)
	{
		ifstream train_file(file_train_name[file_num]);//���ζ���txt
		while (train_file >> train_data_raw[file_num][0][samp_num])//��
		{
			//cout << "��" << file_num << "������";
			for (int i = 1; i < 8; i++)
			{
				train_file >> train_data_raw[file_num][i][samp_num];
				//cout <<" "<< train_data_raw[file_num][i][samp_num];
			}
			//cout << endl;
			samp_num++;
		}
		
		cout << "ѵ�������ܹ�" << samp_num << "����" << endl;
		samp_num = 0;
		train_file.close();
	}
}
void Extract_trainTD()
{
	/////////////////////ѵ�����ݷִ��ڣ�������ȡ��д������///////////////////////
	for (size_t file_num = 0; file_num <6; file_num++)//5�������ļ�
	{
		for (size_t win_num = 1; win_num < 11; win_num++)//�ֳ�10������
		{
			for (int chan_num = 0; chan_num < 8; chan_num++)//8��ͨ��
			{
				for (int sampoint_num = SAMP * win_num, tmp_point = 0; sampoint_num < SAMP * (win_num + 1) && tmp_point < SAMP; tmp_point++, sampoint_num++)
				{

					tmp[tmp_point] = train_data_raw[file_num][chan_num][sampoint_num];//����0��8��ͨ�����в�����ÿ��50����
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
	//////////////ѵ�����ݱ��2ά/////////////////////

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
		cout << "��" << i + 1 << "��������";
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
	cout << "���Զ�����" << random_num+1 << endl;
	ifstream test_file(file_test_name[random_num]);//���ζ���txt
//////////////////��������д������////////////////////////
	while (test_file >> test_data_raw[0][samp_num])//��
	{
		for (int i = 1; i < 8; i++)
		{
			test_file >> test_data_raw[i][samp_num];
		}
		samp_num++;
	}

	cout << "�����ܹ�" << samp_num << "����" << endl;
	samp_num = 0;
	test_file.close();
}

void Extract_testTD()
{
	srand((int)time(0));
	int random_num = random(1, 10);
	cout << "ѡȡ��" << random_num * 50 << "����" << (random_num + 1) * 50 << "���������" << endl;
	///////////////////�������ݷִ��ڣ�������ȡ��д������////////////
	for (int chan_num = 0; chan_num < 8; chan_num++)//8��ͨ��
	{
		for (int sampoint_num = SAMP* random_num, tmp_point = 0; sampoint_num < SAMP* (random_num+1) && tmp_point < SAMP; tmp_point++, sampoint_num++)
		{

			tmp[tmp_point] = test_data_raw[chan_num][sampoint_num];//����0��8��ͨ�����в�����ÿ��50����

		}
		testTD_tmp = TD(tmp);
		for (size_t fd_num = 0; fd_num < 4; fd_num++)
		{
			testTD_data[chan_num][fd_num] = testTD_tmp[fd_num];
		}

	}

	//////////////�������ݱ��2ά/////////////////////
	for (size_t n = 0; n < 8; n++)
	{
		for (size_t m = 0; m < 4; m++)
		{

			testTD_data_ten[0][m + n * 4] = testTD_data[n][m];
		}
	}
	cout << "���Զ���������:" << 0;
	for (size_t j = 0; j < 32; j++)
	{
		cout << " " << testTD_data_ten[0][j];
	}
	cout << endl;
}
//////////////////////SVMѵ��/////////////////////
void SVMtrain()
{
	//Mat img = cv::Mat::zeros(600, 640, CV_8UC3);
	Load_traindata();
	Extract_trainTD();
	//////////////////svmѵ��/////////////////
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
	std::cout << "������Ϊ������" << endl;
	std::cout << result << endl;
	system("pause");
}
int SVM_realtimetest(float x[8][50])
{
	for (int chan_num = 0; chan_num < 8; chan_num++)//8��ͨ��
	{
		for (int tmp_point = 0; tmp_point < SAMP; tmp_point++)
		{

			tmp[tmp_point] = x[chan_num][tmp_point];//����0��8��ͨ�����в�����ÿ��50����

		}
		testTD_tmp = TD(tmp);
		for (size_t fd_num = 0; fd_num < 4; fd_num++)
		{
			testTD_data[chan_num][fd_num] = testTD_tmp[fd_num];
		}

	}
	//////////////�������ݱ��2ά/////////////////////
	for (size_t n = 0; n < 8; n++)
	{
		for (size_t m = 0; m < 4; m++)
		{

			testTD_data_ten[0][m + n * 4] = testTD_data[n][m];
		}
	}
	//cout << "���Զ���������:" << 0;
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
	std::cout << "������Ϊ������" << endl;
	std::cout << result << endl;
	return result;
}