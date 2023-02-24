#include "GetArmPosition.h"
/************************获取前后端点****************************************/
TwoPoints::TwoPoints()
{
}
void TwoPoints::draw_arm(Mat&color_image, Point pointHand, Point pointElbow)
{
	line(color_image, Point(pointHand.x, pointHand.y),
		Point(pointElbow.x, pointElbow.y), Scalar(255, 125, 125), 9);
}
void TwoPoints::get_two_point(Point pO, Point pA)
{
	pointO = pO;//原点
	pointA = pA;//
}
double  TwoPoints::distance() {

	double distance;
	distance = powf((pointO.x - pointA.x), 2) + powf((pointO.y - pointA.y), 2);
	distance = sqrtf(distance);
	return distance;
}
double  TwoPoints::angle() {
	double angle = 0;
	Point point;
	double temp;
	point = Point((pointA.x - pointO.x), (pointA.y - pointO.y));
	if ((0 == point.x) && (0 == point.y)) {
		return 0;
	}
	if (0 == point.x) {
		angle = 90;
		return angle;
	}
	if (0 == point.y) {
		angle = 0;
		return angle;
	}
	temp = fabsf(float(point.y) / float(point.x));
	temp = atan(temp);
	temp = temp * 180 / CV_PI;
	if ((0 < point.x) && (0 < point.y)) {
		angle = 360 - temp;
		return angle;
	}
	if ((0 > point.x) && (0 < point.y)) {
		angle = 360 - (180 - temp);
		return angle;
	}
	if ((0 < point.x) && (0 > point.y)) {
		angle = temp;
		return angle;
	}
	if ((0 > point.x) && (0 > point.y)) {
		angle = 180 - temp;
		return angle;
	}
	printf("sceneDrawing :: getAngle error!");
	return -1;
}

