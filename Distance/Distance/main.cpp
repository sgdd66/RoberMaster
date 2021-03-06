#include "stdafx.h"

#include "Position.h"
void takePhoto();
void calibration();
void getDistance();
void getPerspectiveTransform();
void test();
int main(int *argc,char** argv){
	//test();
	getDistance();
	//calibration();
	return 0;
}

void takePhoto(){

	char filePath[100];
	int index=0;
	sprintf(filePath,"F:/TestData/ChessFile.txt");
	ofstream file(filePath,ios_base::trunc);
	
	int retkey;
	try{
		VideoCapture capture(1);
		while(1){
			Mat frame;
			capture>>frame;
			imshow("Frame",frame);
			retkey=waitKey(30);
			if((char)retkey=='q')
				break;
			else if((char)retkey=='s'){			
				sprintf(filePath,"F:/TestData/%d.jpg",index++);
				imwrite(filePath,frame);
				file<<filePath<<"\n";
			}
		}
		file.close();
	}catch(std::exception &e){
		e.what();
		file.close();
		return;
	}


}

void calibration(){
	ifstream fin("F:/TestData/ChessFile.txt",ios_base::in); /* 标定所用图像文件的路径 */  
    ofstream fout("F:/TestData/CaliberationResult.txt",ios_base::out);  /* 保存标定结果的文件 */    
    //读取每一幅图像，从中提取出角点，然后对角点进行亚像素精确化   
    cout<<"开始提取角点………………\n";  
    Size image_size;  /* 图像的尺寸 */  
    Size board_size = Size(6,8);    /* 标定板上每行、列的角点数 */  
    vector<Point2f> image_points_buf;  /* 缓存每幅图像上检测到的角点 */  
    vector<vector<Point2f>> image_points_seq; /* 保存检测到的所有角点 */  
    string filename;  
    int count= -1 ;//用于存储角点个数。  
	int imageNum=0;/* 图像数量 */ 
	Mat src,gray;
	while (getline(fin,filename))  
    {  
		imageNum++;
		image_points_buf.clear();
		image_points_buf.reserve(60);
		try{
			src=imread(filename); 
		}catch(std::exception &e){
			e.what();
			printf("open file : %s failed",filename);
			return;
		}

        if (imageNum == 1)  //读入第一张图片时获取图像宽高信息  
        {  
            image_size.width = src.cols;  
            image_size.height =src.rows;           
            //cout<<"image_size.width = "<<image_size.width<<endl;  
            //cout<<"image_size.height = "<<image_size.height<<endl;  
        }  
  
        /* 提取角点 */  
        if (0 == findChessboardCorners(src,board_size,image_points_buf))  
        {             
            cout<<"can not find chessboard corners!\n"; //找不到角点  
            exit(1);  
        }   
        else   
        {  
            
            cvtColor(src,gray,CV_RGB2GRAY);  
            /* 亚像素精确化 */  
            find4QuadCornerSubpix(gray,image_points_buf,Size(5,5)); //对粗提取的角点进行精确化  
            //cornerSubPix(gray,image_points_buf,Size(5,5),Size(-1,-1),TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER,30,0.1));  
            image_points_seq.push_back(image_points_buf);  //保存亚像素角点  
            /* 在图像上显示角点位置 */  
            drawChessboardCorners(gray,board_size,image_points_buf,true); //用于在图片中标记角点  
			
			//circle(gray,Point(image_points_buf.at(0).x,image_points_buf.at(0).y),5,Scalar(0,0,0));
   //         circle(gray,Point(image_points_buf.at(6).x,image_points_buf.at(6).y),5,Scalar(0,0,0));
            
			imshow("Camera Calibration",gray);//显示图片  

            waitKey(10);//暂停0.5S         
        }  
    }   

    int CornerNum=board_size.width*board_size.height;  //每张图片上总的角点数  

  
    //以下是摄像机标定  

    //棋盘三维信息
    Size square_size = Size(40,40);  /* 实际测量得到的标定板上每个棋盘格的大小 */  
    vector<vector<Point3f>> pointsCoordinate; /* 保存标定板上角点的三维坐标 */  

    //内外参数 
    Mat cameraMat=Mat(3,3,CV_32FC1,Scalar::all(0)); /* 摄像机内参数矩阵 */  
    vector<int> point_counts;  // 每幅图像中角点的数量  
    Mat distMat=Mat(1,5,CV_32FC1,Scalar::all(0)); /* 摄像机的5个畸变系数：k1,k2,p1,p2,k3 */  
    vector<Mat> rotMat;  /* 每幅图像的旋转向量 */  
    vector<Mat> traMat; /* 每幅图像的平移向量 */  
    //初始化标定板上角点的三维坐标
    int i,j,t;  
	vector<Point3f> tempPointsCoordinate; 
	Point3f realPoint;  
    for (t=0;t<imageNum;t++)   
    {  
		tempPointsCoordinate.clear();
		tempPointsCoordinate.reserve(60);
        for (i=0;i<board_size.height;i++)   
        {  
            for (j=0;j<board_size.width;j++)   
            {  
                /* 假设标定板放在世界坐标系中z=0的平面上 */  
                realPoint.x = i*square_size.width;  
                realPoint.y = j*square_size.height;  
                realPoint.z = 0;  
                tempPointsCoordinate.push_back(realPoint);  
            }  
        }  
        pointsCoordinate.push_back(tempPointsCoordinate);  
    }  
    /* 初始化每幅图像中的角点数量，假定每幅图像中都可以看到完整的标定板 */  
    for (i=0;i<imageNum;i++)  
    {  
        point_counts.push_back(board_size.width*board_size.height);  
    }     

    /* 开始标定 */  
    calibrateCamera(pointsCoordinate,image_points_seq,image_size,cameraMat,distMat,rotMat,traMat,0);  
    cout<<"标定完成！\n";  
    //对标定结果进行评价  
    cout<<"开始评价标定结果………………\n";  
    double total_err = 0.0; /* 所有图像的平均误差的总和 */  
    double err = 0.0; /* 每幅图像的平均误差 */  
    vector<Point2f> image_points_new; /* 保存重新计算得到的投影点 */  
	vector<Point2f> image_points_old; /* 保存原图中的投影点 */ 

    cout<<"\t每幅图像的标定误差：\n";  
    fout<<"每幅图像的标定误差：\n";  
    for (i=0;i<imageNum;i++)  
    {  
        tempPointsCoordinate=pointsCoordinate[i];  
        /* 通过得到的摄像机内外参数，对空间的三维点进行重新投影计算，得到新的投影点 */  
        projectPoints(tempPointsCoordinate,rotMat[i],traMat[i],cameraMat,distMat,image_points_new);  
        /* 计算新的投影点和旧的投影点之间的误差*/  
        image_points_old = image_points_seq[i];  
        Mat oldPointMat = Mat(1,image_points_old.size(), CV_32FC2);  
        Mat newPointMat = Mat(1,image_points_new.size(), CV_32FC2);  
        for (int j = 0 ; j < image_points_old.size(); j++)  
        {  
            oldPointMat.at<Vec2f>(0,j) = Vec2f(image_points_old[j].x, image_points_old[j].y);  
            newPointMat.at<Vec2f>(0,j) = Vec2f(image_points_new[j].x, image_points_new[j].y);  
        }  
        err = norm(newPointMat, oldPointMat, NORM_L2);  
        err /=  point_counts[i];   
		total_err += err;
        std::cout<<"第"<<i+1<<"幅图像的平均误差："<<err<<"像素"<<endl;     
        fout<<"第"<<i+1<<"幅图像的平均误差："<<err<<"像素"<<endl;     
    }     
    std::cout<<"总体平均误差："<<total_err/imageNum<<"像素"<<endl;     
    fout<<"总体平均误差："<<total_err/imageNum<<"像素"<<endl<<endl;     
    std::cout<<"评价完成！"<<endl;   

    //保存定标结果      
    std::cout<<"开始保存定标结果………………"<<endl;         
    Mat rotation_matrix = Mat(3,3,CV_32FC1, Scalar::all(0)); /* 保存每幅图像的旋转矩阵 */ 
	fout<<"图片尺寸："<<endl;
	fout<<"row="<<image_size.height<<endl;
	fout<<"width="<<image_size.width<<endl;
    fout<<"相机内参数矩阵："<<endl;     
    fout<<cameraMat<<endl<<endl;     
    fout<<"畸变系数：\n";     
    fout<<distMat<<endl<<endl<<endl;     
    for (int i=0; i<imageNum; i++)   
    {   
        fout<<"第"<<i+1<<"幅图像的旋转向量："<<endl;     
        fout<<rotMat[i]<<endl;      
        /* 将旋转向量转换为相对应的旋转矩阵 */     
        Rodrigues(rotMat[i],rotation_matrix);     
        fout<<"第"<<i+1<<"幅图像的旋转矩阵："<<endl;     
        fout<<rotation_matrix<<endl;     
        fout<<"第"<<i+1<<"幅图像的平移向量："<<endl;     
        fout<<traMat[i]<<endl<<endl;     
    }     
    std::cout<<"完成保存"<<endl;   
    fout<<endl;  
    ///************************************************************************   
    //显示定标结果   
    //*************************************************************************/  

    Mat mapx = Mat(image_size,CV_32FC1);  
    Mat mapy = Mat(image_size,CV_32FC1);  
    Mat R = Mat::eye(3,3,CV_32F);  
	//求解映射矩阵
	initUndistortRectifyMap(cameraMat,distMat,R,cameraMat,image_size,CV_32FC1,mapx,mapy);
	//fout<<"mapx="<<endl;
	//fout<<mapx<<endl<<endl;
	//fout<<"mapy="<<endl;
	//fout<<mapy<<endl<<endl;

    std::cout<<"保存矫正图像"<<endl;  

	fin.clear(ios::goodbit);
	fin.seekg(0,ios::beg);
      
	int pos;
	Mat newSrc;
    while(getline(fin,filename))
    {  

        src = imread(filename);  
        newSrc = src.clone();  
        //另一种不需要转换矩阵的方式  
        //undistort(imageSource,newimage,cameraMatrix,distCoeffs);  
        remap(src,newSrc,mapx, mapy, INTER_LINEAR);         
		pos=filename.find('.');
		filename.insert(pos,"_t");
        imwrite(filename,newSrc);  
    }  
    std::cout<<"保存结束"<<endl;  
	fin.close();
	fout.close();

    return ;  
}  

