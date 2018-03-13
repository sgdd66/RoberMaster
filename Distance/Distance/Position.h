#pragma once
#include"stdafx.h"
class CPosition
{
public:
	//������ڲ�����ʼ��
	CPosition(const Mat &cameraMat,const Mat &distMat);
	//������ڲ����ļ���ʼ��
	CPosition(const char* CalibrationResultPath);
	CPosition();
	~CPosition(void);
	//��������̬������һ��������ͼƬ�е�ӳ���ϵ��⣬point2dΪ������ͼƬ���ĸ������λ�ã�˳������
	//width��heightΪʵ�ʾ��εĿ����߶�
	void getPosition(vector<Point2f> &point2d,double width,double height);

	//�������̰棬����sΪ���棬����qΪ�˳�
	//savefilePath�������ļ��洢λ�ã�ע�����ļ��е�λ�ã������ƶ��ļ���������"D:/myfolder"
	//cameraIndex:���������ţ�0�ǱʼǱ����
	void getChessPhoto(const char* savefilePath,int cameraIndex=0);
	//����궨
	//chessPhotoPath:�洢����ͼ���ļ��У�����getChessPhoto������savefilePath
	//angularPt_size:���̰�ǵ�ĳߴ磬���磨6��8����6��8�й�48���ǵ�
	//square_size:���̰�ÿ������ĳߴ磬�Ժ���Ϊ��λ
	void calibration(const char* chessPhotoPath,const Size angularPt_size,const Size square_size);
	//���ļ��ж�ȡ����ڲ���
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

