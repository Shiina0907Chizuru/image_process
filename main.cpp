//TODO:寻找更合理的方法寻中线
//TODO:考虑不采用二值化的方式而直接通过灰度图探测道路线，比如差比和differenceSum(已实现)
#include <iostream>
#include <chrono>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace cv;
using namespace std;

int showDuration(int (*functionPtr)(Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num),Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num);

Mat baseImgGrey(Mat &img);
Mat Compress(Mat &imgGray);
Mat threshold(Mat &imgGray,int thresh);
Mat adaptiveThreshold(Mat &imgGray);
Mat otsuThreshold(Mat &imgGray);
Mat erode(Mat &imgThreshold);
int differenceSum(int pixel1,int pixel2);
int differenceSumThreshold(Mat &imgGray,int diff);
int leftEdgeDetectWithBinary(Mat &img,Point leftEdge[]);
int leftEdgeDetectWithDifferenceSum(Mat &img,Point leftEdge[],int threshold,int period=5);
int midlineDetectWithAve(Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num);
int midlineDetectWithVertical(Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num);
int midlineDetectWithCurveFitting(Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num);
double calculateErrors(Point midline[],int midlineNum);
int main(){
	
	// Mat img=imread("imgs/straight.jpg");//之后做了灰度变换和腐蚀处理但寻中线midlineNum=midlineDetectWithCurveFitting函数只能使用的img才正确，为什么？
	Mat img=imread("imgs/twists.jpg");
	// Mat img=imread("imgs/0.jpg");
	if(img.empty()){
		cout<<"image is empty or the path is invalid"<<endl;
		return 1;
	}
	
	// {
	// 	circle(img,Point{img.cols/2,img.rows/2},1,Scalar(255,0,0),2);
	// }
	
	Mat imgGray=baseImgGrey(img);
	Mat imgThreshold=otsuThreshold(imgGray);
	// Mat imgThreshold=threshold(imgGray,130);
	Mat imgErode=erode(imgThreshold);
	Point leftEdge[img.rows*2];
	// int leftEdgeNum=leftEdgeDetectWithBinary(imgErode,leftEdge);
	int leftEdgeNum=leftEdgeDetectWithDifferenceSum(imgErode,leftEdge,differenceSumThreshold(imgGray,200),2);//在这里传入imgGray却不行，为什么？
	
	{
		cout<<"leftEdgeNum:"<<leftEdgeNum<<endl;
	}
	
	for(int i=0;i<leftEdgeNum;i++){
		circle(img,leftEdge[i],1,Scalar(0,0,255),2);
	}//画出左边界点

	Point midline[leftEdgeNum];


	int midlineNum=midlineDetectWithCurveFitting(imgErode,leftEdge,leftEdgeNum,midline,10);
	// int midlineNum=midlineDetectWithAve(imgErode,leftEdge,leftEdgeNum,midline,10);
	// int midlineNum=midlineDetectWithVertical(imgErode,leftEdge,leftEdgeNum,midline,10);

	
	// int midlineNum=showDuration(midlineDetectWithAve,imgErode,leftEdge,leftEdgeNum,midline,1);
	
	{
		cout<<"midlineNum:"<<midlineNum<<endl;
	}
	for(int i=0;i<midlineNum;i++){
		circle(img,midline[i],1,Scalar(0,255,0),2);
	}//画出中线点
	
	double error=calculateErrors(midline,midlineNum);
	{
		cout<<"error:"<<error<<endl;
	}


	

	imshow("img0",imgErode);
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
int showDuration(int (*functionPtr)(Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num),Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num){
	auto startTime = chrono::system_clock::now();
	int ret=0;
	// cout<<"startTime:"<<chrono::system_clock::to_time_t(startTime)<<endl;
	ret=(*functionPtr)(img,leftEdge,leftEdgeNum,midline,num);
	auto endTime = chrono::system_clock::now();
	auto duration = chrono::duration_cast<chrono::microseconds>(endTime - startTime);
	// cout<<"endTime:"<<chrono::system_clock::to_time_t(chrono::system_clock::now())<<endl;
	cout<<"duration:"<<duration.count()*chrono::microseconds::period::num/chrono::microseconds::period::den<<"s"<<endl;
	return ret;
}

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
//二值化寻左线
int leftEdgeDetectWithBinary(Mat &img,Point leftEdge[]){
	int leftEdgeNum=0;
	int direction=0;//标记方向，避免搜索时重复，1，2，3，4，5，6，7，8分别为1右上，2正上，3左上，4正左，5左下，6正下，7右下，8正右，即从右上角逆时针方向
	Point currentPoint;//当前点，x为列，y为行，注意和img.at<>中的顺序不同
	//找到第一个左边界点
	for(int i=img.cols/2;i>0;i--){
		if(img.at<uchar>(img.rows-2,i)==0){//img.at<uchar>(,)，第一个参数是行，第二个参数是列
			leftEdge[leftEdgeNum].x=i;
			leftEdge[leftEdgeNum].y=img.rows-2;
			leftEdgeNum++;
			currentPoint.x=i;
			currentPoint.y=img.rows-2;
			break;
		}
	}
	//从第一个左边界点开始，八邻域边缘跟踪寻找左边界
	while(currentPoint.x>0&&currentPoint.y>0){
		if(img.at<uchar>(currentPoint.y-1,currentPoint.x+1)==0&&direction!=4&&direction!=5&&direction!=6){//右上角
			direction=1;
			leftEdge[leftEdgeNum].x=currentPoint.x+1;
			leftEdge[leftEdgeNum].y=currentPoint.y-1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x+1;
			currentPoint.y=currentPoint.y-1;
		}else if(img.at<uchar>(currentPoint.y-1,currentPoint.x)==0&&direction!=5&&direction!=6&&direction!=7){//正上方
			direction=2;
			leftEdge[leftEdgeNum].x=currentPoint.x;
			leftEdge[leftEdgeNum].y=currentPoint.y-1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x;
			currentPoint.y=currentPoint.y-1;
		}else if(img.at<uchar>(currentPoint.y-1,currentPoint.x-1)==0&&direction!=6&&direction!=7&&direction!=8){//左上角
			direction=3;
			leftEdge[leftEdgeNum].x=currentPoint.x-1;
			leftEdge[leftEdgeNum].y=currentPoint.y-1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x-1;
			currentPoint.y=currentPoint.y-1;
		}else if(img.at<uchar>(currentPoint.y,currentPoint.x-1)==0&&direction!=1&&direction!=2&&direction!=6&&direction!=7&&direction!=8){//正左方
			direction=4;
			leftEdge[leftEdgeNum].x=currentPoint.x-1;
			leftEdge[leftEdgeNum].y=currentPoint.y;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x-1;
			currentPoint.y=currentPoint.y;
		}else if(img.at<uchar>(currentPoint.y+1,currentPoint.x-1)==0&&direction!=1&&direction!=2&&direction!=8){//左下角
			direction=5;
			leftEdge[leftEdgeNum].x=currentPoint.x-1;
			leftEdge[leftEdgeNum].y=currentPoint.y+1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x-1;
			currentPoint.y=currentPoint.y+1;
		}else if(img.at<uchar>(currentPoint.y+1,currentPoint.x)==0&&direction!=1&&direction!=2&&direction!=3&&direction!=4&&direction!=8){//正下方
			direction=6;
			leftEdge[leftEdgeNum].x=currentPoint.x;
			leftEdge[leftEdgeNum].y=currentPoint.y+1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x;
			currentPoint.y=currentPoint.y+1;
		}else if(img.at<uchar>(currentPoint.y+1,currentPoint.x+1)==0&&direction!=2&&direction!=3&&direction!=4){//右下角
			direction=7;
			leftEdge[leftEdgeNum].x=currentPoint.x+1;
			leftEdge[leftEdgeNum].y=currentPoint.y+1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x+1;
			currentPoint.y=currentPoint.y+1;
		}else if(img.at<uchar>(currentPoint.y,currentPoint.x+1)==0&&direction!=2&&direction!=3&&direction!=4&&direction!=5&&direction!=6){//正右方
			direction=8;
			leftEdge[leftEdgeNum].x=currentPoint.x+1;
			leftEdge[leftEdgeNum].y=currentPoint.y;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x+1;
			currentPoint.y=currentPoint.y;
		}else{
			break;
		}

		for(int i=currentPoint.x+1;img.at<uchar>(currentPoint.y,i)==0;i++){
			leftEdge[leftEdgeNum].x=i;//找到最左线右边的像素作为左线的点
		}
	}
	
	return leftEdgeNum;
}
//差比和算法
int differenceSum(int pixel1,int pixel2){
	if(pixel1+pixel2==0){
		return 0;
	}//防止除数为0
	return (pixel1-pixel2)>=0?100*(pixel1-pixel2)/(pixel1+pixel2):-100*(pixel1-pixel2)/(pixel1+pixel2);
}
//间隔五个点，差比和得到分辨黑白点合适的阈值，diff为黑白点灰度值之差
int differenceSumThreshold(Mat &imgGray,int diff){
	int threshold=0;
	for(int i=imgGray.rows;i>5;i--){
		for(int j=0;j<imgGray.cols;j++){
			if((imgGray.at<uchar>(i,j)-imgGray.at<uchar>(i-5,j))>=diff){//找到黑白点
				threshold=differenceSum(imgGray.at<uchar>(i,j),imgGray.at<uchar>(i-5,j))*0.7;//阈值为黑白点的差比和的0.7倍
				{
					cout<<"differenceSumThreshold:"<<threshold<<endl;
				}
				return threshold;
			}
			
		}
	}
	return threshold;
}
//差比和寻左线
int leftEdgeDetectWithDifferenceSum(Mat &imgGray,Point leftEdge[],int threshold,int period){
	int leftEdgeNum=0;
	//八邻域边缘跟踪寻找左边界，类似于leftEdgeDetectWithBinary函数中的实现，区别是判断方法不同
	int direction=0;//标记方向，避免搜索时重复，1，2，3，4，5，6，7，8分别为1右上，2正上，3左上，4正左，5左下，6正下，7右下，8正右，即从右上角逆时针方向
	Point currentPoint;//当前点
	//找到第一个左边界点
	for(int i=imgGray.cols/2;i>period;i-=period){
		if(differenceSum(imgGray.at<uchar>(imgGray.rows-2,i),imgGray.at<uchar>(imgGray.rows-2,i-period))>=threshold){//大于阈值即为边界点
			leftEdge[leftEdgeNum].x=i-period;
			leftEdge[leftEdgeNum].y=imgGray.rows-2;
			leftEdgeNum++;
			currentPoint.x=i-period;
			currentPoint.y=imgGray.rows-2;
			break;
		}
	}
	//开始边缘跟踪
	
	while(currentPoint.x>0&&currentPoint.y>0&&currentPoint.x<imgGray.cols&&currentPoint.y<imgGray.rows){
		if(differenceSum(imgGray.at<uchar>(currentPoint.y,currentPoint.x),imgGray.at<uchar>(currentPoint.y-1,currentPoint.x+1))<=threshold&&direction!=4&&direction!=5&&direction!=6){//右上角
			direction=1;
			leftEdge[leftEdgeNum].x=currentPoint.x+1;
			leftEdge[leftEdgeNum].y=currentPoint.y-1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x+1;
			currentPoint.y=currentPoint.y-1;
		}else if(differenceSum(imgGray.at<uchar>(currentPoint.y,currentPoint.x),imgGray.at<uchar>(currentPoint.y-1,currentPoint.x))<=threshold&&direction!=5&&direction!=6&&direction!=7){//正上方
			direction=2;
			leftEdge[leftEdgeNum].x=currentPoint.x;
			leftEdge[leftEdgeNum].y=currentPoint.y-1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x;
			currentPoint.y=currentPoint.y-1;
		}else if(differenceSum(imgGray.at<uchar>(currentPoint.y,currentPoint.x),imgGray.at<uchar>(currentPoint.y-1,currentPoint.x-1))<=threshold&&direction!=6&&direction!=7&&direction!=8){//左上角
			direction=3;
			leftEdge[leftEdgeNum].x=currentPoint.x-1;
			leftEdge[leftEdgeNum].y=currentPoint.y-1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x-1;
			currentPoint.y=currentPoint.y-1;
		}else if(differenceSum(imgGray.at<uchar>(currentPoint.y,currentPoint.x),imgGray.at<uchar>(currentPoint.y,currentPoint.x-1))<=threshold&&direction!=1&&direction!=2&&direction!=6&&direction!=7&&direction!=8){//正左方
			direction=4;
			leftEdge[leftEdgeNum].x=currentPoint.x-1;
			leftEdge[leftEdgeNum].y=currentPoint.y;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x-1;
			currentPoint.y=currentPoint.y;
		}else if(differenceSum(imgGray.at<uchar>(currentPoint.y,currentPoint.x),imgGray.at<uchar>(currentPoint.y+1,currentPoint.x-1))<=threshold&&direction!=1&&direction!=2&&direction!=8){//左下角
			direction=5;
			leftEdge[leftEdgeNum].x=currentPoint.x-1;
			leftEdge[leftEdgeNum].y=currentPoint.y+1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x-1;
			currentPoint.y=currentPoint.y+1;
		}else if(differenceSum(imgGray.at<uchar>(currentPoint.y,currentPoint.x),imgGray.at<uchar>(currentPoint.y+1,currentPoint.x))<=threshold&&direction!=1&&direction!=2&&direction!=3&&direction!=4&&direction!=8){//正下方
			direction=6;
			leftEdge[leftEdgeNum].x=currentPoint.x;
			leftEdge[leftEdgeNum].y=currentPoint.y+1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x;
			currentPoint.y=currentPoint.y+1;
		}else if(differenceSum(imgGray.at<uchar>(currentPoint.y,currentPoint.x),imgGray.at<uchar>(currentPoint.y+1,currentPoint.x+1))<=threshold&&direction!=2&&direction!=3&&direction!=4){//右下角
			direction=7;
			leftEdge[leftEdgeNum].x=currentPoint.x+1;
			leftEdge[leftEdgeNum].y=currentPoint.y+1;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x+1;
			currentPoint.y=currentPoint.y+1;
		}else if(differenceSum(imgGray.at<uchar>(currentPoint.y,currentPoint.x),imgGray.at<uchar>(currentPoint.y,currentPoint.x+1))<=threshold&&direction!=2&&direction!=3&&direction!=4&&direction!=5&&direction!=6){//正右方
			direction=8;
			leftEdge[leftEdgeNum].x=currentPoint.x+1;
			leftEdge[leftEdgeNum].y=currentPoint.y;
			leftEdgeNum++;
			currentPoint.x=currentPoint.x+1;
			currentPoint.y=currentPoint.y;
		}else{
			break;
		}

		for(int i=currentPoint.x+1;differenceSum(imgGray.at<uchar>(currentPoint.y,currentPoint.x),imgGray.at<uchar>(currentPoint.y,i))<=threshold;i++){
			leftEdge[leftEdgeNum].x=i;//找到最左线右边的像素作为左线的点
		}
	}
	return leftEdgeNum;
}
//直接横向找右线点另一点，取中点做中线上的点，直道效果好，弯道效果有偏差，间隔num个像素找一个点以减少计算量
int midlineDetectWithAve(Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num){
	int midlineNum=0;
	for(int i=0;i<leftEdgeNum;i+=num){
		for(int j=leftEdge[i].x+5;j<img.cols;j++){
			if(img.at<uchar>(leftEdge[i].y,j)==0){
				midline[midlineNum].x=(leftEdge[i].x+j)/2;
				midline[midlineNum].y=leftEdge[i].y;
				midlineNum++;
				break;
			}
		}
	}
	return midlineNum;
}
//废弃，间隔num找两个点做垂线交右线于另一点，取中点做中线上的点，效果非常不好，不能使用，应采用拟合方式midlineDetectWithCurveFitting，即，一方面因为垂线可能和左线自身相交，另一方面因为像素离散，垂线和右线相交会偏向一侧
int midlineDetectWithVertical(Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num){//求左线的两点的垂线，求垂线与右线的交点，求交点的中点，作为中线的点
	int midlineNum=0;
	double k;//左线两点垂线方程的斜率
	double c;//左线两点垂线方程的常数项
	for(int i=0;i<leftEdgeNum-num;i+=num){
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
			if(y0>=img.rows-1||x0>=img.cols-1){
				continue;//垂线与右线的交点不在图像内，跳过
			}
			if(img.at<uchar>(y0,x0)==0&&(x0-leftEdge[i].x>=20)){//按垂线找到与右线的交点，交点坐标为(leftEdge[i].x+j,k*(leftEdge[i].x)+c)
				midline[midlineNum].x=(leftEdge[i].x+x0)/2;
				midline[midlineNum].y=(leftEdge[i].y+y0)/2;
				midlineNum++;
				break;
			}
		}
	}
	return midlineNum;
}

