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

//�������£����ϣ����ϣ���������
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
	
	ifstream fin(filePath,ios_base::in); /* �궨����ͼ���ļ���·�� */  
	if(!fin.good()){
		cout<<"ChessPhotoPath.txt�ļ�������"<<endl;
		return;
	}

	sprintf(filePath,"%s/CaliberationReport.txt",chessPhotoPath);
    ofstream fout(filePath,ios_base::out);  /* ����궨������ļ� */    
    //��ȡÿһ��ͼ�񣬴�����ȡ���ǵ㣬Ȼ��Խǵ���������ؾ�ȷ��   
    cout<<"��ʼ��ȡ�ǵ㡭����������\n";  
    Size image_size;  /* ͼ��ĳߴ� */  

    vector<Point2f> image_points_buf;  /* ����ÿ��ͼ���ϼ�⵽�Ľǵ� */  
    vector<vector<Point2f>> image_points_seq; /* �����⵽�����нǵ� */  
    string filename;  
	int imageNum=0;/* ͼ������ */ 
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

        if (imageNum == 1)  //�����һ��ͼƬʱ��ȡͼ������Ϣ  
        {  
            image_size.width = src.cols;  
            image_size.height =src.rows;           
        }  
  
        /* ��ȡ�ǵ� */  
        if (0 == findChessboardCorners(src,angularPt_size,image_points_buf))  
        {             
            cout<<"can not find chessboard corners!\n"; //�Ҳ����ǵ�  
            exit(1);  
        }   
        else   
        {  
            
            cvtColor(src,gray,CV_RGB2GRAY);  
            /* �����ؾ�ȷ�� */  
            find4QuadCornerSubpix(gray,image_points_buf,Size(5,5)); //�Դ���ȡ�Ľǵ���о�ȷ��  
            //cornerSubPix(gray,image_points_buf,Size(5,5),Size(-1,-1),TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER,30,0.1));  
            image_points_seq.push_back(image_points_buf);  //���������ؽǵ�  
            /* ��ͼ������ʾ�ǵ�λ�� */  
            drawChessboardCorners(gray,angularPt_size,image_points_buf,true); //������ͼƬ�б�ǽǵ�  
            
			imshow("Camera Calibration",gray);//��ʾͼƬ  

            waitKey(10);//��ͣ0.01S         
        }  
    }   

    int CornerNum=angularPt_size.width*angularPt_size.height;  //ÿ��ͼƬ���ܵĽǵ���  

  
    //������������궨  

    //������ά��Ϣ
    vector<vector<Point3f>> pointsCoordinate; /* ����궨���Ͻǵ����ά���� */  

    //������� 
    vector<int> point_counts;  // ÿ��ͼ���нǵ������  
    vector<Mat> rotMat;  /* ÿ��ͼ�����ת���� */  
    vector<Mat> traMat; /* ÿ��ͼ���ƽ������ */  
    //��ʼ���궨���Ͻǵ����ά����
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
                /* ����궨�������������ϵ��z=0��ƽ���� */  
                realPoint.x = i*square_size.width;  
                realPoint.y = j*square_size.height;  
                realPoint.z = 0;  
                tempPointsCoordinate.push_back(realPoint);  
            }  
        }  
        pointsCoordinate.push_back(tempPointsCoordinate);  
    }  
    /* ��ʼ��ÿ��ͼ���еĽǵ��������ٶ�ÿ��ͼ���ж����Կ��������ı궨�� */  
    for (i=0;i<imageNum;i++)  
    {  
        point_counts.push_back(angularPt_size.width*angularPt_size.height);  
    }     

    /* ��ʼ�궨 */  
    calibrateCamera(pointsCoordinate,image_points_seq,image_size,cameraMat,distMat,rotMat,traMat,0);  
    cout<<"�궨��ɣ�\n";  
    //�Ա궨�����������  
    cout<<"��ʼ���۱궨���������������\n";  
    double total_err = 0.0; /* ����ͼ���ƽ�������ܺ� */  
    double err = 0.0; /* ÿ��ͼ���ƽ����� */  
    vector<Point2f> image_points_new; /* �������¼���õ���ͶӰ�� */  
	vector<Point2f> image_points_old; /* ����ԭͼ�е�ͶӰ�� */ 

    cout<<"\tÿ��ͼ��ı궨��\n";  
    fout<<"ÿ��ͼ��ı궨��\n";  
    for (i=0;i<imageNum;i++)  
    {  
        tempPointsCoordinate=pointsCoordinate[i];  
        /* ͨ���õ������������������Կռ����ά���������ͶӰ���㣬�õ��µ�ͶӰ�� */  
        projectPoints(tempPointsCoordinate,rotMat[i],traMat[i],cameraMat,distMat,image_points_new);  
        /* �����µ�ͶӰ��;ɵ�ͶӰ��֮������*/  
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
        std::cout<<"��"<<i+1<<"��ͼ���ƽ����"<<err<<"����"<<endl;     
        fout<<"��"<<i+1<<"��ͼ���ƽ����"<<err<<"����"<<endl;     
    }     
    std::cout<<"����ƽ����"<<total_err/imageNum<<"����"<<endl;     
    fout<<"����ƽ����"<<total_err/imageNum<<"����"<<endl<<endl;     
    std::cout<<"������ɣ�"<<endl;   

    //���涨����      
    std::cout<<"��ʼ���涨����������������"<<endl;         
    Mat rotation_matrix = Mat(3,3,CV_32FC1, Scalar::all(0)); /* ����ÿ��ͼ�����ת���� */ 
	fout<<"ͼƬ�ߴ磺"<<endl;
	fout<<"row="<<image_size.height<<endl;
	fout<<"width="<<image_size.width<<endl;
    fout<<"����ڲ�������"<<endl;     
    fout<<cameraMat<<endl<<endl;     
    fout<<"����ϵ����\n";     
    fout<<distMat<<endl<<endl<<endl;     
    for (int i=0; i<imageNum; i++)   
    {   
        fout<<"��"<<i+1<<"��ͼ�����ת������"<<endl;     
        fout<<rotMat[i]<<endl;      
        /* ����ת����ת��Ϊ���Ӧ����ת���� */     
        Rodrigues(rotMat[i],rotation_matrix);     
        fout<<"��"<<i+1<<"��ͼ�����ת����"<<endl;     
        fout<<rotation_matrix<<endl;     
        fout<<"��"<<i+1<<"��ͼ���ƽ��������"<<endl;     
        fout<<traMat[i]<<endl<<endl;     
    }     
    std::cout<<"��ɱ���"<<endl;   
    fout<<endl;  
    ///************************************************************************   
    //��ʾ������   
    //*************************************************************************/  

    Mat mapx = Mat(image_size,CV_32FC1);  
    Mat mapy = Mat(image_size,CV_32FC1);  
    Mat R = Mat::eye(3,3,CV_32F);  
	//���ӳ�����
	initUndistortRectifyMap(cameraMat,distMat,R,cameraMat,image_size,CV_32FC1,mapx,mapy);
	//fout<<"mapx="<<endl;
	//fout<<mapx<<endl<<endl;
	//fout<<"mapy="<<endl;
	//fout<<mapy<<endl<<endl;

    std::cout<<"�������ͼ��"<<endl;  

	fin.clear(ios::goodbit);
	fin.seekg(0,ios::beg);
      
	int pos;
	Mat newSrc;
    while(getline(fin,filename))
    {  

        src = imread(filename);  
        newSrc = src.clone();  
        //��һ�ֲ���Ҫת������ķ�ʽ  
        //undistort(imageSource,newimage,cameraMatrix,distCoeffs);  
        remap(src,newSrc,mapx, mapy, INTER_LINEAR);         
		pos=filename.find('.');
		filename.insert(pos,"_t");
        imwrite(filename,newSrc);  
    }  
    std::cout<<"�������"<<endl;  
	fin.close();
	fout.close();

	sprintf(filePath,"%s/CalibrationResult.txt",chessPhotoPath);
	fout.open(filePath,ios_base::trunc);
	fout<<"����ڲ�������"<<endl;
	fout<<cameraMat<<endl;
	fout<<"����ϵ����"<<endl;
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
	//��ȡ�궨����

	string tempStr;
	
	string regStr;
	regex regformat;
	smatch result;
	while(getline(fin,fileStr)){

		if(fileStr=="����ڲ�������"){
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

		if(fileStr=="����ϵ����"){
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