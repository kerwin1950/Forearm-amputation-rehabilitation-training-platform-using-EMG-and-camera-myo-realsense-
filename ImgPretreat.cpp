#include "ImgPretreat.h"

/***********************************ͼ��Ԥ����*********************************************/
ImgPretreatment::ImgPretreatment()
{
}
void ImgPretreatment::setImage(Mat dimage, Mat img, Mat bimage)
{
	depthimage = dimage;
	image = img;
	binimage = bimage;
}
void ImgPretreatment::TogetForeground(int distance) { //ȥ����������ȡǰ�� distance�ɵ���

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
void ImgPretreatment::TogetSkincolor() { //��ɫ�ָ�
	Mat Y, Cr, Cb, tmp;
	vector<Mat> channels;
	cvtColor(image, tmp, CV_BGR2YCrCb);	//RGBתYCrCb
	split(tmp, channels);				//����tmp��ͨ��
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
			uchar pix = (*p1 - 10) / 5;//�����Ϣ����һ��
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

void ImgPretreatment::removeSmallareas(Mat &bimage) { //ȥ��С������
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	//���������Ͳ�νṹ
	vector< vector< Point>> contours;//vector�����������һ��vector��������������ŵ� ��������   
	vector< Vec4i> hierarchy;//��4��int������
	vector< vector< Point> >::iterator itc; //����������  
   //��������
	cvtColor(binimage, binimage, CV_BGR2GRAY);
	threshold(binimage, binimage, 100, 255, THRESH_BINARY);
	findContours(binimage, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);//
	//�������������С������������
	double area;
	double minarea = 3500;
	double maxarea = 0;
	Moments mom; // ������ 
	Rect rect;
	itc = contours.begin();   //ʹ�õ�����ȥ����������  

	while (itc != contours.end()) {
		char str[10];
		area = contourArea(*itc);
		if ((area > minarea)) {// && area > 0) {
			itc = contours.erase(itc);  //itcһ��erase����Ҫ���¸�ֵ  
		}
		else {
			itc++;
		}
	}
	//������ͨ����������������������
	for (int i = 0; i < contours.size(); i++) {//while (itc != contours.end())
		drawContours(bimage, contours, i, Scalar(0, 0, 0), -1, 8);   //�������� 
	}
	//imshow("1", binimage);
}

