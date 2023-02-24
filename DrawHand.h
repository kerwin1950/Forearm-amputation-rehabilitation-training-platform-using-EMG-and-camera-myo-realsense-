#pragma once
#include"Common.h"
using namespace std;
using namespace cv;
using namespace rs2;
using namespace myo;

#define PI 3.1415926

class DrawHand
{
public:
	DrawHand();
	~DrawHand();
};
class HandMovement
{
public:
	HandMovement();//¹¹Ôìº¯Êý
	//Point getPointO();
	//float getAngle();
	void setMovement(Point p, float a);
	//void Fist(Mat&color_image, Point pointO, float angle);
	//void Relax(Mat&color_image, Point pointO, float angle);
	void Fist(Mat color_image);
	void Relax(Mat color_image);
	//~HandMovement();
private:
	Point pointO;
	float angle;
};

