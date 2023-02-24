#include "FaceRecogniton.h"
#include"Common.h"
using namespace std;
using namespace cv;
using namespace rs2;
using namespace myo;
string xmlprofilefacePath = "E:\\opencv3.4\\sources\\data\\haarcascades\\haarcascade_profileface.xml";
string xmlfrontalfacePath = "E:\\opencv3.4\\sources\\data\\haarcascades\\haarcascade_frontalface_default.xml";

void Face::face_load(cv::Mat image)
{
	color_image = image;
	ccf_proface.load(xmlprofilefacePath);//加载脸部分类器
	ccf_froface.load(xmlfrontalfacePath);
}

void Face::reduceimage()//缩小图像
{
	cv::Mat gray, smallImg, smallImg_mirror;
	double fx = 1 / scale;
	cvtColor(color_image, gray, CV_BGR2GRAY);
	resize(gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT);
	equalizeHist(smallImg, smallImg);
	flip(smallImg, smallImg_mirror, 1);
	ccf_proface.detectMultiScale(smallImg, leftface, 1.1, 3, 0 | CASCADE_SCALE_IMAGE,
		Size(30, 30));
	ccf_proface.detectMultiScale(smallImg_mirror, rightface, 1.1, 3, 0 | CASCADE_SCALE_IMAGE,
		Size(30, 30));
	ccf_froface.detectMultiScale(smallImg, frontface, 1.1, 3, 0 | CASCADE_SCALE_IMAGE,
		Size(30, 30));
}
void Face::removeface(Mat& image)//去除脸部区域
{
	for (int i = 0; i < leftface.size(); i++)
	{
		Rect rectFace = leftface[i];

		for (size_t i = rectFace.x * scale; i < (rectFace.x + rectFace.width)*scale; i++)
		{
			for (size_t j = rectFace.y * scale; j < (rectFace.y + rectFace.height)*scale; j++)
			{
				image.at<cv::Vec3b>(cv::Point(i, j)) = cv::Vec3b(0, 0, 0);
			}
		}
	}
	for (int i = 0; i < rightface.size(); i++)
	{
		Rect rectFace = rightface[i];

		for (size_t i = (110 - rectFace.x) * scale; i < (110 - rectFace.x + rectFace.width)*scale; i++)
		{
			for (size_t j = rectFace.y * scale; j < (rectFace.y + rectFace.height)*scale; j++)
			{
				image.at<cv::Vec3b>(cv::Point(i, j)) = cv::Vec3b(0, 0, 0);
			}

		}
	}
	for (int i = 0; i < frontface.size(); i++)
	{
		Rect rectFace = frontface[i];

		for (size_t i = rectFace.x * scale; i < (rectFace.x + rectFace.width)*scale; i++)
		{
			for (size_t j = rectFace.y * scale; j < (rectFace.y + rectFace.height)*scale; j++)
			{
				image.at<cv::Vec3b>(cv::Point(i, j)) = cv::Vec3b(0, 0, 0);

			}
		}

	}
}
