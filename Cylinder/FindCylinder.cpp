#include"FindCylinder.h"
#include<fstream>

FindCylinder::FindCylinder(int source,const char* fileName){
	namedWindow("img");
	Mat img;
	if(source==0){
		VideoCapture capture(0);
		while(1){
			capture>>img;
			//this->ImageProcess(&img);
			imshow("img",img);
			waitKey(1);
		}
	}else if(source==1){
		if(fileName==NULL)
			return;
		VideoCapture capture(fileName);
		if(!capture.isOpened()){
			printf("error in open %s",fileName);
			return;
		}
		int index=0;
		while(1){
			if(!capture.read(img)){
				printf("EOF:%s",fileName);
				break;
			}
			pyrDown(img,img,Size(img.cols/2,img.rows/2));
			pyrDown(img,img,Size(img.cols/2,img.rows/2));
			printf("%d\n",index++);
			if(index==45){
				this->outputGrayToTxt(&img,"F:/TestData/img.txt");
				//break;
			}
			this->ImageProcess(&img);
			imshow("img",img);



			waitKey(100);
		}
	}else if(source==2){
		img=imread(fileName);
		pyrDown(img,img,Size(img.cols/2,img.rows/2));
		pyrDown(img,img,Size(img.cols/2,img.rows/2));
		this->outputGrayToTxt(&img,"F:/TestData/img.txt");
		if(img.empty())
			return;
		this->ImageProcess(&img);
		imshow("img",img);
		waitKey(0);
	}

}

void FindCylinder::outputGrayToTxt(Mat* img, const char* destination){
	Mat gray;
	cvtColor(*img,gray,CV_BGR2GRAY);

	char num[10];
	ofstream file(destination,ios::trunc);
	for(int i=0;i<gray.rows;i++){
		for(int j=0;j<gray.cols;j++){
			if(j!=gray.cols-1)
			{
				sprintf(num,"%d ",gray.at<uchar>(i,j));
				file<<num;
			}else{
				sprintf(num,"%d\n\t",gray.at<uchar>(i,j));
				file<<num;
			}
		}
	}
	file.close();
}

void FindCylinder::ImageProcess(Mat* src){
	Mat gray;
	cvtColor(*src,gray,CV_BGR2GRAY);
	int col=this->findMid(gray);
	float* data=new float[gray.rows];
	for(int i=0;i<gray.rows;i++)
		data[i]=gray.at<uchar>(i,col);
	section row=this->findPanel(data,gray.rows);
	//col=this->fineTune(&gray,col,row);
	//Point pt1(col,row.mid);
	//circle(gray,pt1,3,Scalar(255,0,0),2);
	//imshow("im1",gray);

	Point pt(col,row.mid);
	circle(*src,pt,3,Scalar(255,0,0),2);

	delete [] data;
}

int FindCylinder::findMid(Mat &img){
	int row=img.rows;
	int col=img.cols;
	float* y,*z;
	float *g;
	y=new float[col];
	g=new float[col];
	z=new float[col];
	//零点数目统计
	float *zerosNum=new float[col];
	for(int i=0;i<col;i++)
		zerosNum[i]=0;
	//限定采样区域在begin和end之间,采样数num；
	float begin=row*0.1;
	float end=row*0.9;
	float num=30;
	float interval=(end-begin)/num;
	for(int i=begin;i<end;i+=interval){
		for(int j=0;j<col;j++)
			y[j]=img.at<uchar>(i,j);

		this->smoothing(y,col,3);

		this->getGradient(y,col,g);
	
		this->smoothing(g,col,2);

		this->getZeros(g,col,0,z);
		
		for(int j=0;j<col;j++){
			if(z[j]==-1)
				zerosNum[j]=zerosNum[j]+1;
			if(z[j]==1)
				zerosNum[j]=zerosNum[j]-1;
		}	
	}

	this->smoothing(zerosNum,col,3);
	int mid=this->filterMid(zerosNum,col);
	//int mid=this->getMax(zerosNum,0,col);
	delete [] y;
	delete [] g;
	delete [] z;
	delete [] zerosNum;
	return mid;
}

section FindCylinder::findPanel(float* data,short row){
	float* g=new float[row];
	float* z=new float[row];

	this->getGradient(data,row,g);
	this->abs(g,row);
	this->smoothing(g,row,5);
	this->getZeros(g,row,10,z);


	vector<section> candidates;
	candidates.reserve(50);
	this->getPeak(g,row,10,&candidates,z);
	this->getCandidate(&candidates,z,row);
	this->firstSelect(&candidates);
	section region=this->secondSelect(&candidates);
	if (region.mid==0)
		region.mid=row/2;
	if(region.r2==0)
		region.r2=row-1;

	delete [] g;
	delete [] z;
	return region;




}

// 滤波,win代表窗口的一半

void FindCylinder::smoothing(float* data, short length, short win)
{
	if(length<win)
		return;
	float* tmp=new float[length];
	for(int i=0;i<length;i++){
		if(i<win)
			tmp[i]=this->mean(data,length,0,i+win+1);
		else if(i>length-win-1)
			tmp[i]=this->mean(data,length,i-win,length);
		else
			tmp[i]=this->mean(data,length,i-win,i+win+1);
	}
	for(int i=0;i<length;i++)
		data[i]=tmp[i];
	delete [] tmp;
}


