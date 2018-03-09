#include"stdafx.h"
//用于寻找图像中的圆柱区域

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
	//构造函数，
	//source:图像来源，0-摄像头，1-视频文件，2-图片
	FindCylinder(int source,const char* fileName=NULL);
	//图像数据处理
	void ImageProcess(Mat* img);
	//输出灰度数据
	void outputGrayToTxt(Mat* img,const char* destination);
private:
	int findMid(Mat &img);

	section findPanel(float* data,short length); 

	// 滤波,win代表窗口的一半
	void smoothing(float* data, short length, short win);
	// 对数组data从begin到end-1求均值

	float mean(float* data,short length, short begin, short end);
	// 获取数据梯度
	void getGradient(float* data, short length,float* gradient);
	// 标记零点，从负到正为1，从正到负为-1

	void getZeros(float* data, short length,float threshold, float* zeros);
	// 获取最大值的索引，如果数组中有两个最大值，返回第一个的索引

	int getMax(float* data, short begin, short end);

	// 取绝对值

	void abs(float* data,short length);
	// 获取峰值，注意这里的峰值是指超过门限值的上凸区域的最大值

	void getPeak(float* data,short length, float threshold, vector<section>* peaks,const float* zeros);
	// 获取候选区域，实际是搜索峰值两侧的两个过零点
	void getCandidate(vector<section>* candidates, float* zeros,short length);
	// 第一次筛选
	void firstSelect(vector<section>* candidates);

	// 第二轮筛选
	section secondSelect(vector<section>* candidates);

	void showData(float* data,short length);

	int toInteget(double data);
	// 精细调整，重点是对圆柱中心的位置做精细调整，也只返回col
	int fineTune(Mat* img, short col, section row);
public:
	// 筛选圆柱中心线的位置
	int filterMid(float* data, short length);
	void abs(float* data);
};