void getPerspectiveTransform(){
	//读取标定文件
	ifstream fin("F:/TestData/distance/CaliberationResult.txt",ios_base::in);   
	string fileStr;
	//获取标定数据
	int row,col;
	Mat cameraMat(3,3,CV_32FC1,Scalar(0));
	Mat distMat(1,5,CV_32FC1,Scalar(0));
	string tempStr;
	
	string regStr;
	regex regformat;
	smatch result;
	while(getline(fin,fileStr)){

		regStr="row=(\\d*)";
		regformat.assign(regStr,regex::icase);
		if(regex_match(fileStr,result,regformat)){
			tempStr=result[1];
			row=atoi(tempStr.c_str());
		}

		regStr="width=(\\d*)";
		regformat.assign(regStr,regex::icase);
		if(regex_match(fileStr,result,regformat)){
			tempStr=result[1];
			col=atoi(tempStr.c_str());
		}

		if(fileStr=="相机内参数矩阵："){
			getline(fin,fileStr);
			regStr="\\[(\\d+\\.\\d*).*?(\\d+\\.\\d*);";
			regformat.assign(regStr,regex::icase);
			if(regex_match(fileStr,result,regformat)){
				tempStr=result[1];
				cameraMat.at<float>(0,0)=atof(tempStr.c_str());
				tempStr=result[2];
				cameraMat.at<float>(0,2)=atof(tempStr.c_str());				
			}
			getline(fin,fileStr);
			regStr=".*?(\\d+\\.\\d*), (\\d+\\.\\d*);";
			regformat.assign(regStr,regex::icase);
			if(regex_match(fileStr,result,regformat)){
				tempStr=result[1];
				cameraMat.at<float>(1,1)=atof(tempStr.c_str());
				tempStr=result[2];
				cameraMat.at<float>(1,2)=atof(tempStr.c_str());				
			}

			cameraMat.at<float>(2,2)=1.0;
		}

		if(fileStr=="畸变系数："){
			getline(fin,fileStr);
			regStr="\\[(-*\\d*\\.\\d*), (-*\\d*\\.\\d*), (-*\\d*\\.\\d*), (-*\\d*\\.\\d*), (-*\\d*\\.\\d*)\\]";
			regformat.assign(regStr,regex::icase);
			if(regex_match(fileStr,result,regformat)){
				tempStr=result[1];
				distMat.at<float>(0,0)=atof(tempStr.c_str());
				tempStr=result[2];
				distMat.at<float>(0,1)=atof(tempStr.c_str());
				tempStr=result[3];
				distMat.at<float>(0,2)=atof(tempStr.c_str());
				tempStr=result[4];
				distMat.at<float>(0,3)=atof(tempStr.c_str());
				tempStr=result[5];
				distMat.at<float>(0,4)=atof(tempStr.c_str());
			}
			break;
		}

	}
    Mat mapx = Mat(row,col,CV_32FC1);  
    Mat mapy = Mat(row,col,CV_32FC1);  
    Mat R = Mat::eye(3,3,CV_32F);  
	//求解映射矩阵
	initUndistortRectifyMap(cameraMat,distMat,R,cameraMat,Size(col,row),CV_32FC1,mapx,mapy);
	//矫正图片
	Mat src,newSrc,gray;
	src = imread("F:/TestData/distance/0.jpg",1);  
	newSrc = src.clone();  
	remap(src,newSrc,mapx, mapy, INTER_LINEAR);         
	cvtColor(newSrc,gray,CV_BGR2GRAY);

	//寻找四个亚像素角点	
	vector<Point2f> image_points;
	Size board_size(6,8);
    if (0 == findChessboardCorners(newSrc,board_size,image_points))  
    {             
        cout<<"can not find chessboard corners!\n"; //找不到角点  
        exit(1);  
    }   
    else   
    {  
            
        cvtColor(src,gray,CV_RGB2GRAY);  
        /* 亚像素精确化 */  
        find4QuadCornerSubpix(gray,image_points,Size(5,5));   
    }



	Point2f virtualQuad[4], realQuad[4];
	Mat warp_matrix(3,3,CV_32FC1,Scalar(0));

	virtualQuad[0]=image_points.at(3*6+2);//左下
	virtualQuad[1]=image_points.at(3*6+3);//左上
	virtualQuad[2]=image_points.at(4*6+2);//右下
	virtualQuad[3]=image_points.at(4*6+3);//右上

	realQuad[0]=Point2f(col/2-40,row/2+40);
	realQuad[1]=Point2f(col/2-40,row/2-40);
	realQuad[2]=Point2f(col/2+40,row/2+40);
	realQuad[3]=Point2f(col/2+40,row/2-40);

	warp_matrix=getPerspectiveTransform(virtualQuad,realQuad);
	Mat myMat(3,3,CV_32FC1,Scalar(0));

	warpPerspective(newSrc,src,warp_matrix,Size(col,row));
	imshow("img",src);
	waitKey(0);
	ofstream fout("F:/answer.txt",ios_base::trunc);
	fout<<warp_matrix<<endl;
	fout.close();

}


