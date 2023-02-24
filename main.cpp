//
// @brief        Realsense435i 识别手臂位姿信息
// @author       薛金伟
// @version:    
// @date:    
//
#include"Common.h"
#include"FaceRecogniton.h"
#include "ImgPretreat.h"
#include "GetArmPosition.h"
#include "DrawHand.h"
#include "TimeMaker.h"
#include "Myo.h"
#include "Zhang.h"
#include "SVM_train.h"
#define CVUI_IMPLEMENTATION 
#define KEYDOWN(VK_RETURN) ((GetAsyncKeyState(VK_UP) & 0x8000) ? 1 : 0) //抓握
#define KEYUP(VK_RETURN) ((GetAsyncKeyState(VK_UP) & 0x8000) ? 0 : 1) 
#define KEYDOWN2(VK_RETURN) ((GetAsyncKeyState(VK_DOWN) & 0x8000) ? 1 : 0) //放大缩小
#define KEYDOWN3(VK_RETURN) ((GetAsyncKeyState(VK_LEFT) & 0x8000) ? 1 : 0) 

#include "E:/opencv3.4.1/cvui.h"
#include "E:/graphutils-master/graphutils.h"

using namespace cv;
using namespace rs2;
using namespace myo;
using namespace std;

const int winHeight =  480;	//窗口的高
const int winWidth =  640;	//窗口的宽
const int binHeight = 240;	//窗口的高
const int binWidth = 320;	//窗口的宽
int temp_i = 0;
bool once = true;
int action = 0;
string button1="Start";
string button2 = "Train";
string button3 = "Test";
float tmpWindata[8][50];
/****************************缩小图像**************************************/
Rect operator*(Rect rectFace, float b)
{
	//Rect rectFace;
	rectFace.x = rectFace.x*b; //- (b-1) * (rectFace.width);
	rectFace.y = rectFace.y*b;// - (b-1)* (rectFace.height);
	rectFace.height = rectFace.height*(b);
	rectFace.width = rectFace.width*(b);

	return rectFace;
}
/***************************获取端点坐标***********************************/
void getEndpoints(vector< Point> points, Mat image,
	Point& pointO, Point& pointA)
{
	vector< Point>::iterator it;//迭代输出端点
	Point point;//当前端点
	int size = 1;//防止kmeans函数输入为0
	int clusterCount = 2;//分成2类
	if (points.size()) {
		size = points.size();
	}
	Mat mypoints(size, 1, CV_32FC2), labels;//存储端点数，实际上为2通道的列向量，元素类型为Point2f
	Mat centers(clusterCount, 1, mypoints.type());//用来存储聚类后的中心点
	int i = 0;
	for (it = points.begin(); it != points.end(); it++) {
		point = *it;
		float *p = mypoints.ptr<float>(i++);//端点坐标放入mypoints
		*p++ = it->x;
		*p = it->y;
	}
	if (mypoints.rows >= clusterCount) {		//如果端点大于clusterCount，可以进行分类
		kmeans(mypoints, clusterCount, labels,
			TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
			3, KMEANS_PP_CENTERS, centers);  //聚类3次，取结果最好的那次，聚类的初始化采用PP特定的随机算法。
		float *first_xy = centers.ptr<float>(0);
		float *second_xy = centers.ptr<float>(1);

		pointO = Point(first_xy[0], first_xy[1]);
		pointA = Point(second_xy[0], second_xy[1]);

		int first_d = image.at< Vec3b>(Point(pointO.x, pointO.y))[0];
		int second_d = image.at< Vec3b>(Point(pointA.x, pointA.y))[0];//
		if (first_d > second_d && first_d != 0 && second_d != 0 && first_d != 255 && second_d != 255) {
			Point tmp;
			tmp = pointO;
			pointO = pointA;
			pointA = tmp;
		}

	}
}
//int ball_xx[10];
//int ball_yy[10];
////bool ret = false;
//void rand_tenball()
//{
//	
//	int ball_low = 180, ball_high = 400;//球随机出现的范围
//	for (int i = 0; i < 10; i++)
//	{
//		//srand(time(NULL));
//		ball_xx[i] = rand() % (ball_high - ball_low + 1) + ball_low;//球的坐标
//		ball_yy[i] = rand() % (ball_high - ball_low + 1) + ball_low;
//		//if (ball_xx[i] == ball_xx[i - 1])ret = true;
//		//if (!ret) { i++; ret = false; }
//	}
//
//}
/************************************主函数****************************************/
int main(int argc, char * argv[]) try {
	///////////////////////摄像头初始化//////////////////////////
	rs2::log_to_console(RS2_LOG_SEVERITY_ERROR);
	rs2::context context;
	auto devs = context.query_devices();  //获取设备列表
	int device_num = devs.size();
	cout << "设备数量: " << device_num << endl;//设备数量
	rs2::device dev = devs[0];// 当无设备连接时此处抛出rs2::error异常
	rs2::config config;
	//设置从设备管道获取的深度图和彩色图的配置对象
	config.enable_stream(RS2_STREAM_COLOR, winWidth, winHeight, RS2_FORMAT_BGR8, 30);//配置彩色图像流：分辨率640*480，图像格式：BGR， 帧率：30帧/秒
	config.enable_stream(RS2_STREAM_DEPTH, winWidth, winHeight, RS2_FORMAT_Z16, 30);//配置深度图像流：分辨率640*480，图像格式：Z16， 帧率：30帧/秒
	rs2::pipeline pipe;//生成Realsense管道，用来封装实际的相机设备
	rs2::pipeline_profile profile = pipe.start(config);//根据给定的配置启动相机管道
	rs2_stream align_to = RS2_STREAM_COLOR;//选择彩色图像数据流来作为对齐对象
	rs2::align align(align_to);
	rs2::frameset CameraData = pipe.wait_for_frames();//等待一帧数据，默认等待5s,堵塞程序直到新的一帧捕获
	auto Processed_CameraData = align.process(CameraData);//获取对齐后的帧
	rs2::depth_frame depth_frame = Processed_CameraData.get_depth_frame(); //获取深度图像数据
	rs2::video_frame  color_frame = Processed_CameraData.get_color_frame();//.apply_filter(c);  //获取彩色图像数据
	rs2::stream_profile depthprofile = depth_frame.get_profile();//深度
	rs2::stream_profile colorprofile = color_frame.get_profile();//彩色

	//获取彩色相机内参
	rs2::video_stream_profile cvsprofile(colorprofile);
	rs2_intrinsics color_intrin = cvsprofile.get_intrinsics();
	cout << "\n彩色内联函数: ";cout << color_intrin.width << "  " << color_intrin.height << "  ";
	cout << color_intrin.ppx << "  " << color_intrin.ppy << "  ";cout << color_intrin.fx << "  " << color_intrin.fy << endl;
	cout << "多项式系数: ";
	for (auto value : color_intrin.coeffs)cout << value << "  ";cout << endl;cout << "畸变模型: " << color_intrin.model << endl;
	//获取深度相机内参
	rs2::video_stream_profile dvsprofile(depthprofile);
	rs2_intrinsics depth_intrin = dvsprofile.get_intrinsics();
	cout << "\n深度内联函数: ";cout << depth_intrin.width << "  " << depth_intrin.height << "  ";
	cout << depth_intrin.ppx << "  " << depth_intrin.ppy << "  ";cout << depth_intrin.fx << "  " << depth_intrin.fy << endl;
	cout << "多项式系数: ";
	for (auto value : depth_intrin.coeffs) cout << value << "  ";cout << endl; cout << "畸变模型: " << depth_intrin.model << endl;//畸变模型
  // 获取深度相机相对于彩色相机的外参，即变换矩阵: P_color = R * P_depth + T
	rs2_extrinsics extrin = depthprofile.get_extrinsics_to(colorprofile);
	cout << "\n深度摄像头的内联函数到彩色摄像头的: \n旋转: " << endl;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			float value = extrin.rotation[3 * i + j];
			cout << value << "  ";
		} cout << endl;
	}
	cout << endl; cout << "转化: ";for (auto value : extrin.translation)cout << value << "  "; cout << endl;
	
	////////////////彩色窗口与二值窗口/////////////////////////
	const char* Color_Win = "原始彩色图";
	namedWindow(Color_Win, WINDOW_AUTOSIZE);
	const char* Bin_Win = "肤色分割图";
	namedWindow(Bin_Win, WINDOW_AUTOSIZE);
	moveWindow(Color_Win, 0, 50);
	moveWindow(Bin_Win, 640, 50);
	/////////////////UI界面初始化///////////////////////////////
	bool Game_start_checked = false;
	bool Game_mode1_checked = false;
	bool Game_mode2_checked = false;
	bool EMGshow_checked = false;
	bool EMGcollect_checked = false;
	bool Action1_checked = false;
	bool Action2_checked = false;
	bool Action3_checked = false;
	bool Action4_checked = false;
	bool Action5_checked = false;
	bool Action6_checked = false;
	int time_mark;//计时标记
	cvui::init(Bin_Win);//初始化界面
	int TotalGoalCount = 0;//进球数统计
	int GoalsCount1 = 0;
	int GoalsCount2 = 0;
	int GoalsCount3 = 0;
	int GoalsCount4 = 0;
	float GoalsTime = 0;//所花费的时间
	srand(time(NULL));
	int depthValue = 880;
	int ball_low = 180, ball_high = 400;//球随机出现的范围
	int ball_x = rand() % (ball_high - ball_low + 1) + ball_low;//球的坐标
	int ball_y = rand() % (ball_high - ball_low + 1) + ball_low;
	//int ball_xx[10];
	//int ball_yy[10];
	//for (int i = 0; i < 10; i++)
	//{
	//	//srand(time(NULL));
	//	ball_xx[i] = rand() % (ball_high - ball_low + 1) + ball_low;//球的坐标
	//	ball_yy[i] = rand() % (ball_high - ball_low + 1) + ball_low;
	//	//if (ball_xx[i] == ball_xx[i - 1])ret = true;
	//	//if (!ret) { i++; ret = false; }
	//}
	Scalar ball_color = Scalar(0, 0, 255);//球的颜色
	int g_FistFlag_a = 0;//是否在远处按下按键
	int g_FistFlag_b = 0;//
	int SphereRadius = 12;//球的半径
	if (!Game_mode1_checked && Game_mode2_checked) { ball_x = 300; ball_y = 300; }
	
	ofstream TXTDataFile;//创建一个写文件对象
	ofstream EMGdataFile;// (train_1_txt, ios::trunc);
	//////////////////UI界面初始化//////////////////////
	bool g_Flag_c = TRUE;//忘了干啥用的，总是也是个保险
	Point pointHand, pointElbow, pointBall;//记录手，肘以及球的位置
	Timer GoalTiming;//得分时间
	HandMovement Hand_Movment;//手的动作
	ImgPretreatment Img_Pretreatment;//图像预处理
	TwoPoints HandToElbow;//OA的角度，手臂的角度
	TwoPoints HandToBall;//OB的距离，手到球的距离
	Arms Arm;//手臂信息
	Face face;//人脸识别
	Rect Rect_Basket_1, Rect_Basket_2, Rect_Basket_3;//球筐
	Scalar Basket_1_Color, Basket_2_Color, Basket_3_Color;//球筐颜色
	int last_time = time(0), time_interval = 0;
	////////////////myo初始化//////////////////////////
  	myo::Hub hub("com.example.emg-data-sample");//
	std::cout << "试图找到一个Myo…" << std::endl;
	myo::Myo* myo = hub.waitForMyo(10000);//我们将尝试寻找一个Myo 10秒钟，如果失败，函数将返回一个空指针。
	if (!myo) {//如果waitForMyo()返回了一个空指针，那么我们就无法找到Myo，因此退出时带有一条错误消息。
		throw std::runtime_error("找不到Myo!");
	}
	std::cout << "连接到Myo臂带!" << std::endl << std::endl;
	myo->setStreamEmg(myo::Myo::streamEmgEnabled);//在找到的Myo上启用EMG流。
	DataCollector collector;//构造DeviceListener的一个实例，以便可以将它注册到中心。
	//Hub::addListener()获取类继承自DeviceListener的任何对象的地址，并将导致Hub::run()将事件发送给所有注册的设备监听器。
	hub.addListener(&collector);

	/**********************************大循环开始*****************************************/
	while (cvGetWindowHandle(Bin_Win)) //&& cvGetWindowHandle(depth_win)
		//&& cvGetWindowHandle(Color_Win)) 
	{
	//////////////时间初始化/////////////////////////////
		time_t now_time = time(NULL);//获取本地时间 
		tm*  t_tm = localtime(&now_time);
	//////////////获取图像///////////////////////////
		CameraData = pipe.wait_for_frames();//等待一帧数据，默认等待5s堵塞程序直到新的一帧捕获
		Processed_CameraData = align.process(CameraData);//获取对齐后的帧
		rs2::depth_frame depth_frame = Processed_CameraData.get_depth_frame(); //获取深度图像数据
		rs2::video_frame  color_frame = Processed_CameraData.get_color_frame();//.apply_filter(c);  //获取彩色图像数据
		rs2::stream_profile dprofile = depth_frame.get_profile();
		rs2::stream_profile cprofile = color_frame.get_profile();

		if (!color_frame || !depth_frame) {  //如果获取不到数据则退出
			cout << "未能读取摄像头信息" << endl;
			break;
		}
		//将彩色图像和深度图像转换为Opencv格式
		Mat Orig_Image(Size(winWidth, winHeight),
			CV_8UC3, (void*)color_frame.get_data(), Mat::AUTO_STEP);
		Mat Depth_Image(Size(winWidth, winHeight),
			CV_16U, (void*)depth_frame.get_data(), Mat::AUTO_STEP);
		Mat Bin_Image(Size(winWidth, winHeight), CV_8UC3, Scalar(255, 255, 255));//存储肤色分割后图像
		Mat Color_Image(Size(winWidth, winHeight), CV_8UC3, Scalar(255, 255, 255));//存储彩色图像
		Mat DepThinForm_Image(Size(winWidth, winHeight), CV_8UC3, Scalar(255, 255, 255));//存储细化图像
	   //w=640,h=480
		flip(Orig_Image, Color_Image, 1);
		flip(Orig_Image, DepThinForm_Image, 1);
		flip(Depth_Image, Depth_Image, 1);
		//orig_image.copyTo(color_image);
		//orig_image.copyTo(depthinform_image);
		//人脸识别
		//face.face_load(color_image);
		//face.reduceimage();
		//face.removeface(depthinform_image);
	//////////////手臂识别//////////////////////////////////
	/****************肤色识别***************************/
		Img_Pretreatment.setImage(Depth_Image, DepThinForm_Image, Bin_Image);
		Img_Pretreatment.TogetForeground(depthValue);//将取得的深度图背景去掉，并在彩色图呈现
		Img_Pretreatment.TogetSkincolor();//分割肤色
		Img_Pretreatment.removeSmallareas(Bin_Image);
		morphologyEx(Bin_Image, Bin_Image, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(3, 3)));
		dilate(Bin_Image, Bin_Image, cv::getStructuringElement(cv::MORPH_RECT,
			cv::Size(3, 3), cv::Point(-1, -1)), cv::Point(-1, -1), 3);//进行膨胀操作
		erode(Bin_Image, Bin_Image, cv::getStructuringElement(cv::MORPH_RECT,
			cv::Size(3, 3), cv::Point(-1, -1)), cv::Point(-1, -1), 5); //腐蚀
	/****************端点识别*************************/
		cvtColor(Bin_Image, Bin_Image, CV_BGR2GRAY);
		threshold(Bin_Image, Bin_Image, 100, 255, THRESH_BINARY);
		Mat Thin_Bin_Image = Arm.thinImage(Bin_Image);//图像细化，骨骼化  
		Arm.filterOver(Thin_Bin_Image);//过滤细化后的图像
		vector< Point> points = Arm.getPoints(Thin_Bin_Image, 6, 90, 5);//查找端点和交叉点 900 5 
		pointBall = Point((ball_x), (ball_y)); //球位置
		getEndpoints(points, DepThinForm_Image, pointHand, pointElbow);//得到端点
	/////////////////游戏按键//////////////////////////////
		if (Game_start_checked) {//游戏开始按键
			//画手臂
			cvui::checkbox(Bin_Image, 120, 70, "Mode1", &Game_mode1_checked);
			cvui::checkbox(Bin_Image, 120, 90, "Mode2", &Game_mode2_checked);

			HandToElbow.get_two_point(pointElbow, pointHand);
			HandToElbow.angle();
			pointHand.x = pointHand.x;// +cos(HandToElbow.angle()* PI / 180.0f) * 100;
			pointHand.y = pointHand.y;// - sin(HandToElbow.angle()* PI / 180.0f) * 100;
			HandToBall.get_two_point(pointHand, pointBall);
			HandToBall.distance();
			HandToBall.draw_arm(Color_Image, pointHand, pointElbow);
			Hand_Movment.setMovement(pointHand, HandToElbow.angle());//画手
			if (KEYDOWN(VK_UP) || action == 4 || action == 5 || action == 6) {//如果按键按下，握拳
				Hand_Movment.Fist(Color_Image);
				if (HandToBall.distance() > 100) g_FistFlag_b = 0;
			}
			else if (KEYUP(VK_UP)||action == 1 || action == 2 || action == 3) {
				g_FistFlag_b = 1;
				Hand_Movment.Relax(Color_Image);
				ball_color = Scalar(0, 0, 255);
			}
			switch (g_FistFlag_a) {//抓球动作
			case 0: if (HandToBall.distance() <= 100) { g_FistFlag_a = 2; } break;
			case 2:
				if (g_FistFlag_b != 0 && (KEYDOWN(VK_UP) || action == 4 || action == 5 || action == 6)) {
					//circle(color_image, Point(ball_x, ball_y), 6, Scalar(0, 255, 255), 12);
					ball_x = pointHand.x + 35 * cos((HandToElbow.angle() + 15) * PI / 180.0f);
					ball_y = pointHand.y - 35 * sin((HandToElbow.angle() + 15) * PI / 180.0f);
					ball_color = Scalar(0, 255, 255);
					//circle(color_image, Point(ball_x, ball_y), 6, Scalar(0, 255, 255), 12);
				}
				else if (HandToBall.distance() < 50 && SphereRadius < 24 && KEYDOWN2(VK_DOWN)) {
					SphereRadius++;
					ball_color = Scalar(255, 0, 255);
					Hand_Movment.Fist(Color_Image);
				}
				else if (HandToBall.distance() < 50 && SphereRadius > 6 && KEYDOWN3(VK_LEFT)) {
					SphereRadius--;
					Hand_Movment.Fist(Color_Image);
					ball_color = Scalar(255, 0, 255);
				}
				else {
					ball_color = Scalar(0, 0, 255);
				}break;
			default: break;
			}
			if (ball_x > 640)ball_x = 630;
			if (ball_x < 0)ball_x = 10;
			if (ball_y > 480)ball_y = 470;
			if (ball_y < 0)ball_y = 10 ;
			//如果手碰到球 球移动
			if (ball_x <= 630 && ball_x > 10 && ball_y <= 470 && ball_y > 10) {
				if ((HandToBall.distance() < 30)) {
					if ((pointHand.x > pointElbow.x)) {
						ball_x = ball_x + 3;
					}
					else { ball_x = ball_x - 3; }
					if ((pointHand.y > pointElbow.y)) {
						ball_y = ball_y + 3;
					}
					else { ball_y = ball_y - 3; }
				}
			}
			//模式1
			if (Game_mode1_checked)//&& !Game_mode2_checked)
			{
				Game_mode2_checked = false;
				time_mark = 1;
				//随机球出现
				circle(Color_Image, Point(ball_x, ball_y), SphereRadius, ball_color, -1);
				//3个框出现
				Rect_Basket_1 = Rect(70, 80, 100, 100);
				Rect_Basket_2 = Rect(450, 80, 100, 100);
				Rect_Basket_3 = Rect(260, 280, 100, 100);
				Basket_1_Color = Scalar(175, 255, 75);
				Basket_2_Color = Scalar(175, 255, 75);
				Basket_3_Color = Scalar(175, 255, 75);
				if (TotalGoalCount == 10)
				{
					TXTDataFile.open("data.txt", ofstream::app);
					TXTDataFile << "模式一10次任务总共用时：" << GoalsTime << "s" << endl;
					TXTDataFile << asctime(t_tm) << endl;
					TXTDataFile.close();
					time_mark = 0;
					TotalGoalCount = 0;
					GoalsCount1 = 0;
					GoalsCount2 = 0;
					GoalsCount3 = 0;
					GoalsCount4 = 0;
					last_time = time(0);
					time_interval = 0;
					Game_mode1_checked = FALSE;
					GoalsTime = 0;
				}
				//判断球在哪个框，框变色
				if (ball_x > Rect_Basket_1.tl().x&&ball_x<Rect_Basket_1.br().x&&ball_y>Rect_Basket_1.tl().y&&ball_y < Rect_Basket_1.br().y)
				{
					Basket_1_Color = Scalar(0, 0, 255);
					time_interval = time(0) - last_time;
					last_time = time(0);

					if (g_Flag_c)
					{
						if (time_interval != 0)
						{
							TotalGoalCount++;
							GoalsCount1++;
							TXTDataFile.open("data.txt", ofstream::app);
							TXTDataFile << "左上框第 " << GoalsCount1 << " 次：" << time_interval << " s" << endl;
							TXTDataFile.close();
							GoalsTime = GoalsTime + time_interval;
						}

						ball_x = rand() % (ball_high - ball_low + 1) + ball_low;//球的坐标
						ball_y = rand() % (ball_high - ball_low + 1) + ball_low;
						g_Flag_c = FALSE;
						
					}
				}
				else if (ball_x > Rect_Basket_2.tl().x&&ball_x<Rect_Basket_2.br().x&&ball_y>Rect_Basket_2.tl().y&&ball_y < Rect_Basket_2.br().y)
				{
					Basket_2_Color = Scalar(0, 0, 255);
					time_interval = time(0) - last_time;
					last_time = time(0);
					if (g_Flag_c)
					{
						if (time_interval != 0)
						{
							TotalGoalCount++;
							GoalsCount2++;
							TXTDataFile.open("data.txt", ofstream::app);
							TXTDataFile << "右上框第 " << GoalsCount2 << " 次：" << time_interval << " s" << endl;
							TXTDataFile.close();
							GoalsTime = GoalsTime + time_interval;
						}
						ball_x = rand() % (ball_high - ball_low + 1) + ball_low;//球的坐标
						ball_y = rand() % (ball_high - ball_low + 1) + ball_low;
						g_Flag_c = FALSE;
					}
				}
				else if (ball_x > Rect_Basket_3.tl().x&&ball_x<Rect_Basket_3.br().x&&ball_y>Rect_Basket_3.tl().y&&ball_y < Rect_Basket_3.br().y)
				{
					Basket_3_Color = Scalar(0, 0, 255);
					time_interval = time(0) - last_time;
					last_time = time(0);			
					if (g_Flag_c)
					{
						if (time_interval != 0)
						{
							TotalGoalCount++;
							GoalsCount3++;
							TXTDataFile.open("data.txt", ofstream::app);
							TXTDataFile << "中间框第 " << GoalsCount3 << " 次：" << time_interval << " s" << endl;
							TXTDataFile.close();
							GoalsTime = GoalsTime + time_interval;
						}
						ball_x = rand() % (ball_high - ball_low + 1) + ball_low;//球的坐标
						ball_y = rand() % (ball_high - ball_low + 1) + ball_low;
						g_Flag_c = FALSE;
					}
				}
				if (g_Flag_c == FALSE)
					g_Flag_c = TRUE; 
				char tmpbuf[4];
				sprintf_s(tmpbuf, 4, "%d", GoalsCount1);
				putText(Color_Image, tmpbuf, Point(Rect_Basket_1.x + 30, Rect_Basket_1.y + 65), FONT_HERSHEY_COMPLEX, 2, Scalar(0, 255, 255), 3, 8, 0);
				sprintf_s(tmpbuf, 4, "%d", GoalsCount2);
				putText(Color_Image, tmpbuf, Point(Rect_Basket_2.x + 30, Rect_Basket_2.y + 65), FONT_HERSHEY_COMPLEX, 2, Scalar(0, 255, 255), 3, 8, 0);
				sprintf_s(tmpbuf, 4, "%d", GoalsCount3);
				putText(Color_Image, tmpbuf, Point(Rect_Basket_3.x + 30, Rect_Basket_3.y + 65), FONT_HERSHEY_COMPLEX, 2, Scalar(0, 255, 255), 3, 8, 0);
				
				rectangle(Color_Image, Rect_Basket_1, Basket_1_Color, 4, CV_AA, 0);
				rectangle(Color_Image, Rect_Basket_2, Basket_2_Color, 4, CV_AA, 0);
				rectangle(Color_Image, Rect_Basket_3, Basket_3_Color, 4, CV_AA, 0);
			}
			else if (Game_mode2_checked)
			{
				Game_mode1_checked = false;
				time_mark = 1;
				circle(Color_Image, Point(ball_x, ball_y), SphereRadius, ball_color, -1);//随机出现球
				Rect_Basket_1 = Rect(260, 150, 100, 100);
				Basket_1_Color = Scalar(175, 255, 75);
				if (TotalGoalCount == 10)
				{
					TXTDataFile.open("data.txt", ofstream::app);
					TXTDataFile << "模式二10次任务总共用时：" << GoalsTime << "s" << endl;
					TXTDataFile << asctime(t_tm) << endl;
					TXTDataFile.close();
					time_mark = 0;
					TotalGoalCount = 0;
					GoalsCount1 = 0;
					last_time = time(0);
					time_interval = 0;
					Game_mode2_checked = FALSE;
					GoalsTime = 0;
				}
				if (ball_x > Rect_Basket_1.tl().x&&ball_x<Rect_Basket_1.br().x&&ball_y>Rect_Basket_1.tl().y&&ball_y < Rect_Basket_1.br().y)
				{
					Basket_1_Color = Scalar(0, 0, 255);
					time_interval = time(0) - last_time;
					last_time = time(0);
					
					if (g_Flag_c)
					{

						if (time_interval != 0)
						{
							TotalGoalCount++;
							GoalsCount1++;
							TXTDataFile.open("data.txt", ofstream::app);
							TXTDataFile << "中间框第 " << GoalsCount1 << " 次：" << time_interval << " s" << endl;
							TXTDataFile.close();
							GoalsTime = GoalsTime + time_interval;
						}
						ball_x = rand() % (ball_high - 10 + 1) + 10;//球的坐标
						ball_y = rand() % (ball_high - 10 + 1) + 10;
						g_Flag_c = FALSE;
					}
				}
				g_Flag_c = TRUE;
				if (g_Flag_c == FALSE)
					g_Flag_c = TRUE;
				char tmpbuf[4];
				sprintf_s(tmpbuf, 4, "%d", GoalsCount1);
				putText(Color_Image, tmpbuf, Point(Rect_Basket_1.x + 30, Rect_Basket_1.y + 65), FONT_HERSHEY_COMPLEX, 2, Scalar(0, 255, 255), 3, 8, 0);
				rectangle(Color_Image, Rect_Basket_1, Basket_1_Color, 4, CV_AA, 0);
			}
			else {
				time_mark = 0;
				TotalGoalCount = 0;
				GoalsCount1 = 0;
				GoalsCount2 = 0;
				GoalsCount3 = 0;
				GoalsCount4 = 0;
				last_time = time(0);
				time_interval = 0;
				GoalsTime = 0;
			}
			
			cvui::text(Bin_Image, 190, 75, "Times:");
			cvui::printf(Bin_Image, 230, 75, 0.4, 0xffffff, " %d", TotalGoalCount);
			cvui::text(Bin_Image, 190, 95, "Times:");
			cvui::printf(Bin_Image, 230, 95, 0.4, 0xffffff, " %ds", time_interval);
			switch (time_mark)
			{
			case 0:GoalTiming.Stop(); break;
			case 1:GoalTiming.Start(); break;
			default: break;
			}
			if (!GoalTiming.isStop())
			{
				GoalTiming.show(Color_Image);
			}
		imshow(Color_Win, Color_Image);//彩色图
		}
		else {
		    destroyWindow(Color_Win);
			Game_mode1_checked = false;
			Game_mode2_checked = false;
			GoalTiming.Stop();
			TotalGoalCount = 0;
			GoalsCount1 = 0;
			GoalsCount2 = 0;
			GoalsCount3 = 0;
			GoalsCount4 = 0;
			//depthValue = 880;
			last_time = time(0);
		}
		if (EMGcollect_checked) {
			cvui::checkbox(Bin_Image, 250, 135, "NM1", &Action1_checked);
			cvui::checkbox(Bin_Image, 250, 165, "NM2", &Action2_checked);
			cvui::checkbox(Bin_Image, 250, 195, "NM3", &Action3_checked);
			cvui::checkbox(Bin_Image, 330, 135, "HC1", &Action4_checked);
			cvui::checkbox(Bin_Image, 330, 165, "HC2", &Action5_checked);
			cvui::checkbox(Bin_Image, 330, 195, "HC3", &Action6_checked);
			if (cvui::button(Bin_Image, 20, 130, button1) && button1 == "Start")
			{
				button1 = "End";
				
			}
			else if (cvui::button(Bin_Image, 20,130, button1) && button1 == "End")
			{
				button1 = "Start";

			}
			if (button1 =="End")
			{
				if (Action1_checked)
				{
					//Action1_checked = false;
						Action2_checked = false;
						Action3_checked = false;
						Action4_checked = false;
						Action5_checked = false;
						Action6_checked = false;
						collector.collect(0);
				}
				 if (Action2_checked)
				{
						 Action1_checked = false;
						 //Action2_checked = false;
						 Action3_checked = false;
						 Action4_checked = false;
						 Action5_checked = false;
						 Action6_checked = false;
						 collector.collect(1);
					 
				}
				 if (Action3_checked)
				{
					Action1_checked = false;
					Action2_checked = false;
					//Action3_checked = false;
					Action4_checked = false;
					Action5_checked = false;
					Action6_checked = false;
					collector.collect(2);
				}
				 if (Action4_checked)
				{
					Action1_checked = false;
					Action2_checked = false;
					Action3_checked = false;
					//Action4_checked = false;
					Action5_checked = false;
					Action6_checked = false;
					collector.collect(3);
				}
				 if (Action5_checked)
				{
					Action1_checked = false;
					Action2_checked = false;
					Action3_checked = false;
					Action4_checked = false;
					//Action5_checked = false;
					Action6_checked = false;
					collector.collect(4);
				} if (Action6_checked)
				{
					Action1_checked = false;
					Action2_checked = false;
					Action3_checked = false;
					Action4_checked = false;
					Action5_checked = false;
					//Action6_checked = false;
					collector.collect(5);
				}
			}
			if (button1 == "Start")
			{
				if (Action1_checked)
				{
					//Action1_checked = false;
					Action2_checked = false;
					Action3_checked = false;
					Action4_checked = false;
					Action5_checked = false;
					Action6_checked = false;
					//collector.collect(0);
				}
				 if (Action2_checked)
				{
					Action1_checked = false;
					//Action2_checked = false;
					Action3_checked = false;
					Action4_checked = false;
					Action5_checked = false;
					Action6_checked = false;
					//collector.collect(1);
				}
			 if (Action3_checked)
				{
					Action1_checked = false;
					Action2_checked = false;
					//Action3_checked = false;
					Action4_checked = false;
					Action5_checked = false;
					Action6_checked = false;
					//collector.collect(2);
				}
				 if (Action4_checked)
				{
					Action1_checked = false;
					Action2_checked = false;
					Action3_checked = false;
					//Action4_checked = false;
					Action5_checked = false;
					Action6_checked = false;
					//collector.collect(3);
				}
				 if (Action5_checked)
				{
					Action1_checked = false;
					Action2_checked = false;
					Action3_checked = false;
					Action4_checked = false;
					//Action5_checked = false;
					Action6_checked = false;
					//collector.collect(4);
				}if (Action6_checked)
				{
					Action1_checked = false;
					Action2_checked = false;
					Action3_checked = false;
					Action4_checked = false;
					Action5_checked = false;
					//Action6_checked = false;
					//collector.collect(4);
				}
			}
			if (cvui::button(Bin_Image, 100, 130, button2))
			{
				SVMtrain();
			}
			/*if (cvui::button(Bin_Image, 180, 130, button3) )
			{
				

			}*/
		}else
		{
			button1 = "Start";
			Action1_checked = false;
			Action2_checked = false;
			Action3_checked = false;
			Action4_checked = false;
			Action5_checked = false;
			Action6_checked = false;
		}
	///////////////UI界面显示//////////////////////////////	
		cvui::text(Bin_Image, 20, 25, "Depth(cm): ", 0.4);
		cvui::trackbar(Bin_Image, 90, 15, 500, &depthValue, 500, 1500);
		cvui::checkbox(Bin_Image, 20, 70, "Game Start", &Game_start_checked);
		cvui::checkbox(Bin_Image, 20,90, "EMG Show", &EMGshow_checked);
		cvui::checkbox(Bin_Image, 20, 110, "Collect EMG", &EMGcollect_checked);
		//cvui::printf(Bin_Image, 215, 450, 0.4, 0xffffff, " %s", asctime(t_tm));
		cvui::printf(Bin_Image, 225, 450, 0.6, 0xffffff,"%d-%d-%d %d:%d:%d",
			1900 + t_tm->tm_year,
			1 + t_tm->tm_mon,
			t_tm->tm_mday,
			t_tm->tm_hour,
			t_tm->tm_min,
			t_tm->tm_sec);
		//printf("%s", asctime(p));
		cvui::update();
		imshow(Bin_Win, Bin_Image);//肤色二值图
		

	/*	cout << "端点A坐标：" << Point(pointHand.x, pointHand.y) << endl;
		cout << "深度信息：" << (int)DepThinForm_Image.at< Vec3b>(Point(pointHand.x, pointHand.y))[0] << endl;
		cout << "端点B坐标：" << Point(pointElbow.x, pointElbow.y) << endl;
		cout << "深度信息：" <<(int)DepThinForm_Image.at< Vec3b>(Point(pointElbow.x, pointElbow.y))[0] << endl;*/
	//////////////myo//////////////////////////////////////////	
   
     hub.run(1);
	 
	//collector.collect(4);//////////////////采集肌电数据
	for (size_t i = 0; i < 8; i++)
	{
		//cout << (float)collector.emgData[i];
		tmpWindata[i][temp_i]= (float)collector.emgData[i];
	}
	temp_i++;
	//cout << endl;
	if (temp_i==50)temp_i = 0;
	action=SVM_realtimetest(tmpWindata);	
	if (EMGshow_checked)
	{
		collector.print();
	}
	else { destroyWindow("8通道肌电图"); }//once);
	
	waitKey(1);
	}
 //   SVMtrain();
 //   SVMtest();
	return EXIT_SUCCESS;
}
catch (const rs2::error & e) {
	//捕获相机设备的异常
	cerr << " RealSense错误调用 " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << endl;
	return EXIT_FAILURE;
}
catch (const  exception& e) {
	cerr << " 其他错误 : " << e.what() << endl;
	return EXIT_FAILURE;
}

