#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace cv;
using namespace std;

//读入图片并转换为灰度图
Mat baseImgGrey(Mat &img){
	Mat imgGray=Mat::zeros(img.size(),img.type());
	if(img.empty()){
		cout<<"Error: Image cannot be loaded"<<endl;
		return Mat();
	}
	cvtColor(img,imgGray,COLOR_BGR2GRAY);
	return imgGray;
}
//压缩图片
Mat Compress(Mat &imgGray){
	Mat imgCompress=Mat::zeros(imgGray.rows/2,imgGray.cols/2,imgGray.type());
	for(int i=0;i<imgGray.rows;i+=2){
		for(int j=0;j<imgGray.cols;j+=2){
			imgCompress.at<uchar>(i/2,j/2)=(imgGray.at<uchar>(i,j)+imgGray.at<uchar>(i+1,j)+imgGray.at<uchar>(i,j+1)+imgGray.at<uchar>(i+1,j+1))/4;//取2x2像素块四个点的平均值
		}
	}
	return imgCompress;
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

//大津法阈值分割
Mat otsuThreshold(Mat &imgGray){
	Mat imgThreshold=Mat::zeros(imgGray.size(),imgGray.type());
	int hist[256]={0};//存储灰度直方图
	for(int i=0;i<imgGray.rows;i++){
		for(int j=0;j<imgGray.cols;j++){
			hist[imgGray.at<uchar>(i,j)]++;
		}
	}//计算灰度直方图
	int total=imgGray.rows*imgGray.cols;//总像素点
	float sum=0;//总灰度值
	for(int i=0;i<256;i++){
		sum+=i*hist[i];
	}//计算总灰度值
	float sumB=0;//前景灰度值
	int wB=0;//前景像素点数
	int wF=0;//背景像素点数
	float varMax=0;//最大类间方差
	int threshold=0;//最大类间方差对应的阈值
	
	//前景是灰度值等于i的像素点，背景是灰度值不等于i的像素点
	//遍历每个灰度值，比较前景背景类间方差，取最大的类间方差对应的灰度值为阈值
	for(int i=0;i<256;i++){
		wB+=hist[i];//前景像素点数
		if(wB==0){
			continue;
		}
		wF=total-wB;//背景像素点数
		if(wF==0){
			break;
		}
		sumB+=i*hist[i];//前景灰度值
		float mB=sumB/wB;//前景灰度均值
		float mF=(sum-sumB)/wF;//背景灰度均值
		float varBetween=wB*wF*(mB-mF)*(mB-mF);//前景背景类间方差
		if(varBetween>varMax){
			varMax=varBetween;
			threshold=i;
		}//更新最大类间方差
	}
	for(int i=0;i<imgGray.rows;i++){
		for(int j=0;j<imgGray.cols;j++){
			if(imgGray.at<uchar>(i,j)>threshold){
				imgThreshold.at<uchar>(i,j)=255;
			}else{
				imgThreshold.at<uchar>(i,j)=0;
			}
		}
	}//二值化
	return imgThreshold;
}
//腐蚀
Mat erode(Mat &imgThreshold){
	Mat imgErode=Mat::zeros(imgThreshold.size(),imgThreshold.type());
	for(int i=1;i<imgThreshold.rows;i++){
		for(int j=1;j<imgThreshold.cols;j++){
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

int leftEdgeDetect(Mat &img,Point leftEdge[]){
	int leftEdgeNum=0;
	Point currentPoint;//当前点，x为列，y为行，注意和img.at<>中的顺序不同
	//找到第一个左边界点
	for(int i=1;i<img.cols/2;i++){
		if(img.at<uchar>(img.rows-2,img.cols/2-i)==0){//img.at<uchar>(,)，第一个参数是行，第二个参数是列
			leftEdge[leftEdgeNum].x=img.cols/2-i;
			leftEdge[leftEdgeNum].y=img.rows-2;
			leftEdgeNum++;
			currentPoint.x=img.cols/2-i;
			currentPoint.y=img.rows-2;
			break;
		}
	}
	//从第一个左边界点开始，向上寻找左边界点
	while(currentPoint.x>0&&currentPoint.y>0){
		if(img.at<uchar>(currentPoint.y-1,currentPoint.x-1)==0){//左上角
			leftEdge[leftEdgeNum].x=currentPoint.x-1;
			leftEdge[leftEdgeNum].y=currentPoint.y-1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x-1;
			currentPoint.y=currentPoint.y-1;
		}else if(img.at<uchar>(currentPoint.y-1,currentPoint.x)==0){//正上方
			leftEdge[leftEdgeNum].x=currentPoint.x;
			leftEdge[leftEdgeNum].y=currentPoint.y-1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x;
			currentPoint.y=currentPoint.y-1;
		}else if(img.at<uchar>(currentPoint.y-1,currentPoint.x+1)==0){//右上角
			leftEdge[leftEdgeNum].x=currentPoint.x+1;
			leftEdge[leftEdgeNum].y=currentPoint.y-1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x+1;
			currentPoint.y=currentPoint.y-1;
		}else{
			break;
		}
	}
	return leftEdgeNum;
}
int midlineDetect(Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[]){//求左线的两点的垂线，求垂线与右线的交点，求交点的中点，作为中线的点
	int midlineNum=0;
	double k;//左线两点垂线方程的斜率
	double c;//左线两点垂线方程的常数项
	for(int i=0;i<leftEdgeNum;i++){
		//y-y0=k(x-x0)
		//y=kx+y0-kx0
		//c=y0-kx0
		if(leftEdge[i].y==leftEdge[i+1].y){
			continue;//左线两点的y坐标相等，垂线不存在，跳过
		}
		k=-(double)(leftEdge[i].x-leftEdge[i+1].x)/(double)(leftEdge[i].y-leftEdge[i+1].y);
		c=leftEdge[i].y-k*leftEdge[i].x;
		for(int j=1;j<img.cols-leftEdge[i].x;j++){
			int x0=leftEdge[i].x+j;
			int y0=k*x0+c;
			if(y0>img.rows-1||x0>img.cols-1){
				continue;//垂线与右线的交点不在图像内，跳过
			}
			if(img.at<uchar>(x0,y0)==0){//按垂线找到与右线的交点，交点坐标为(leftEdge[i].x+j,k*(leftEdge[i].x)+c)
				midline[midlineNum].x=(leftEdge[i].x+x0)/2;
				midline[midlineNum].y=(leftEdge[i].y+y0)/2;
				midlineNum++;
				break;
			}
		}
	}

	return midlineNum;
}
int main(){

	Mat img=imread("imgs/straight.jpg");
	Mat imgGray=baseImgGrey(img);
	Mat imgThreshold=otsuThreshold(imgGray);
	// Mat imgThreshold=threshold(imgGray,130);
	Mat imgErode=erode(imgThreshold);

	Point leftEdge[img.rows];
	int leftEdgeNum=leftEdgeDetect(imgErode,leftEdge);
	cout<<"leftEdgeNum:"<<leftEdgeNum<<endl;
	
	circle(img,Point(img.cols/2,img.rows/2),1,Scalar(0,0,255),2);
	for(int i=0;i<leftEdgeNum;i++){
		circle(img,leftEdge[i],1,Scalar(0,0,255),2);
	}//画出左边界点

	Point midline[leftEdgeNum];
	int midlineNum=midlineDetect(imgErode,leftEdge,leftEdgeNum,midline);
	cout<<"midlineNum:"<<midlineNum<<endl;
	for(int i=0;i<midlineNum;i++){
		circle(img,midline[i],1,Scalar(0,255,0),2);
	}//画出中线点
	imshow("img",imgErode);
	waitKey(0);
	imshow("imgThreshold",img);
	waitKey(0);	
	

	// Mat img=imread("imgs/straight.jpg");
	// imshow("Image0",img);
	// waitKey(0);

	// Mat imgGray=baseImgGrey(img);
	// Mat imgThreshold=threshold(imgGray,130);
	// Mat imgErode=erode(imgThreshold);
	// imshow("Image1",imgErode);
	// waitKey(0);

	// Mat imgCompress=Compress(imgGray);
	// Mat imgThreshold2=threshold(imgCompress,130);
	// Mat imgErode2=erode(imgThreshold2);

	// imshow("Image2",imgThreshold2);
	// waitKey(0);

	// Mat otsuImg=otsuThreshold(imgGray);
	// Mat otsuImgErode=erode(otsuImg);
	// imshow("Image3",otsuImg);
	// waitKey(0);

	// VideoCapture cap;
	// cap.open(0);
	// Mat frame;
	// Mat frameCompress;
	// Mat frameGray;
	// Mat frameErode;
	// Mat frameThreshold;
	// while (1){
	// 	cap>>frame;
	// 	frameCompress=Compress(frame);
	// 	frameGray=baseImgGrey(frameCompress);
	// 	frameErode=erode(frameGray);
	// 	frameThreshold=threshold(frameErode,10);
	// 	imshow("frame",frameThreshold);
	// 	if(waitKey(30)>=0) {
	// 		break;
	// 	}
	// }
	// cap.release();
	


	destroyAllWindows();
	return 0;
}