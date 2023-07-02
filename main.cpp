#include <iostream>
#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

//读入图片并转换为灰度图
Mat baseImgGrey(){
	Mat img,imgGray;
	img=imread("0.jpg");
	if(img.empty()){
		cout<<"Error: Image cannot be loaded"<<endl;
		return Mat();
	}
	cvtColor(img,imgGray,COLOR_BGR2GRAY);
	return imgGray;
}
//固定阈值分割
Mat threshold(Mat &imgGray,int thresh){
	Mat imgThreshold=Mat::zeros(imgGray.size(),imgGray.type());
	for(int i=0;i<imgGray.rows;i++){
		for(int j=0;j<imgGray.cols;j++){
			if(imgGray.at<uchar>(i,j)>thresh){
				imgThreshold.at<uchar>(i,j)=255;
			}else{
				imgThreshold.at<uchar>(i,j)=0;
			}
		}
	}
	return imgThreshold;
}
//自适应阈值分割
Mat adaptiveThreshold(Mat &imgGray){
	Mat imgThreshold=Mat::zeros(imgGray.size(),imgGray.type());
	int blockSize=5;
	int C=-10;
	for(int i=0;i<imgGray.rows;i++){
		for(int j=0;j<imgGray.cols;j++){
			int sum=0;
			for(int m=i-blockSize/2;m<i+blockSize/2;m++){
				for(int n=j-blockSize/2;n<j+blockSize/2;n++){
					if(m>=0&&m<imgGray.rows&&n>=0&&n<imgGray.cols){
						sum+=imgGray.at<uchar>(m,n);
					}
				}
			}
			if(imgGray.at<uchar>(i,j)>(sum/(blockSize*blockSize)-C)){
				imgThreshold.at<uchar>(i,j)=255;
			}else{
				imgThreshold.at<uchar>(i,j)=0;
			}
		}
	}
	return imgThreshold;
}
//腐蚀
Mat erode(Mat imgThreshold){
	Mat imgErode=Mat::zeros(imgThreshold.size(),imgThreshold.type());
	for(int i=0;i<imgThreshold.rows;i++){
		for(int j=0;j<imgThreshold.cols;j++){
			if(imgThreshold.at<uchar>(i,j)==255){
				if(imgThreshold.at<uchar>(i-1,j)==0||imgThreshold.at<uchar>(i+1,j)==0||imgThreshold.at<uchar>(i,j-1)==0||imgThreshold.at<uchar>(i,j+1)==0){
					imgErode.at<uchar>(i,j)=0;
				}else{
					imgErode.at<uchar>(i,j)=255;
				}
			}else{
				imgErode.at<uchar>(i,j)=0;
			}
		}
	}
	return imgErode;
}
int main(){

	Mat imgGray=baseImgGrey();
	Mat imgThreshold=adaptiveThreshold(imgGray);
	// Mat imgErode=erode(imgThreshold);
	imshow("Image",imgThreshold);
	waitKey(0);



	return 0;
}