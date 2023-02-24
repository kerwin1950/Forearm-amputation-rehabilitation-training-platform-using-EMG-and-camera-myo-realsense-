#include "Zhang.h"
Arms::Arms()
{
}
/****************************�ֱ���Ϣ��ȡ**********************************/
// @brief: ������ͼ�����ϸ��,������
// @param: srcΪ����ͼ��,��cvThreshold�����������8λ�Ҷ�ͼ���ʽ��Ԫ����ֻ��0��1,1������Ԫ�أ�0����Ϊ�հ�
// @param: maxIterations���Ƶ���������������������ƣ�Ĭ��Ϊ-1���������Ƶ���������ֱ��������ս��
// @return: Ϊ��srcϸ��������ͼ��,��ʽ��src��ʽ��ͬ��Ԫ����ֻ��0��1,1������Ԫ�أ�0����Ϊ�հ�
Mat Arms::thinImage(const  Mat & src, const int maxIterations) { //ϸ��ͼ�� zhang���ٲ����㷨
	assert(src.type() == CV_8UC1);
	Mat dst;
	int width = src.cols;
	int height = src.rows;
	src.copyTo(dst);
	int count = 0;  //��¼��������  

	while (true) {
		count++;
		if (maxIterations != -1 && count > maxIterations) break;//���ƴ������ҵ�����������  	
		vector<uchar *> mFlag; //���ڱ����Ҫɾ���ĵ�  
	   //�Ե���  
		for (int i = 0; i < height; ++i) {
			uchar * p = dst.ptr<uchar>(i);

			for (int j = 0; j < width; ++j) {
				//��������ĸ����������б��  
				//  p9 p2 p3  
				//  p8 p1 p4  
				//  p7 p6 p5  
				uchar p1 = p[j];

				if (p1 != 1) continue;
				uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
				uchar p8 = (j == 0) ? 0 : *(p + j - 1);
				uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
				uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
				uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
				uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
				uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
				uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);

				if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6) {
					int ap = 0;
					if (p2 == 0 && p3 == 1) ++ap;
					if (p3 == 0 && p4 == 1) ++ap;
					if (p4 == 0 && p5 == 1) ++ap;
					if (p5 == 0 && p6 == 1) ++ap;
					if (p6 == 0 && p7 == 1) ++ap;
					if (p7 == 0 && p8 == 1) ++ap;
					if (p8 == 0 && p9 == 1) ++ap;
					if (p9 == 0 && p2 == 1) ++ap;

					if (ap == 1 && p2 * p4 * p6 == 0 && p4 * p6 * p8 == 0) {//���  
						mFlag.push_back(p + j);
					}
				}
			}
		}

		//����ǵĵ�ɾ��  
		for (vector<uchar *>::iterator i = mFlag.begin(); i != mFlag.end(); ++i) {
			**i = 0;
		}
		//ֱ��û�е����㣬�㷨����  
		if (mFlag.empty()) {
			break;
		}
		else {
			mFlag.clear();//��mFlag���  
		}

		//�Ե���  
		for (int i = 0; i < height; ++i) {
			uchar * p = dst.ptr<uchar>(i);
			for (int j = 0; j < width; ++j) {
				//��������ĸ����������б��  
				//  p9 p2 p3  
				//  p8 p1 p4  
				//  p7 p6 p5  
				uchar p1 = p[j];
				if (p1 != 1) continue;
				uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
				uchar p8 = (j == 0) ? 0 : *(p + j - 1);
				uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
				uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
				uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
				uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
				uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
				uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);

				if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2
					&& (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6) {
					int ap = 0;
					if (p2 == 0 && p3 == 1) ++ap;
					if (p3 == 0 && p4 == 1) ++ap;
					if (p4 == 0 && p5 == 1) ++ap;
					if (p5 == 0 && p6 == 1) ++ap;
					if (p6 == 0 && p7 == 1) ++ap;
					if (p7 == 0 && p8 == 1) ++ap;
					if (p8 == 0 && p9 == 1) ++ap;
					if (p9 == 0 && p2 == 1) ++ap;

					if (ap == 1 && p2 * p4 * p8 == 0 && p2 * p6 * p8 == 0) {//���  

						mFlag.push_back(p + j);
					}
				}
			}
		}

		//����ǵĵ�ɾ��  
		for (vector<uchar *>::iterator i = mFlag.begin();
			i != mFlag.end(); ++i) {
			**i = 0;
		}

		//ֱ��û�е����㣬�㷨����  
		if (mFlag.empty()) {
			break;
		}
		else {
			mFlag.clear();//��mFlag���  
		}
	}
	return dst;
}

