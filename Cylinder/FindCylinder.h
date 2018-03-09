#include"stdafx.h"
//����Ѱ��ͼ���е�Բ������

struct section{
	int l1;
	int l2;
	int mid;
	int r1;
	int r2;
};

struct myPoint{
	int index;
	float value;
};

class FindCylinder{
public:
	//���캯����
	//source:ͼ����Դ��0-����ͷ��1-��Ƶ�ļ���2-ͼƬ
	FindCylinder(int source,const char* fileName=NULL);
	//ͼ�����ݴ���
	void ImageProcess(Mat* img);
	//����Ҷ�����
	void outputGrayToTxt(Mat* img,const char* destination);
private:
	int findMid(Mat &img);

	section findPanel(float* data,short length); 

	// �˲�,win�����ڵ�һ��
	void smoothing(float* data, short length, short win);
	// ������data��begin��end-1���ֵ

	float mean(float* data,short length, short begin, short end);
	// ��ȡ�����ݶ�
	void getGradient(float* data, short length,float* gradient);
	// �����㣬�Ӹ�����Ϊ1����������Ϊ-1

	void getZeros(float* data, short length,float threshold, float* zeros);
	// ��ȡ���ֵ��������������������������ֵ�����ص�һ��������

	int getMax(float* data, short begin, short end);

	// ȡ����ֵ

	void abs(float* data,short length);
	// ��ȡ��ֵ��ע������ķ�ֵ��ָ��������ֵ����͹��������ֵ

	void getPeak(float* data,short length, float threshold, vector<section>* peaks,const float* zeros);
	// ��ȡ��ѡ����ʵ����������ֵ��������������
	void getCandidate(vector<section>* candidates, float* zeros,short length);
	// ��һ��ɸѡ
	void firstSelect(vector<section>* candidates);

	// �ڶ���ɸѡ
	section secondSelect(vector<section>* candidates);

	void showData(float* data,short length);

	int toInteget(double data);
	// ��ϸ�������ص��Ƕ�Բ�����ĵ�λ������ϸ������Ҳֻ����col
	int fineTune(Mat* img, short col, section row);
public:
	// ɸѡԲ�������ߵ�λ��
	int filterMid(float* data, short length);
	void abs(float* data);
};