void getDistance(){
	CPosition position;
	char filePath[100];
	sprintf(filePath,"F:/TestData/CalibrationResult.txt");

	position.getCameraParam(filePath);

	Mat src,gray;
	src = imread("F:/TestData/distance/0.jpg",1);  
	cvtColor(src,gray,CV_BGR2GRAY);

	//寻找四个亚像素角点	
	vector<Point2f> image_points;
	Size board_size(6,8);
    if (0 == findChessboardCorners(src,board_size,image_points))  
    {             
        cout<<"can not find chessboard corners!\n"; //找不到角点  
        exit(1);  
    }   
    else   
    {          
        cvtColor(src,gray,CV_RGB2GRAY);  
        find4QuadCornerSubpix(gray,image_points,Size(5,5));   
    }



	vector<Point2f> virtualQuad;
	virtualQuad.reserve(4);


	virtualQuad.push_back(image_points.at(2*6+1));//左下
	virtualQuad.push_back(image_points.at(2*6+4));//左上
	virtualQuad.push_back(image_points.at(5*6+4));//右上
	virtualQuad.push_back(image_points.at(5*6+1));//右下
	for(int i=0;i<4;i++){
		circle(src,virtualQuad.at(i),4,Scalar(255,0,0));
	}
	imshow("img",src);



	position.getPosition(virtualQuad,12,12);

	cout<<position.distance<<endl;
	cout<<position.traMat<<endl;
	cout<<position.rotAngle<<endl;
	waitKey(0);


}

void test(){
	CPosition position;
	char filePath[100];
	//sprintf(filePath,"F:/Test");
	//position.getChessPhoto(filePath,1);
	sprintf(filePath,"F:/TestData");
	position.calibration(filePath,Size(6,8),Size(40,40));

	position.getCameraParam(filePath);
}