// @brief �Թ�����ͼ���ݽ��й��ˣ�ʵ��������֮�����ٸ�һ���հ�����
// @param thinSrcΪ����Ĺ�����ͼ��,8λ�Ҷ�ͼ���ʽ��Ԫ����ֻ��0��1,1������Ԫ�أ�0����Ϊ�հ�
void Arms::filterOver(Mat thinSrc) { //���߳ɵ�
	assert(thinSrc.type() == CV_8UC1);
	int width = thinSrc.cols;
	int height = thinSrc.rows;

	for (int i = 0; i < height; ++i) {
		uchar * p = thinSrc.ptr<uchar>(i);

		for (int j = 0; j < width; ++j) {
			// ʵ��������֮�����ٸ�һ������
			//  p9 p2 p3  
			//  p8 p1 p4  
			//  p7 p6 p5  
			uchar p1 = p[j];

			if (p1 != 1) continue;
			uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
			uchar p8 = (j == 0) ? 0 : *(p + j - 1);
			uchar p2 = (i == 0) ? 0 : *(p - thinSrc.step + j);
			uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - thinSrc.step + j + 1);
			uchar p9 = (i == 0 || j == 0) ? 0 : *(p - thinSrc.step + j - 1);
			uchar p6 = (i == height - 1) ? 0 : *(p + thinSrc.step + j);
			uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + thinSrc.step + j + 1);
			uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + thinSrc.step + j - 1);

			if (p2 + p3 + p8 + p9 >= 1) {
				p[j] = 0;
			}
		}
	}
}

// @brief �ӹ��˺�Ĺ�����ͼ����Ѱ�Ҷ˵�ͽ����
// @param thinSrcΪ����Ĺ��˺������ͼ��,8λ�Ҷ�ͼ���ʽ��Ԫ����ֻ��0��1,1������Ԫ�أ�0����Ϊ�հ�
// @param raudis����뾶���Ե�ǰ���ص�λԲ�ģ���Բ��Χ���жϵ��Ƿ�Ϊ�˵�򽻲��
// @param thresholdMax�������ֵ���������ֵΪ����� 
// @param thresholdMin�˵���ֵ��С�����ֵΪ�˵� 5�ȽϺ�
// @return Ϊ��srcϸ��������ͼ��,��ʽ��src��ʽ��ͬ��Ԫ����ֻ��0��1,1������Ԫ�أ�0����Ϊ�հ�
vector<Point> Arms::getPoints(const Mat &thinSrc, unsigned int raudis , //Ѱ�Ҷ˵�յ�
	unsigned int thresholdMax  , unsigned int thresholdMin ) {
	assert(thinSrc.type() == CV_8UC1);
	int width = thinSrc.cols;
	int height = thinSrc.rows;
	Mat tmp;
	thinSrc.copyTo(tmp);
	vector< Point> points;

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			if (*(tmp.data + tmp.step * i + j) == 0) {
				continue;
			}
			int count = 0;
			for (int k = i - raudis; k < i + raudis + 1; k++) {
				for (int l = j - raudis; l < j + raudis + 1; l++) {
					if (k < 0 || l < 0 || k>height - 1 || l>width - 1) {
						continue;
					}
					else if (*(tmp.data + tmp.step * k + l) == 1) {
						count++;
					}
				}
			}

			if (count > thresholdMax || count < thresholdMin) {
				Point point(j, i);
				points.push_back(point);
			}
		}
	}
	return points;
}

