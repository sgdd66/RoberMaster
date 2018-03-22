//#include"stdafx.h"
#include"DBSCAN.h"


DBSCAN::DBSCAN(double** data,int pointNum,int dimNum){

	this->uniform(data,pointNum,dimNum);
	this->points.reserve(50);
	for(int i=0;i<pointNum;i++){
		Ind *point=new Ind(data[i],dimNum);
		this->points.push_back(point);
	}
	this->distance=new double*[pointNum];
	for(int i=0;i<pointNum;i++){
		this->distance[i]=new double[pointNum];
		this->distance[i][i]=0;
	}


	for(int i=0;i<pointNum-1;i++){
		for(int j=i+1;j<pointNum;j++){
			distance[i][j]=this->points.at(i)->distance(this->points.at(j));
			distance[j][i]=distance[i][j];			
		}
	}
}

DBSCAN::~DBSCAN(){
	int pointNum=this->points.size();

	for(int i=0;i<pointNum;i++){
		this->points.at(i)->~Ind();
	}

	
	for(int i=0;i<pointNum;i++)
		delete [] this->distance[i];
	delete distance;
	
}

int* DBSCAN::cluster(double Eps,int Mindps){
	Mindps-=1;
	for(int i=0;i<this->points.size()-1;i++){
		for(int j=i+1;j<this->points.size();j++){
			if(this->distance[i][j]<Eps){
				this->points.at(i)->neighbour.push_back(this->points.at(j));
				this->points.at(j)->neighbour.push_back(this->points.at(i));
			}

		}
	}

	for(int i=0;i<this->points.size();i++){
		if(this->points.at(i)->neighbour.size()>=Mindps)
			this->points.at(i)->kind=COREPOINT;
	}

	for(int i=0;i<this->points.size();i++){
		if(this->points.at(i)->neighbour.size()<Mindps){
			Ind *pt=this->points.at(i);
			for(int j=0;j<pt->neighbour.size();j++){
				if(pt->neighbour.at(j)->kind==COREPOINT){
					pt->kind=BORDERPOINT;
					break;
				}
			}
			if(pt->kind==0){
				pt->kind=NOISEPOINT;
				pt->cluster=-1;
			}
		}
	}

	int clusterIndex=1;
	for(int i=0;i<this->points.size();i++){
		if(this->points.at(i)->cluster!=0)
			continue;
		this->points.at(i)->setCluster(clusterIndex);
		clusterIndex++;
	}

	int* answer=new int[this->points.size()];
	for(int i=0;i<this->points.size();i++){
		answer[i]=this->points.at(i)->cluster;
	}
	return answer;
}


Ind::Ind(double* data,int dimNum){
	this->dimension=dimNum;
	this->data=new double[dimNum];
	for(int i=0;i<dimNum;i++)
		this->data[i]=data[i];
	this->neighbour.reserve(50);
	this->kind=0;
	this->cluster=0;
}

Ind::~Ind(){
	delete [] data;
	this->neighbour.clear();
}

double Ind::distance(Ind* point){
	double L=0;
	double x,y;
	for(int i=0;i<this->dimension;i++){
		x=this->data[i];
		y=point->data[i];
		L+=pow(x-y,2);
	}

	return sqrt(L);
}

void Ind::setCluster(int cluster){
	if(this->cluster!=0)
		return;
	this->cluster=cluster;
	for(int i=0;i<this->neighbour.size();i++){
		this->neighbour.at(i)->setCluster(cluster);
	}
}

void DBSCAN::uniform(double** data,int pointNum,int dimNum){
	double min,max;
	
	for(int i=0;i<dimNum;i++){
		min=data[0][i];
		max=data[0][i];
		for(int j=0;j<pointNum;j++){
			if(data[j][i]<min)
				min=data[j][i];
			if(data[j][i]>max)
				max=data[j][i];
		}
		for(int j=0;j<pointNum;j++){
			data[j][i]=(data[j][i]-min)/(max-min);
		}
	}
}