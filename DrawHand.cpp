#include "DrawHand.h"

void HandMovement::setMovement(Point p, float a)
{
	pointO = p;
	angle = a;
}
HandMovement::HandMovement()
{
	pointO = Point(0, 0);
	angle = 0.0;
}
//Point HandMovement:: getPointO()
//{
//	return pointO;
//}
/*float HandMovement::getAngle()
{
	return angle;
}*/

void HandMovement::Fist(Mat color_image)
{

	line(color_image, Point(pointO.x, pointO.y),
		Point(pointO.x + 35 * cos((angle + 15) * PI / 180.0f),
			pointO.y - 35 * sin((angle + 15) * PI / 180.0f)),
		Scalar(0, 255, 0), 6);
	line(color_image, Point(pointO.x, pointO.y),
		Point(pointO.x + 35 * cos((angle + 345) * PI / 180.0f),
			pointO.y - 35 * sin((angle + 345) * PI / 180.0f)),
		Scalar(0, 255, 0), 6);
}
void HandMovement::Relax(Mat color_image)
{
	line(color_image, Point(pointO.x, pointO.y),
		Point(pointO.x + 35 * cos((angle + 45) * PI / 180.0f),
			pointO.y - 35 * sin((angle + 45) * PI / 180.0f)),
		Scalar(0, 255, 0), 6);
	line(color_image, Point(pointO.x, pointO.y),
		Point(pointO.x + 35 * cos((angle + 315) * PI / 180.0f),
			pointO.y - 35 * sin((angle + 315) * PI / 180.0f)),
		Scalar(0, 255, 0), 6);
}

