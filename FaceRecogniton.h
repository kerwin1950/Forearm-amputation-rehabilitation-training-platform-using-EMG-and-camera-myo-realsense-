#pragma once
#include"Common.h"
using namespace std;
using namespace cv;
using namespace rs2;
using namespace myo;

class FaceRecogniton
{
public:
	FaceRecogniton();
	~FaceRecogniton();
};
class Face
{
public:
	void face_load(cv::Mat image);
	void reduceimage();
	void removeface(cv::Mat& image);

private:
	CascadeClassifier ccf_proface, ccf_froface;      //创建脸部对象
	vector<Rect> leftface, rightface, frontface;
	double scale = 4;
	cv::Mat color_image;

};

