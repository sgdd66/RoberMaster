#include "Position.h"


CPosition::CPosition(const Mat &cameraMat,const Mat &distMat)
{
	cameraMat.copyTo(this->cameraMat);
	distMat.copyTo(this->distMat);
	rotMat = cv::Mat::eye(3,3,CV_64FC1);
	traMat = cv::Mat::zeros(3,1,CV_64FC1);
	rotAngle=cv::Mat::zeros(3,1,CV_64FC1);
	distance=0;
	angle_x=0;
	angle_y=0;
	angle_z=0;

}
CPosition::CPosition(const char* CalibrationResultPath){
	cameraMat= cv::Mat::zeros(3,3,CV_64FC1);
	distMat = cv::Mat::zeros(1,5,CV_64FC1);
	this->getCameraParam(CalibrationResultPath);
	rotMat = cv::Mat::eye(3,3,CV_64FC1);
	traMat = cv::Mat::zeros(3,1,CV_64FC1);
	rotAngle=cv::Mat::zeros(3,1,CV_64FC1);
	distance=0;
	angle_x=0;
	angle_y=0;
	angle_z=0;
}

CPosition::CPosition(){
	cameraMat= cv::Mat::zeros(3,3,CV_64FC1);
	distMat = cv::Mat::zeros(1,5,CV_64FC1);
	rotMat = cv::Mat::eye(3,3,CV_64FC1);
	traMat = cv::Mat::zeros(3,1,CV_64FC1);
	rotAngle=cv::Mat::zeros(3,1,CV_64FC1);
	distance=0;
	angle_x=0;
	angle_y=0;
	angle_z=0;
}


CPosition::~CPosition(void)
{
}

void CPosition::getPosition(vector<Point2f> &point2d,double width,double height){

	if(point2d.size()!=4){
		throw std::exception("the num of point in 2D must be 4");
		return;
	}

	if(width<0 || height<0){
		throw std::exception("the width and height should be longger than 0");
		return;
	}
	this->sortPoint2D(point2d);

	double half_x=width/2;
	double half_y=height/2;

	vector<Point3f> point3d;
	point3d.push_back(Point3f(-half_x, -half_y, 0));
    point3d.push_back(Point3f(-half_x, half_y, 0));
    point3d.push_back(Point3f(half_x, half_y, 0));
    point3d.push_back(Point3f(half_x, -half_y, 0));

    
    cv::solvePnP(point3d, point2d, cameraMat, distMat, rotAngle, traMat);
    Rodrigues(rotAngle, rotMat);

	double x=traMat.at<double>(0,0);
	double y=traMat.at<double>(1,0);
	double z=traMat.at<double>(2,0);

	distance=sqrt(x*x+y*y+z*z);

	angle_x=acos(x/distance);
	angle_y=acos(y/distance);
	angle_z=acos(z/distance);

}

//按照左下，左上，右上，右下排序
void CPosition::sortPoint2D(vector<Point2f>& point2d)
{
	Point2f ptArray[4];
	Point2f tmp;
	for(int i=0;i<4;i++)
		ptArray[i]=point2d.at(i);
	sort(ptArray, ptArray + 4, [](const Point2f & p1, const Point2f & p2) { return p1.x < p2.x; });
    if (ptArray[0].y < ptArray[1].y){
        tmp=ptArray[0];
		ptArray[0]=ptArray[1];
		ptArray[1]=tmp;
    }

    if (ptArray[2].y > ptArray[3].y)	{
        tmp=ptArray[2];
		ptArray[2]=ptArray[3];
		ptArray[3]=tmp;
    }
	point2d.clear();
	point2d.reserve(4);
	for(int i=0;i<4;i++)
		point2d.push_back(ptArray[i]);	

}