// 对数组data从begin到end-1求均值
float FindCylinder::mean(float* data,short length, short begin, short end)
{
	float sum=0;
	if(begin>=end || end>length)
		return -1;

	for(int i=begin;i<end;i++)
		sum+=data[i];

	return sum/(end-begin);
}


// 获取数据梯度
void FindCylinder::getGradient(float* data, short length,float* gradient)
{
	for(int i=0;i<length;i++){
		if(i==0)
			gradient[i]=data[i+1]-data[i];
		else if(i==length-1)
			gradient[i]=data[i]-data[i-1];
		else
			gradient[i]=(data[i+1]-data[i-1])/2.0;
	}
}


// 标记零点，从负到正为1，从正到负为-1
void FindCylinder::getZeros(float* data, short length,float threshold, float* zeros)
{
	if(length<1)
		return;
	zeros[0]=0;
	for(int i=1;i<length;i++){
		zeros[i]=0;
		if(data[i]==threshold){
			if(data[i]>data[i-1])
				zeros[i]=1;
			else if(data[i]<data[i-1])
				zeros[i]=-1;
		}else if((data[i]-threshold)*(data[i-1]-threshold)<0){
			if(data[i]>data[i-1])
				zeros[i]=1;
			else if(data[i]<data[i-1])
				zeros[i]=-1;
		}
	}
}


// 获取最大值的索引，如果数组中有两个最大值，返回第一个的索引
int FindCylinder::getMax(float* data, short begin, short end)
{
	float max=0;
	int index=begin;
	for(int i=begin;i<end;i++){
		if(data[i]>max){
			max=data[i];
			index=i;
		}
	}
	return index;
}


// 取绝对值
void FindCylinder::abs(float* data,short length)
{
	for(int i=0;i<length;i++){
		if(data[i]<0)
			data[i]=-data[i];
	}
}


// 获取峰值，注意这里的峰值是指超过门限值的上凸区域的最大值
void FindCylinder::getPeak(float* data,short length, float threshold, vector<section>* peaks,const float* zeros)
{
	bool isUp=false;
	int begin,end;
	section peak;
	for(int i=0;i<length;i++){
		if(zeros[i]==1){
			begin=i;
			isUp=true;
		}else if(zeros[i]==-1){
			end=i;
			if(isUp){
				peak.mid=this->getMax(data,begin,end);
				peaks->push_back(peak);
			}
			isUp=false;
		}
	}
}


// 获取候选区域，实际是搜索峰值两侧的两个过零点
void FindCylinder::getCandidate(vector<section>* candidates, float* zeros,short length)
{
	int num=candidates->size();
	int peak=0;
	for(int i=0;i<num;i++){
		peak=candidates->at(i).mid;
		for(int j=peak;j>=0;j-=1){
			if(j==peak && zeros[j]==-1)
				continue;
			else if(zeros[j]==1)
				candidates->at(i).l1=j;
			else if(zeros[j]==-1){
				candidates->at(i).l2=j;
				break;
			}else if(j==0)
				candidates->at(i).l2=j;
		}
		for(int j=peak;j<length;j++){
			if(j==peak && zeros[j]==1)
				continue;
			if(zeros[j]==-1)
				candidates->at(i).r1=j;
			else if(zeros[j]==1){
				candidates->at(i).r2=j;
				break;
			}else if(j==length-1){
				candidates->at(i).r2=j;
			}
		}
		if(candidates->at(i).r1<candidates->at(i).mid ||
			candidates->at(i).r1>=candidates->at(i).r2)
			candidates->at(i).r1=(candidates->at(i).mid+candidates->at(i).r2)/2;
		if(candidates->at(i).l1>candidates->at(i).mid ||
			candidates->at(i).l1<=candidates->at(i).l2)
			candidates->at(i).l1=(candidates->at(i).mid+candidates->at(i).l2)/2;

	}
}


// 第一次筛选
void FindCylinder::firstSelect(vector<section>* candidates)
{
	vector<section> tmp;
	tmp.reserve(50);
	section candidate;
	int L1,L2,L3;
	for(int i=candidates->size()-1;i>=0;i--){
		candidate=candidates->at(i);
		L1=candidate.l1-candidate.l2;
		L2=candidate.r1-candidate.l1;
		L3=candidate.r2-candidate.r1;
		if(L3>L1 && L3>L2 && L1>L2){
			tmp.push_back(candidate);
		}
	}
	candidates->clear();
	for(int i=tmp.size()-1;i>=0;i--){
		candidate=tmp.at(i);
		candidates->push_back(candidate);
	}
}


// 第二轮筛选
section FindCylinder::secondSelect(vector<section>* candidates)
{
	section cylinder;
	if(candidates->empty()){
		cylinder.l1=0;
		cylinder.l2=0;
		cylinder.mid=0;
		cylinder.r1=0;
		cylinder.r2=0;
		return cylinder;
	}
	int num=candidates->size();
	float* length=new float[num];
	
	for(int i=0;i<num;i++){
		length[i]=candidates->at(i).r2-candidates->at(i).l2;
	}
	int index=this->getMax(length,0,num);
	delete [] length;
	return candidates->at(index);
}


