#include"exercise5.h"


void findManInAVI(){
	VideoCapture avi("2.avi");
	if(!avi.isOpened()){
		printf("error in open video file");
		return;
	}
	vector<Rect> rect;
	Point point;
	int picInd=0;
	Mat src;
	while(1){
		if (!avi.read(src)){
			//��ȡ��Ƶ��һ֡  
			break; 
		}
		picInd++;
		printf("%d\n",picInd);
		rect.clear();
		rect.reserve(50);
		FindRect(src,&rect);
		if(rect.size()==0)
			continue;

		point=FindMan(src,rect);
		if(point.x==0 && point.y==0)
			continue;		
		circle(src,point,4,Scalar(0,0,255),2);
		pyrDown(src,src,Size(src.cols/2,src.rows/2));
		imshow("img",src);

		waitKey(50);
	}
	waitKey(0);

}

//�ӷ�������ѡ����Ҫ�����ţ��б��������Ǹ����Ԫ����Ŀ��ӽ�bestValue��
//��������Ŷ�Ӧ�ĸ���������Ŵ���index��
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

//��ͼ�����ҵ�9�����λ���
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

	vector<vector<Point>> contours;
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

//��9��ͼ�����ҵ�С�˵�λ��
Point FindMan(Mat src,vector<Rect> rect){
	Mat gray;
	cvtColor(src,gray,COLOR_BGR2GRAY);
	vector<Mat> roi;
	double **data;
	data=new double*[rect.size()];
	int row,col;
	
	for(int i=0;i<rect.size();i++){
		data[i]=new double[1];
		rect[i].x=rect[i].x+rect[i].width*0.1;
		rect[i].width=rect[i].width*0.8;
		rect[i].y=rect[i].y+rect[i].height*0.1;
		rect[i].height=rect[i].height*0.8;
		Mat mat=gray(rect[i]);
		Canny(mat,mat,15,45,3);
		/*threshold(mat,mat,125,255,CV_THRESH_BINARY);*/

		row=mat.rows;
		col=mat.cols;

		//for(int x=0;x<row;x++){		
		//	if(mat.at<uchar>(x,x)>100){
		//		floodFill(mat,Point(1,1),Scalar(125),0,Scalar(1),Scalar(1));
		//		break;
		//	}			
		//}
		
		
		//imshow("gray",mat);
		//waitKey(500);
		
		data[i][0]=0;
		for(int x=0;x<row;x++){
			for(int y=0;y<col;y++){
				if(mat.at<uchar>(x,y)>125)
					data[i][0]+=1;
			}
		}
		//data[i][0]=(row*col)-data[i][0];
	}

	DBSCAN* dbs;
	int* kind;
	int num;

	for(float i=0;i<1;i+=0.01){
		dbs=new DBSCAN(data,rect.size(),1);
		kind=dbs->cluster(i,8);
		num=0;
		for(int j=0;j<rect.size();j++){
			if(kind[j]==-1)
				num++;
		}
		if(num==1)
			break;
	}
	if(num!=1)
		return Point(0,0);
	for(int j=0;j<rect.size();j++){
		if(kind[j]==-1){
			num=j;
			break;
		}
	}

	return Point(rect[num].x+rect[num].width/2,rect[num].y+rect[num].height/2);

	
}