void CPosition::getChessPhoto(const char* savefilePath,int cameraIndex){
	char filePath[100];
	sprintf(filePath,"%s/ChessPhotoPath.txt",savefilePath);
	ofstream file(filePath,ios_base::trunc);
	int index=0;
	int retkey;
	try{
		VideoCapture capture(cameraIndex);
		while(1){
			Mat frame;
			capture>>frame;
			imshow("Frame",frame);
			retkey=waitKey(10);
			if((char)retkey=='q')
				break;
			else if((char)retkey=='s'){			
				sprintf(filePath,"%s/%d.jpg",savefilePath,index++);
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

void CPosition::calibration(const char* chessPhotoPath,const Size angularPt_size,const Size square_size){
	char filePath[100];
	sprintf(filePath,"%s/ChessPhotoPath.txt",chessPhotoPath);
	
	ifstream fin(filePath,ios_base::in); /* 标定所用图像文件的路径 */  
	if(!fin.good()){
		cout<<"ChessPhotoPath.txt文件不存在"<<endl;
		return;
	}

	sprintf(filePath,"%s/CaliberationReport.txt",chessPhotoPath);
    ofstream fout(filePath,ios_base::out);  /* 保存标定结果的文件 */    
    //读取每一幅图像，从中提取出角点，然后对角点进行亚像素精确化   
    cout<<"开始提取角点………………\n";  
    Size image_size;  /* 图像的尺寸 */  

    vector<Point2f> image_points_buf;  /* 缓存每幅图像上检测到的角点 */  
    vector<vector<Point2f>> image_points_seq; /* 保存检测到的所有角点 */  
    string filename;  
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
        }  
  
        /* 提取角点 */  
        if (0 == findChessboardCorners(src,angularPt_size,image_points_buf))  
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
            drawChessboardCorners(gray,angularPt_size,image_points_buf,true); //用于在图片中标记角点  
            
			imshow("Camera Calibration",gray);//显示图片  

            waitKey(10);//暂停0.01S         
        }  
    }   

    int CornerNum=angularPt_size.width*angularPt_size.height;  //每张图片上总的角点数  

  
    //以下是摄像机标定  

    //棋盘三维信息
    vector<vector<Point3f>> pointsCoordinate; /* 保存标定板上角点的三维坐标 */  

    //内外参数 
    vector<int> point_counts;  // 每幅图像中角点的数量  
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
        for (i=0;i<angularPt_size.height;i++)   
        {  
            for (j=0;j<angularPt_size.width;j++)   
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
        point_counts.push_back(angularPt_size.width*angularPt_size.height);  
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

	sprintf(filePath,"%s/CalibrationResult.txt",chessPhotoPath);
	fout.open(filePath,ios_base::trunc);
	fout<<"相机内参数矩阵："<<endl;
	fout<<cameraMat<<endl;
	fout<<"畸变系数："<<endl;
	fout<<distMat<<endl;
	fout.close();
}

void CPosition::getCameraParam(const char* CalibrationResultPath){
	ifstream fin(CalibrationResultPath,ios_base::in);
	if(!fin.good()){
		cout<<"can't open "<<CalibrationResultPath<<endl;
		return;
	}
	string fileStr;
	//获取标定数据

	string tempStr;
	
	string regStr;
	regex regformat;
	smatch result;
	while(getline(fin,fileStr)){

		if(fileStr=="相机内参数矩阵："){
			getline(fin,fileStr);
			regStr="\\[(\\d+\\.\\d*).*?(\\d+\\.\\d*);";
			regformat.assign(regStr,regex::icase);
			if(regex_match(fileStr,result,regformat)){
				tempStr=result[1];
				cameraMat.at<double>(0,0)=atof(tempStr.c_str());
				tempStr=result[2];
				cameraMat.at<double>(0,2)=atof(tempStr.c_str());				
			}
			getline(fin,fileStr);
			regStr=".*?(\\d+\\.\\d*), (\\d+\\.\\d*);";
			regformat.assign(regStr,regex::icase);
			if(regex_match(fileStr,result,regformat)){
				tempStr=result[1];
				cameraMat.at<double>(1,1)=atof(tempStr.c_str());
				tempStr=result[2];
				cameraMat.at<double>(1,2)=atof(tempStr.c_str());				
			}

			cameraMat.at<double>(2,2)=1.0;
		}

		if(fileStr=="畸变系数："){
			getline(fin,fileStr);
			regStr="\\[(-*\\d*\\.\\d*), (-*\\d*\\.\\d*), (-*\\d*\\.\\d*), (-*\\d*\\.\\d*), (-*\\d*\\.\\d*)\\]";
			regformat.assign(regStr,regex::icase);
			if(regex_match(fileStr,result,regformat)){
				tempStr=result[1];
				distMat.at<double>(0,0)=atof(tempStr.c_str());
				tempStr=result[2];
				distMat.at<double>(0,1)=atof(tempStr.c_str());
				tempStr=result[3];
				distMat.at<double>(0,2)=atof(tempStr.c_str());
				tempStr=result[4];
				distMat.at<double>(0,3)=atof(tempStr.c_str());
				tempStr=result[5];
				distMat.at<double>(0,4)=atof(tempStr.c_str());
			}
			break;
		}
	}
}