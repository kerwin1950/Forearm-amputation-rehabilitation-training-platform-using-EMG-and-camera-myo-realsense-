#pragma once
#include"Common.h"
using namespace std;
using namespace cv;
using namespace rs2;
using namespace myo;

class GetArmPosition
{
public:
	GetArmPosition();
	~GetArmPosition();
};

class TwoPoints
{
public:
	TwoPoints();
	void get_two_point(Point pO, Point pA);
	double distance();
	double angle();
	void draw_arm(Mat&color_image, Point pointHand, Point pointElbow);
private:
	Point pointO;
	Point pointA;
};