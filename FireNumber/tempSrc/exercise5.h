#include"stdafx.h"

//С�����

void findManInAVI();
void FindRect(Mat src,vector<Rect> *rect);
Point FindMan(Mat src,vector<Rect> rect);
void getIndex(int* kind,int pointNum,vector<int>* index,int bestValue);