//TODO:确定垂线的方向问题
//取左线num个点拟合直线再做垂线交右线于另一点，取中点做中线的点，有误差待处理，仍偏向一侧，弯道效果非常不好，原因猜测是最小二分拟合不适用于曲线，考虑使用其他方法进行拟合（如霍夫变换等）
int midlineDetectWithCurveFitting(Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num){//求左线num个点的拟合直线的垂线，求垂线与右线的交点，求交点的中点，作为中线的点
	int midlineNum=0;
	//y=kx+c
	double k=0;//左线拟合直线的垂线方程的斜率
	double c=0;//左线拟合直线的垂线方程的常数项
	int m1=0,m2=0,m3=0,m4=0,m5=0;//最小二乘拟合直线所需参数

	for(int i=0;i<leftEdgeNum-num;i+=num){
		//将参数初始化为0
		m1=0;
		m2=0;
		m3=0;
		m4=0;
		m5=0;
		//求参数
		for(int d=0;d<num;d++){
			m1+=leftEdge[i+d].x*leftEdge[i+d].y;//x*y的和
			m2+=leftEdge[i+d].x;//x的和
			m3+=leftEdge[i+d].y;//y的和
			m4+=leftEdge[i+d].x*leftEdge[i+d].x;//x平方的和
		}
		m5=m2*m2;//x和的平方
		if((num*m1-m2*m3)==0){
			continue;//垂线不存在，跳过
		}
		k=-(double)(num*m4-m5)/(double)(num*m1-m2*m3);//拟合直线的垂线的斜率
		c=(m3-k*m2)/num;//拟合直线的曲线的垂线的常数项
		
		for(int j=leftEdge[i].x+1;j<img.cols;j++){
			int x0=j;
			int y0=k*x0+c;
			if(y0>=img.rows-1||x0>=img.cols-1){
				continue;//垂线与右线的交点不在图像内，跳过
			}
			
			if(img.at<uchar>(y0,x0)==0&&(x0-leftEdge[i].x>=20)){//按垂线找到与右线的交点，交点坐标为(j,k*(leftEdge[i].x)+c)
				midline[midlineNum].x=(leftEdge[i].x+x0)/2;
				midline[midlineNum].y=(leftEdge[i].y+y0)/2;
				midlineNum++;
				break;
			}
		}
	}
	return midlineNum;
}
//errors为正即右偏，为负即左偏
double calculateErrors(Point midline[],int midlineNum){
	int errorsSum=0;
	double errors=0;
	Point basePoint=midline[0];//基准点
	for(int i=1;i<midlineNum;i++){
		errorsSum+=midline[i].x-basePoint.x;//误差为中线点的x坐标与基准点的x坐标的差的和
	}
	errors=(double)errorsSum/(double)(midlineNum-1);//平均误差
	return errors;
}
