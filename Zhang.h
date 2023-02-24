#pragma once
#include"Common.h"
using namespace std;
using namespace cv;
using namespace rs2;
using namespace myo;
class Zhang
{
public:
	Zhang();
	~Zhang();

};

/****************************手臂信息提取**********************************/
class Arms
{
public:
	Arms();
	Mat thinImage(const Mat & src, const int maxIterations = -1);
	void filterOver(Mat thinSrc);
	vector< Point> getPoints(const Mat &thinSrc, unsigned int raudis = 4,
		unsigned int thresholdMax = 6, unsigned int thresholdMin = 4);

};
