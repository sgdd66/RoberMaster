#pragma once
#include"stdafx.h"
class CPosition
{
public:
	//用相机内参数初始化
	CPosition(const Mat &cameraMat,const Mat &distMat);
	//用相机内参数文件初始化
	CPosition(const char* CalibrationResultPath);
	CPosition();
	~CPosition(void);
	//求解相机姿态，采用一个矩形在图片中的映射关系求解，point2d为矩形在图片中四个顶点的位置，顺序任意
	//width与height为实际矩形的宽度与高度
	void getPosition(vector<Point2f> &point2d,double width,double height);

	//拍摄棋盘版，按下s为保存，按下q为退出
	//savefilePath：拍摄文件存储位置，注意是文件夹的位置，不用制定文件名，例如"D:/myfolder"
	//cameraIndex:相机参数编号，0是笔记本相机
	void getChessPhoto(const char* savefilePath,int cameraIndex=0);
	//相机标定
	//chessPhotoPath:存储棋盘图的文件夹，就是getChessPhoto函数的savefilePath
	//angularPt_size:棋盘版角点的尺寸，例如（6，8），6行8列共48个角点
	//square_size:棋盘版每个方块的尺寸，以毫米为单位
	void calibration(const char* chessPhotoPath,const Size angularPt_size,const Size square_size);
	//从文件中读取相机内参数
	void getCameraParam(const char* CalibrationResultPath);
private:
	Mat cameraMat;
	Mat distMat;
	void sortPoint2D(vector<Point2f>& point2d);
public:
	Mat rotMat;
	Mat rotAngle;
	Mat traMat;
	double distance;
	double angle_x,angle_y,angle_z;
};

