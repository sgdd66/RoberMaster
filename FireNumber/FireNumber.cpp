#include"FireNumber.h"
//#include"stdafx.h"
#include"DBSCAN.h"

void FindRect(Mat src,vector<Rect> *rects){


	Mat img,gray;

	//ͼ����СΪԭ�е�ʮ����֮һ
	pyrDown(src,img,Size(src.cols/2,src.rows/2));
	pyrDown(img,img,Size(img.cols/2,img.rows/2));
	pyrDown(img,img,Size(img.cols/2,img.rows/2));
	pyrDown(img,img,Size(img.cols/2,img.rows/2));
		
	//ת��Ϊ�Ҷ�ͼ֮����ֵ������Ϊ�ڰ���ɫ
	cvtColor(img,gray,COLOR_BGR2GRAY);
	threshold(gray,gray,150,255,CV_THRESH_BINARY);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(gray,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE);

	Rect rect;
	double **data;
	data=new double*[contours.size()];
	
	for(int i=0;i<contours.size();i++){
		data[i]=new double[2];
		rect=boundingRect(Mat(contours[i]));
		data[i][0]=rect.width;
		data[i][1]=rect.height;
	}
	
	DBSCAN *dbs;
	int *kind;
	//����ͨ�����ظ������࣬�ҵ����и���������ӽ�9����һ��
	
	vector<int> index;
	dbs=new DBSCAN(data,contours.size(),2);
	kind=dbs->cluster(0.08,8);
	index.reserve(50);
	getIndex(kind,contours.size(),&index,9);

	for(int i=0;i<contours.size();i++)
		delete [] data[i];
	delete [] data;

	if(index.size()!=9)
		return;

	for(int i=0;i<9;i++){
		rect=boundingRect(Mat(contours[index[i]]));
		rect.x*=16;
		rect.y*=16;
		rect.width*=16;
		rect.height*=16;
		rects->push_back(rect);
	}
	
}

void getIndex(int* kind,int pointNum,vector<int>* index,int bestValue){

	int max=-1;
	for(int i=0;i<pointNum;i++){
		if(max<kind[i])
			max=kind[i];
	}
	if(max==-1){
		index->clear();
		return;
	}

	int *clusterNum=new int[max];
	for(int i=0;i<max;i++){
		clusterNum[i]=0;
		for(int j=0;j<pointNum;j++){
			if(kind[j]==i+1)
				clusterNum[i]+=1;
		}
		clusterNum[i]=abs(clusterNum[i]-bestValue);
	}

	int min=clusterNum[0];
	int kindIndex=0;
	for(int i=1;i<max;i++){
		if(min>clusterNum[i]){
			min=clusterNum[i];
			kindIndex=i;
		}
	}
	kindIndex+=1;

	index->clear();
	index->reserve(15);

	for(int i=0;i<pointNum;i++){
		if(kind[i]==kindIndex)
			index->push_back(i);
	}
}