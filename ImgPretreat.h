#pragma once
#include"Common.h"
using namespace std;
using namespace cv;
using namespace rs2;
using namespace myo;

class ImgPretreat
{
public:
	ImgPretreat();
	~ImgPretreat();
};
class ImgPretreatment//ͼ��Ԥ����ȥ��Զ������ɫ�ָȥ��С������
{
public:
	void setImage(Mat dimage, Mat image, Mat bimage);
	void TogetForeground(int distance);
	void TogetSkincolor();
	void removeSmallareas(Mat&bimage);
	ImgPretreatment();
private:
	Mat depthimage;
	Mat image;
	Mat binimage;
};


