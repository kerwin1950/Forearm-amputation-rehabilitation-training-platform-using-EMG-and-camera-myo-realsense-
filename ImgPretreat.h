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
class ImgPretreatment//图像预处理：去除远景，肤色分割，去除小块区域
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


