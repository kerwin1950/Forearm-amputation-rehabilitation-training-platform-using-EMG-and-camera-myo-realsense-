//
// @brief        Realsense435i ʶ���ֱ�λ����Ϣ
// @author       Ѧ��ΰ
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
#define KEYDOWN(VK_RETURN) ((GetAsyncKeyState(VK_UP) & 0x8000) ? 1 : 0) //ץ��
#define KEYUP(VK_RETURN) ((GetAsyncKeyState(VK_UP) & 0x8000) ? 0 : 1) 
#define KEYDOWN2(VK_RETURN) ((GetAsyncKeyState(VK_DOWN) & 0x8000) ? 1 : 0) //�Ŵ���С
#define KEYDOWN3(VK_RETURN) ((GetAsyncKeyState(VK_LEFT) & 0x8000) ? 1 : 0) 

#include "E:/opencv3.4.1/cvui.h"
#include "E:/graphutils-master/graphutils.h"

using namespace cv;
using namespace rs2;
using namespace myo;
using namespace std;

const int winHeight =  480;	//���ڵĸ�
const int winWidth =  640;	//���ڵĿ�
const int binHeight = 240;	//���ڵĸ�
const int binWidth = 320;	//���ڵĿ�
int temp_i = 0;
bool once = true;
int action = 0;
string button1="Start";
string button2 = "Train";
string button3 = "Test";
float tmpWindata[8][50];
/****************************��Сͼ��**************************************/
Rect operator*(Rect rectFace, float b)
{
	//Rect rectFace;
	rectFace.x = rectFace.x*b; //- (b-1) * (rectFace.width);
	rectFace.y = rectFace.y*b;// - (b-1)* (rectFace.height);
	rectFace.height = rectFace.height*(b);
	rectFace.width = rectFace.width*(b);

	return rectFace;
}
/***************************��ȡ�˵�����***********************************/
void getEndpoints(vector< Point> points, Mat image,
	Point& pointO, Point& pointA)
{
	vector< Point>::iterator it;//��������˵�
	Point point;//��ǰ�˵�
	int size = 1;//��ֹkmeans��������Ϊ0
	int clusterCount = 2;//�ֳ�2��
	if (points.size()) {
		size = points.size();
	}
	Mat mypoints(size, 1, CV_32FC2), labels;//�洢�˵�����ʵ����Ϊ2ͨ������������Ԫ������ΪPoint2f
	Mat centers(clusterCount, 1, mypoints.type());//�����洢���������ĵ�
	int i = 0;
	for (it = points.begin(); it != points.end(); it++) {
		point = *it;
		float *p = mypoints.ptr<float>(i++);//�˵��������mypoints
		*p++ = it->x;
		*p = it->y;
	}
	if (mypoints.rows >= clusterCount) {		//����˵����clusterCount�����Խ��з���
		kmeans(mypoints, clusterCount, labels,
			TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
			3, KMEANS_PP_CENTERS, centers);  //����3�Σ�ȡ�����õ��ǴΣ�����ĳ�ʼ������PP�ض�������㷨��
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
//	int ball_low = 180, ball_high = 400;//��������ֵķ�Χ
//	for (int i = 0; i < 10; i++)
//	{
//		//srand(time(NULL));
//		ball_xx[i] = rand() % (ball_high - ball_low + 1) + ball_low;//�������
//		ball_yy[i] = rand() % (ball_high - ball_low + 1) + ball_low;
//		//if (ball_xx[i] == ball_xx[i - 1])ret = true;
//		//if (!ret) { i++; ret = false; }
//	}
//
//}
/************************************������****************************************/
int main(int argc, char * argv[]) try {
	///////////////////////����ͷ��ʼ��//////////////////////////
	rs2::log_to_console(RS2_LOG_SEVERITY_ERROR);
	rs2::context context;
	auto devs = context.query_devices();  //��ȡ�豸�б�
	int device_num = devs.size();
	cout << "�豸����: " << device_num << endl;//�豸����
	rs2::device dev = devs[0];// �����豸����ʱ�˴��׳�rs2::error�쳣
	rs2::config config;
	//���ô��豸�ܵ���ȡ�����ͼ�Ͳ�ɫͼ�����ö���
	config.enable_stream(RS2_STREAM_COLOR, winWidth, winHeight, RS2_FORMAT_BGR8, 30);//���ò�ɫͼ�������ֱ���640*480��ͼ���ʽ��BGR�� ֡�ʣ�30֡/��
	config.enable_stream(RS2_STREAM_DEPTH, winWidth, winHeight, RS2_FORMAT_Z16, 30);//�������ͼ�������ֱ���640*480��ͼ���ʽ��Z16�� ֡�ʣ�30֡/��
	rs2::pipeline pipe;//����Realsense�ܵ���������װʵ�ʵ�����豸
	rs2::pipeline_profile profile = pipe.start(config);//���ݸ�����������������ܵ�
	rs2_stream align_to = RS2_STREAM_COLOR;//ѡ���ɫͼ������������Ϊ�������
	rs2::align align(align_to);
	rs2::frameset CameraData = pipe.wait_for_frames();//�ȴ�һ֡���ݣ�Ĭ�ϵȴ�5s,��������ֱ���µ�һ֡����
	auto Processed_CameraData = align.process(CameraData);//��ȡ������֡
	rs2::depth_frame depth_frame = Processed_CameraData.get_depth_frame(); //��ȡ���ͼ������
	rs2::video_frame  color_frame = Processed_CameraData.get_color_frame();//.apply_filter(c);  //��ȡ��ɫͼ������
	rs2::stream_profile depthprofile = depth_frame.get_profile();//���
	rs2::stream_profile colorprofile = color_frame.get_profile();//��ɫ

	//��ȡ��ɫ����ڲ�
	rs2::video_stream_profile cvsprofile(colorprofile);
	rs2_intrinsics color_intrin = cvsprofile.get_intrinsics();
	cout << "\n��ɫ��������: ";cout << color_intrin.width << "  " << color_intrin.height << "  ";
	cout << color_intrin.ppx << "  " << color_intrin.ppy << "  ";cout << color_intrin.fx << "  " << color_intrin.fy << endl;
	cout << "����ʽϵ��: ";
	for (auto value : color_intrin.coeffs)cout << value << "  ";cout << endl;cout << "����ģ��: " << color_intrin.model << endl;
	//��ȡ�������ڲ�
	rs2::video_stream_profile dvsprofile(depthprofile);
	rs2_intrinsics depth_intrin = dvsprofile.get_intrinsics();
	cout << "\n�����������: ";cout << depth_intrin.width << "  " << depth_intrin.height << "  ";
	cout << depth_intrin.ppx << "  " << depth_intrin.ppy << "  ";cout << depth_intrin.fx << "  " << depth_intrin.fy << endl;
	cout << "����ʽϵ��: ";
	for (auto value : depth_intrin.coeffs) cout << value << "  ";cout << endl; cout << "����ģ��: " << depth_intrin.model << endl;//����ģ��
  // ��ȡ����������ڲ�ɫ�������Σ����任����: P_color = R * P_depth + T
	rs2_extrinsics extrin = depthprofile.get_extrinsics_to(colorprofile);
	cout << "\n�������ͷ��������������ɫ����ͷ��: \n��ת: " << endl;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			float value = extrin.rotation[3 * i + j];
			cout << value << "  ";
		} cout << endl;
	}
	cout << endl; cout << "ת��: ";for (auto value : extrin.translation)cout << value << "  "; cout << endl;
	
	////////////////��ɫ�������ֵ����/////////////////////////
	const char* Color_Win = "ԭʼ��ɫͼ";
	namedWindow(Color_Win, WINDOW_AUTOSIZE);
	const char* Bin_Win = "��ɫ�ָ�ͼ";
	namedWindow(Bin_Win, WINDOW_AUTOSIZE);
	moveWindow(Color_Win, 0, 50);
	moveWindow(Bin_Win, 640, 50);
	/////////////////UI�����ʼ��///////////////////////////////
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
	int time_mark;//��ʱ���
	cvui::init(Bin_Win);//��ʼ������
	int TotalGoalCount = 0;//������ͳ��
	int GoalsCount1 = 0;
	int GoalsCount2 = 0;
	int GoalsCount3 = 0;
	int GoalsCount4 = 0;
	float GoalsTime = 0;//�����ѵ�ʱ��
	srand(time(NULL));
	int depthValue = 880;
	int ball_low = 180, ball_high = 400;//��������ֵķ�Χ
	int ball_x = rand() % (ball_high - ball_low + 1) + ball_low;//�������
	int ball_y = rand() % (ball_high - ball_low + 1) + ball_low;
	//int ball_xx[10];
	//int ball_yy[10];
	//for (int i = 0; i < 10; i++)
	//{
	//	//srand(time(NULL));
	//	ball_xx[i] = rand() % (ball_high - ball_low + 1) + ball_low;//�������
	//	ball_yy[i] = rand() % (ball_high - ball_low + 1) + ball_low;
	//	//if (ball_xx[i] == ball_xx[i - 1])ret = true;
	//	//if (!ret) { i++; ret = false; }
	//}
	Scalar ball_color = Scalar(0, 0, 255);//�����ɫ
	int g_FistFlag_a = 0;//�Ƿ���Զ�����°���
	int g_FistFlag_b = 0;//
	int SphereRadius = 12;//��İ뾶
	if (!Game_mode1_checked && Game_mode2_checked) { ball_x = 300; ball_y = 300; }
	
	ofstream TXTDataFile;//����һ��д�ļ�����
	ofstream EMGdataFile;// (train_1_txt, ios::trunc);
	//////////////////UI�����ʼ��//////////////////////
	bool g_Flag_c = TRUE;//���˸�ɶ�õģ�����Ҳ�Ǹ�����
	Point pointHand, pointElbow, pointBall;//��¼�֣����Լ����λ��
	Timer GoalTiming;//�÷�ʱ��
	HandMovement Hand_Movment;//�ֵĶ���
	ImgPretreatment Img_Pretreatment;//ͼ��Ԥ����
	TwoPoints HandToElbow;//OA�ĽǶȣ��ֱ۵ĽǶ�
	TwoPoints HandToBall;//OB�ľ��룬�ֵ���ľ���
	Arms Arm;//�ֱ���Ϣ
	Face face;//����ʶ��
	Rect Rect_Basket_1, Rect_Basket_2, Rect_Basket_3;//���
	Scalar Basket_1_Color, Basket_2_Color, Basket_3_Color;//�����ɫ
	int last_time = time(0), time_interval = 0;
	////////////////myo��ʼ��//////////////////////////
  	myo::Hub hub("com.example.emg-data-sample");//
	std::cout << "��ͼ�ҵ�һ��Myo��" << std::endl;
	myo::Myo* myo = hub.waitForMyo(10000);//���ǽ�����Ѱ��һ��Myo 10���ӣ����ʧ�ܣ�����������һ����ָ�롣
	if (!myo) {//���waitForMyo()������һ����ָ�룬��ô���Ǿ��޷��ҵ�Myo������˳�ʱ����һ��������Ϣ��
		throw std::runtime_error("�Ҳ���Myo!");
	}
	std::cout << "���ӵ�Myo�۴�!" << std::endl << std::endl;
	myo->setStreamEmg(myo::Myo::streamEmgEnabled);//���ҵ���Myo������EMG����
	DataCollector collector;//����DeviceListener��һ��ʵ�����Ա���Խ���ע�ᵽ���ġ�
	//Hub::addListener()��ȡ��̳���DeviceListener���κζ���ĵ�ַ����������Hub::run()���¼����͸�����ע����豸��������
	hub.addListener(&collector);

	/**********************************��ѭ����ʼ*****************************************/
	while (cvGetWindowHandle(Bin_Win)) //&& cvGetWindowHandle(depth_win)
		//&& cvGetWindowHandle(Color_Win)) 
	{
	//////////////ʱ���ʼ��/////////////////////////////
		time_t now_time = time(NULL);//��ȡ����ʱ�� 
		tm*  t_tm = localtime(&now_time);
	//////////////��ȡͼ��///////////////////////////
		CameraData = pipe.wait_for_frames();//�ȴ�һ֡���ݣ�Ĭ�ϵȴ�5s��������ֱ���µ�һ֡����
		Processed_CameraData = align.process(CameraData);//��ȡ������֡
		rs2::depth_frame depth_frame = Processed_CameraData.get_depth_frame(); //��ȡ���ͼ������
		rs2::video_frame  color_frame = Processed_CameraData.get_color_frame();//.apply_filter(c);  //��ȡ��ɫͼ������
		rs2::stream_profile dprofile = depth_frame.get_profile();
		rs2::stream_profile cprofile = color_frame.get_profile();

		if (!color_frame || !depth_frame) {  //�����ȡ�����������˳�
			cout << "δ�ܶ�ȡ����ͷ��Ϣ" << endl;
			break;
		}
		//����ɫͼ������ͼ��ת��ΪOpencv��ʽ
		Mat Orig_Image(Size(winWidth, winHeight),
			CV_8UC3, (void*)color_frame.get_data(), Mat::AUTO_STEP);
		Mat Depth_Image(Size(winWidth, winHeight),
			CV_16U, (void*)depth_frame.get_data(), Mat::AUTO_STEP);
		Mat Bin_Image(Size(winWidth, winHeight), CV_8UC3, Scalar(255, 255, 255));//�洢��ɫ�ָ��ͼ��
		Mat Color_Image(Size(winWidth, winHeight), CV_8UC3, Scalar(255, 255, 255));//�洢��ɫͼ��
		Mat DepThinForm_Image(Size(winWidth, winHeight), CV_8UC3, Scalar(255, 255, 255));//�洢ϸ��ͼ��
	   //w=640,h=480
		flip(Orig_Image, Color_Image, 1);
		flip(Orig_Image, DepThinForm_Image, 1);
		flip(Depth_Image, Depth_Image, 1);
		//orig_image.copyTo(color_image);
		//orig_image.copyTo(depthinform_image);
		//����ʶ��
		//face.face_load(color_image);
		//face.reduceimage();
		//face.removeface(depthinform_image);
	//////////////�ֱ�ʶ��//////////////////////////////////
	/****************��ɫʶ��***************************/
		Img_Pretreatment.setImage(Depth_Image, DepThinForm_Image, Bin_Image);
		Img_Pretreatment.TogetForeground(depthValue);//��ȡ�õ����ͼ����ȥ�������ڲ�ɫͼ����
		Img_Pretreatment.TogetSkincolor();//�ָ��ɫ
		Img_Pretreatment.removeSmallareas(Bin_Image);
		morphologyEx(Bin_Image, Bin_Image, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(3, 3)));
		dilate(Bin_Image, Bin_Image, cv::getStructuringElement(cv::MORPH_RECT,
			cv::Size(3, 3), cv::Point(-1, -1)), cv::Point(-1, -1), 3);//�������Ͳ���
		erode(Bin_Image, Bin_Image, cv::getStructuringElement(cv::MORPH_RECT,
			cv::Size(3, 3), cv::Point(-1, -1)), cv::Point(-1, -1), 5); //��ʴ
	/****************�˵�ʶ��*************************/
		cvtColor(Bin_Image, Bin_Image, CV_BGR2GRAY);
		threshold(Bin_Image, Bin_Image, 100, 255, THRESH_BINARY);
		Mat Thin_Bin_Image = Arm.thinImage(Bin_Image);//ͼ��ϸ����������  
		Arm.filterOver(Thin_Bin_Image);//����ϸ�����ͼ��
		vector< Point> points = Arm.getPoints(Thin_Bin_Image, 6, 90, 5);//���Ҷ˵�ͽ���� 900 5 
		pointBall = Point((ball_x), (ball_y)); //��λ��
		getEndpoints(points, DepThinForm_Image, pointHand, pointElbow);//�õ��˵�
	/////////////////��Ϸ����//////////////////////////////
		if (Game_start_checked) {//��Ϸ��ʼ����
			//���ֱ�
			cvui::checkbox(Bin_Image, 120, 70, "Mode1", &Game_mode1_checked);
			cvui::checkbox(Bin_Image, 120, 90, "Mode2", &Game_mode2_checked);

			HandToElbow.get_two_point(pointElbow, pointHand);
			HandToElbow.angle();
			pointHand.x = pointHand.x;// +cos(HandToElbow.angle()* PI / 180.0f) * 100;
			pointHand.y = pointHand.y;// - sin(HandToElbow.angle()* PI / 180.0f) * 100;
			HandToBall.get_two_point(pointHand, pointBall);
			HandToBall.distance();
			HandToBall.draw_arm(Color_Image, pointHand, pointElbow);
			Hand_Movment.setMovement(pointHand, HandToElbow.angle());//����
			if (KEYDOWN(VK_UP) || action == 4 || action == 5 || action == 6) {//����������£���ȭ
				Hand_Movment.Fist(Color_Image);
				if (HandToBall.distance() > 100) g_FistFlag_b = 0;
			}
			else if (KEYUP(VK_UP)||action == 1 || action == 2 || action == 3) {
				g_FistFlag_b = 1;
				Hand_Movment.Relax(Color_Image);
				ball_color = Scalar(0, 0, 255);
			}
			switch (g_FistFlag_a) {//ץ����
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
			//����������� ���ƶ�
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
			//ģʽ1
			if (Game_mode1_checked)//&& !Game_mode2_checked)
			{
				Game_mode2_checked = false;
				time_mark = 1;
				//��������
				circle(Color_Image, Point(ball_x, ball_y), SphereRadius, ball_color, -1);
				//3�������
				Rect_Basket_1 = Rect(70, 80, 100, 100);
				Rect_Basket_2 = Rect(450, 80, 100, 100);
				Rect_Basket_3 = Rect(260, 280, 100, 100);
				Basket_1_Color = Scalar(175, 255, 75);
				Basket_2_Color = Scalar(175, 255, 75);
				Basket_3_Color = Scalar(175, 255, 75);
				if (TotalGoalCount == 10)
				{
					TXTDataFile.open("data.txt", ofstream::app);
					TXTDataFile << "ģʽһ10�������ܹ���ʱ��" << GoalsTime << "s" << endl;
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
				//�ж������ĸ��򣬿��ɫ
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
							TXTDataFile << "���Ͽ�� " << GoalsCount1 << " �Σ�" << time_interval << " s" << endl;
							TXTDataFile.close();
							GoalsTime = GoalsTime + time_interval;
						}

						ball_x = rand() % (ball_high - ball_low + 1) + ball_low;//�������
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
							TXTDataFile << "���Ͽ�� " << GoalsCount2 << " �Σ�" << time_interval << " s" << endl;
							TXTDataFile.close();
							GoalsTime = GoalsTime + time_interval;
						}
						ball_x = rand() % (ball_high - ball_low + 1) + ball_low;//�������
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
							TXTDataFile << "�м��� " << GoalsCount3 << " �Σ�" << time_interval << " s" << endl;
							TXTDataFile.close();
							GoalsTime = GoalsTime + time_interval;
						}
						ball_x = rand() % (ball_high - ball_low + 1) + ball_low;//�������
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
				circle(Color_Image, Point(ball_x, ball_y), SphereRadius, ball_color, -1);//���������
				Rect_Basket_1 = Rect(260, 150, 100, 100);
				Basket_1_Color = Scalar(175, 255, 75);
				if (TotalGoalCount == 10)
				{
					TXTDataFile.open("data.txt", ofstream::app);
					TXTDataFile << "ģʽ��10�������ܹ���ʱ��" << GoalsTime << "s" << endl;
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
							TXTDataFile << "�м��� " << GoalsCount1 << " �Σ�" << time_interval << " s" << endl;
							TXTDataFile.close();
							GoalsTime = GoalsTime + time_interval;
						}
						ball_x = rand() % (ball_high - 10 + 1) + 10;//�������
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
		imshow(Color_Win, Color_Image);//��ɫͼ
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
	///////////////UI������ʾ//////////////////////////////	
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
		imshow(Bin_Win, Bin_Image);//��ɫ��ֵͼ
		

	/*	cout << "�˵�A���꣺" << Point(pointHand.x, pointHand.y) << endl;
		cout << "�����Ϣ��" << (int)DepThinForm_Image.at< Vec3b>(Point(pointHand.x, pointHand.y))[0] << endl;
		cout << "�˵�B���꣺" << Point(pointElbow.x, pointElbow.y) << endl;
		cout << "�����Ϣ��" <<(int)DepThinForm_Image.at< Vec3b>(Point(pointElbow.x, pointElbow.y))[0] << endl;*/
	//////////////myo//////////////////////////////////////////	
   
     hub.run(1);
	 
	//collector.collect(4);//////////////////�ɼ���������
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
	else { destroyWindow("8ͨ������ͼ"); }//once);
	
	waitKey(1);
	}
 //   SVMtrain();
 //   SVMtest();
	return EXIT_SUCCESS;
}
catch (const rs2::error & e) {
	//��������豸���쳣
	cerr << " RealSense������� " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << endl;
	return EXIT_FAILURE;
}
catch (const  exception& e) {
	cerr << " �������� : " << e.what() << endl;
	return EXIT_FAILURE;
}

