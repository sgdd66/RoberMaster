
#ifndef STDAFX
#include"stdafx.h"
#define STDAFX
#endif

#define COREPOINT 1
#define BORDERPOINT 2
#define NOISEPOINT 3


//�����࣬��ֱ��ʹ��
class Ind{
public:
	Ind(double* data,int dimNum);
	int dimension;
	vector<Ind*> neighbour;
	//�������
	double distance(Ind* ind);
	double *data;
	~Ind();
	/*0:δ����
	1�����ĵ�
	2���߽��
	3�������*/
	int kind;
	/*0:û�з���ر��
	-1�������Ĵر��
	�������������ֵ��Ϊ�ر��*/
	int cluster;
	//�ݹ���ã����ھӽڵ㻮��Ϊͬһ��
	void setCluster(int cluster);

};


class DBSCAN{
public:
	//dataΪ���ݣ�ÿһ����һ�����壨�㣩��ÿһ����һ��ά��
	DBSCAN(double** data,int pointNum,int dimNum);
	~DBSCAN();
	vector<Ind*> points;
	double** distance;
	//�趨ͬ����Eps�ʹ�����С������ĿMinpts����������Int* �������飬ά����data������ÿ��ֵ��Ӧ
	//����Ĵر��
	int* cluster(double Eps,int Minpts);
	void uniform(double **data,int pointNum,int dimNum);
};

