#include "stdafx.h"
void takePhoto();
void calibration();
void getDistance();
int main(int *argc,char** argv){
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
	ifstream fin("F:/TestData/ChessFile.txt",ios_base::in); /* �궨����ͼ���ļ���·�� */  
    ofstream fout("F:/TestData/CaliberationResult.txt",ios_base::out);  /* ����궨������ļ� */    
    //��ȡÿһ��ͼ�񣬴�����ȡ���ǵ㣬Ȼ��Խǵ���������ؾ�ȷ��   
    cout<<"��ʼ��ȡ�ǵ㡭����������\n";  
    Size image_size;  /* ͼ��ĳߴ� */  
    Size board_size = Size(6,8);    /* �궨����ÿ�С��еĽǵ��� */  
    vector<Point2f> image_points_buf;  /* ����ÿ��ͼ���ϼ�⵽�Ľǵ� */  
    vector<vector<Point2f>> image_points_seq; /* �����⵽�����нǵ� */  
    string filename;  
    int count= -1 ;//���ڴ洢�ǵ������  
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
            //cout<<"image_size.width = "<<image_size.width<<endl;  
            //cout<<"image_size.height = "<<image_size.height<<endl;  
        }  
  
        /* ��ȡ�ǵ� */  
        if (0 == findChessboardCorners(src,board_size,image_points_buf))  
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
            drawChessboardCorners(gray,board_size,image_points_buf,true); //������ͼƬ�б�ǽǵ�  
			
			//circle(gray,Point(image_points_buf.at(0).x,image_points_buf.at(0).y),5,Scalar(0,0,0));
   //         circle(gray,Point(image_points_buf.at(6).x,image_points_buf.at(6).y),5,Scalar(0,0,0));
            
			imshow("Camera Calibration",gray);//��ʾͼƬ  

            waitKey(10);//��ͣ0.5S         
        }  
    }   

    int CornerNum=board_size.width*board_size.height;  //ÿ��ͼƬ���ܵĽǵ���  

  
    //������������궨  

    //������ά��Ϣ
    Size square_size = Size(40,40);  /* ʵ�ʲ����õ��ı궨����ÿ�����̸�Ĵ�С */  
    vector<vector<Point3f>> pointsCoordinate; /* ����궨���Ͻǵ����ά���� */  

    //������� 
    Mat cameraMat=Mat(3,3,CV_32FC1,Scalar::all(0)); /* ������ڲ������� */  
    vector<int> point_counts;  // ÿ��ͼ���нǵ������  
    Mat distMat=Mat(1,5,CV_32FC1,Scalar::all(0)); /* �������5������ϵ����k1,k2,p1,p2,k3 */  
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
        for (i=0;i<board_size.height;i++)   
        {  
            for (j=0;j<board_size.width;j++)   
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
        point_counts.push_back(board_size.width*board_size.height);  
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

    return ;  
}  

void getDistance(){
	//��ȡ�궨�ļ�
	ifstream fin("F:/TestData/distance/CaliberationResult.txt",ios_base::in);   
	string fileStr;
	//��ȡ�궨����
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

		if(fileStr=="����ڲ�������"){
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

		if(fileStr=="����ϵ����"){
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
	ofstream fout("F:/answer.txt",ios_base::out);
	fout<<cameraMat<<endl;
	fout<<distMat<<endl;
	fout.close();
    Mat mapx = Mat(row,col,CV_32FC1);  
    Mat mapy = Mat(row,col,CV_32FC1);  
    Mat R = Mat::eye(3,3,CV_32F);  
	//���ӳ�����
	initUndistortRectifyMap(cameraMat,distMat,R,cameraMat,Size(col,row),CV_32FC1,mapx,mapy);


	Mat src,newSrc;
	fin.close();
	fin.open("F:/TestData/ChessFile.txt",ios_base::in);
	string filename;
	int pos;
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
}
