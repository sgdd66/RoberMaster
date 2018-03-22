#include"stdafx.h"

int main()
{

	Mat img;
	VideoCapture avi("Data/new1.avi");
	avi.read(img);
	imshow("img",img);
	waitKey(0);
	// vector<Rect> rect;
	// FindRect(img,&rect)



	return 0;
}