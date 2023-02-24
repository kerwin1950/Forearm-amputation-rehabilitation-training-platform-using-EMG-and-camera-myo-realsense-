#include "ImgPretreat.h"

/***********************************图像预处理*********************************************/
ImgPretreatment::ImgPretreatment()
{
}
void ImgPretreatment::setImage(Mat dimage, Mat img, Mat bimage)
{
	depthimage = dimage;
	image = img;
	binimage = bimage;
}
void ImgPretreatment::TogetForeground(int distance) { //去除背景，获取前景 distance可调节

	int rowNumber = depthimage.rows;
	int colNumber = depthimage.cols*depthimage.channels();
	for (int i = 0; i < rowNumber; ++i) {
		unsigned short int *p1 =
			depthimage.ptr<unsigned short int>(i);
		uchar *data = image.ptr<uchar>(i);
		for (int j = 0; j < colNumber; ++j) {
			data[j * 3] = *p1 > 15 && *p1 < distance ? data[j * 3] : 0;
			data[j * 3 + 1] = *p1 > 15 && *p1 < distance ? data[j * 3 + 1] : 0;
			data[j * 3 + 2] = *p1 > 15 && *p1 < distance ? data[j * 3 + 2] : 0;
			++p1;
		}
	}
}
void ImgPretreatment::TogetSkincolor() { //肤色分割
	Mat Y, Cr, Cb, tmp;
	vector<Mat> channels;
	cvtColor(image, tmp, CV_BGR2YCrCb);	//RGB转YCrCb
	split(tmp, channels);				//分离tmp的通道
	Cr = channels.at(1);
	Cb = channels.at(2);
	int rowNumber = tmp.rows;
	int colNumber = tmp.cols;
	for (int i = 0; i < rowNumber; i++) {
		unsigned short int *p1 =
			depthimage.ptr<unsigned short int>(i);
		uchar* currentCr = Cr.ptr< uchar>(i);
		uchar* currentCb = Cb.ptr< uchar>(i);

		for (int j = 0; j < colNumber; j++) {
			uchar pix = (*p1 - 10) / 5;//深度信息处理一下
			if ((currentCr[j] > 133) && (currentCr[j] < 173)
				&& (currentCb[j] > 77) && (currentCb[j] < 127))
			{
				image.at<cv::Vec3b>(cv::Point(j, i)) = cv::Vec3b(pix, pix, pix);
				//dstimage2.at<cv::Vec3b>(cv::Point(j, i)) = cv::Vec3b(255, 255, 255);
				binimage.at<cv::Vec3b>(cv::Point(j, i)) = cv::Vec3b(255, 255, 255);
			}
			else
			{
				image.at<cv::Vec3b>(cv::Point(j, i)) = cv::Vec3b(0, 0, 0);
				binimage.at<cv::Vec3b>(cv::Point(j, i)) = cv::Vec3b(0, 0, 0);
			}
			++p1;

		}
	}
}

void ImgPretreatment::removeSmallareas(Mat &bimage) { //去除小块区域
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	//定义轮廓和层次结构
	vector< vector< Point>> contours;//vector容器里面放了一个vector容器，子容器里放点 轮廓数组   
	vector< Vec4i> hierarchy;//有4个int的向量
	vector< vector< Point> >::iterator itc; //轮廓迭代器  
   //查找轮廓
	cvtColor(binimage, binimage, CV_BGR2GRAY);
	threshold(binimage, binimage, 100, 255, THRESH_BINARY);
	findContours(binimage, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);//
	//轮廓按照面积大小进行升序排序
	double area;
	double minarea = 3500;
	double maxarea = 0;
	Moments mom; // 轮廓矩 
	Rect rect;
	itc = contours.begin();   //使用迭代器去除噪声轮廓  

	while (itc != contours.end()) {
		char str[10];
		area = contourArea(*itc);
		if ((area > minarea)) {// && area > 0) {
			itc = contours.erase(itc);  //itc一旦erase，需要重新赋值  
		}
		else {
			itc++;
		}
	}
	//绘制连通区域轮廓，计算质心坐标
	for (int i = 0; i < contours.size(); i++) {//while (itc != contours.end())
		drawContours(bimage, contours, i, Scalar(0, 0, 0), -1, 8);   //轮廓绘制 
	}
	//imshow("1", binimage);
}