/*
void findManInAVI(){
	VideoCapture avi("2.avi");
	Mat img,gray,src;
	double **whiteNum;
	int* kind;
	double** L;
	int debug=0;
	//ͼ�����շŴ����
	int ratio=16;
	//���а�ɫ��ͨ��
	vector<Mat> mask_vector;
	Mat maskInAll;
	int row;
	int col;
	Rect comp;
	Mat* mask;
	int flags;
	vector<int> index;
	vector<Rect> rect;
	DBSCAN *dbs;
	vector<Mat> mat;
	Rect* pRect;
	Mat roi;
	int outcome;
	while(1){
		debug++;

		if(debug==26)
			debug=debug;
		if (!avi.read(src))//��ȡ��Ƶ��һ֡  
			break; 

		//ͼ����СΪԭ�е�ʮ����֮һ
		pyrDown(src,img,Size(src.cols/2,src.rows/2));
		pyrDown(img,img,Size(img.cols/2,img.rows/2));
		pyrDown(img,img,Size(img.cols/2,img.rows/2));
		pyrDown(img,img,Size(img.cols/2,img.rows/2));
		
		//ת��Ϊ�Ҷ�ͼ֮����ֵ������Ϊ�ڰ���ɫ
		cvtColor(img,gray,COLOR_BGR2GRAY);
		threshold(gray,gray,150,255,CV_THRESH_BINARY);

		//����ر�����ʼ��
		mask_vector.clear();
		mask_vector.reserve(50);
		maskInAll=Mat::zeros(gray.rows,gray.cols,CV_8UC1);
		row=gray.rows;
		col=gray.cols;

		//�ҵ���ֵ��ͼ������а�ɫ��ͨ��
		for(int i=0;i<row;i++){
			for(int j=0;j<col;j++){
				if(gray.at<uchar>(i,j)==255 && maskInAll.at<uchar>(i,j)!=255){
					mask=new Mat(row+2,col+2,CV_8UC1,Scalar::all(0));
					flags=8 | FLOODFILL_FIXED_RANGE | FLOODFILL_MASK_ONLY |(255<<8);
					floodFill(gray,*mask,Point(j,i),Scalar(255,255,255),&comp,Scalar(5,5,5),Scalar(5,5,5),flags);
					*mask=(*mask)(Rect(1,1,col+1,row+1));
					mask->copyTo(maskInAll,*mask);	
					mask_vector.push_back(*mask);
				}
			}
		}

		imshow("maskInAll",maskInAll);
		
		//����ÿ����ͨ������ظ���
		rect.clear();
		rect.reserve(50);
		L=new double*[mask_vector.size()];
		for(int i=0;i<mask_vector.size();i++){
			rect.push_back(*getRect(mask_vector.at(i)));
			L[i]=new double[2];
			L[i][0]=rect.at(i).width;
			L[i][1]=rect.at(i).height;
		}
		
		//����ͨ�����ظ������࣬�ҵ����и���������ӽ�9����һ��
		dbs=new DBSCAN(L,mask_vector.size(),2);
		kind=dbs->cluster(5,8);
		index.clear();
		getIndex(kind,mask_vector.size(),&index,9);
		delete dbs;
		delete [] kind;
		for(int i=0;i<mask_vector.size();i++)
			delete [] L[i];
		delete [] L;

		if(index.size()==0)
			continue;

		if(debug==12){
			imshow("gray",gray);
			imshow("maskInAll",maskInAll);
			for(int  i=0;i<index.size();i++){
				imshow("mask",mask_vector.at(index.at(i)));
				waitKey(10);		}
		}

		//�����һ�־�����������9�����Ե�һ�־�����������������ͨ�������Ӿ��ε�λ�ã��Դ�Ϊ�������ڶ��ξ���
		if(index.size()>9){
			rect.clear();
			rect.reserve(50);
			L=new double*[index.size()];
			for(int i=0;i<index.size();i++){
				rect.push_back(*getRect(mask_vector.at(index.at(i))));
				L[i]=new double[4];
				L[i][0]=rect.at(i).x;
				L[i][1]=rect.at(i).y;
				L[i][2]=rect.at(i).x+rect.at(i).width;
				L[i][3]=rect.at(i).y+rect.at(i).height;
			}
	
			dbs=new DBSCAN(L,index.size(),4);
			kind=dbs->cluster(45,3);
			getIndex(kind,index.size(),&index,9);
			for(int i=0;i<index.size();i++)
				delete [] L[i];
			delete [] L;
			delete [] kind;
			delete dbs;
		}

		mat.clear();
		mat.reserve(50);
		
		for(int i=0;i<index.size();i++){
			pRect=&rect.at(index.at(i));
			mat.push_back(src(Rect(pRect->x*ratio,pRect->y*ratio,pRect->width*ratio,pRect->height*ratio)));
			//imshow("img",mat.at(i));
			//waitKey(1000);
		}

		outcome=findManInAVI_2(&mat);
		roi=mat.at(outcome);
		circle( roi,Point(roi.cols/2,roi.rows/2),8,Scalar( 0, 0, 255 ),4);
		resize(src,src,Size(600,300));
		imshow("img",src);

		//for(int i=0;i<mask_vector.size();i++){
		//	printf("%d:%d\n",i,kind[i]);
		//	if(kind[i]=2){
		//		imshow("img",*(mask_vector.at(i)));
		//		waitKey(2000);
		//	}
		//}

		waitKey(10);

	}
	

}*/