void FindCylinder::showData(float* data,short num)
{
	for(int i=0;i<num;i++)
		printf("%d:%.2f\n",i,data[i]);
	ofstream file("F:/TestData/test1.txt",ios::trunc);
	char str[10];
	for(int i=0;i<num;i++){
		sprintf(str,"%.2f ",data[i]);
		file<<str;
	}
	file.close();
}


int FindCylinder::toInteget(double data)
{
	
	float left=data-(int)data;

	if(left>=0.5)
		return (int)data+1;
	else if(left<=-0.5)
		return (int)data-1;
	else
		return (int)data;
}


// 精细调整，重点是对圆柱中心的位置做精细调整，也只返回col
int FindCylinder::fineTune(Mat* img, short x, section y)
{
	float *y1,*y2,*g1,*g2,*z1,*z2;
	int row=img->rows;
	int col=img->cols;
	y1=new float[col];
	y2=new float[col];
	g1=new float[col];
	g2=new float[col];
	z1=new float[col];
	z2=new float[col];
	int row1=(y.mid-y.l2)/2;
	int row2=(y.r2-y.mid)/2;
	for(int i=0;i<col;i++){
		y1[i]=img->at<uchar>(row1,i);
		y2[i]=img->at<uchar>(row2,i);
	}
	this->smoothing(y1,col,2);
	this->smoothing(y2,col,2);
	this->getGradient(y1,col,g1);
	this->getGradient(y2,col,g2);
	this->getZeros(g1,col,0,z1);
	this->getZeros(g2,col,0,z2);
	vector<section> candidate1;
	candidate1.reserve(1);
	vector<section> candidate2;
	candidate2.reserve(1);
	section sec1,sec2;
	sec1.mid=x;
	sec2.mid=x;
	candidate1.push_back(sec1);
	candidate2.push_back(sec2);
	this->getCandidate(&candidate1,z1,col);
	this->getCandidate(&candidate2,z2,col);
	sec1=candidate1.at(0);
	sec2=candidate2.at(0);
	int mid1,mid2,l1,r1;
	if(z1[sec1.l1]==1)
		l1=sec1.l1;
	else if(z1[sec1.l2]==1)
		l1=sec1.l2;
	if(z1[sec1.r1]==1)
		r1=sec1.r1;
	else if(z1[sec1.r2]==1)
		r1=sec1.r2;

	mid1=(r1+l1)/2;
	if(z2[sec2.l1]==1)
		l1=sec2.l1;
	else if(z2[sec2.l2]==1)
		l1=sec2.l2;
	if(z2[sec2.r1]==1)
		r1=sec2.r1;
	else if(z2[sec2.r2]==1)
		r1=sec2.r2;
	mid2=(r1+l1)/2;
	delete [] y1;
	delete [] y2;
	delete [] g1;
	delete [] g2;
	delete [] z1;
	delete [] z2;
	return (mid1+mid2)/2;
}


// 筛选圆柱中心线的位置
int FindCylinder::filterMid(float* data, short length)
{
	vector<int> peaks;
	vector<int> valleys;

	//获取所有的峰值和谷值
	for(int i=1;i<length;i++){
		if(data[i]>data[i-1] && data[i]>=data[i+1])
			peaks.push_back(i);

		if(data[i]<data[i-1] && data[i]<=data[i+1])
			valleys.push_back(i);
	}
	//求峰值中的最大值，并筛选出大于最大值*0.8的值
	float max=0;
	for(int i=0;i<peaks.size();i++){
		if(data[peaks.at(i)]>max){
			max=data[peaks.at(i)];
		}
	}
	vector<int> peakCand;
	for(int i=0;i<peaks.size();i++){
		if(data[peaks.at(i)]>max*0.8)
			peakCand.push_back(peaks.at(i));
	}

	//找出谷值中最小的两个值
	float min=0;
	float min_index1,min_index2;
	
	for(int i=0;i<valleys.size();i++){
		if(data[valleys.at(i)]<min){
			min=data[valleys.at(i)];
			min_index1=valleys.at(i);
		}
	}

	min=0;
	for(int i=0;i<valleys.size();i++){
		if(data[valleys.at(i)]<min && valleys.at(i)!=min_index1){
			min=data[valleys.at(i)];
			min_index2=valleys.at(i);
		}
	}
	//从候选峰值中选出距离min_index1和min_index2最近的两个值
	min=((peakCand.at(0)-min_index1)+(peakCand.at(0)-min_index2))/2.0;
	abs(&min);
	int mid=peakCand.at(0);
	float distance=0;
	for(int i=1;i<peakCand.size();i++){
		distance=((peakCand.at(i)-min_index1)+(peakCand.at(i)-min_index2))/2.0;
		abs(&distance);
		if(distance<min){
			mid=peakCand.at(i);
			min=distance;
		}
	}

	return mid;
}


void FindCylinder::abs(float* data)
{
	if(*data<0)
		*data=-*data;
}
