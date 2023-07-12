//TODO:取到合理的差比和的阈值
//TODO:寻找更合理的方法寻中线
//TODO:考虑不采用二值化的方式而直接通过灰度图探测道路线，比如差比和differenceSum
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
int differenceSumThreshold(Mat &imgGray,int period,int diff);
int leftEdgeDetectWithBinary(Mat &img,Point leftEdge[]);
int leftEdgeDetectWithDifferenceSum(Mat &img,Point leftEdge[],int threshold,int period=5);
int midlineDetectWithAve(Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num);
int midlineDetectWithVertical(Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num);
int midlineDetectWithCurveFitting(Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num);
double calculateErrors(Point midline[],int midlineNum);
// Mat skeletonizeWithZhangSuen(Mat &img);//Zhang-Suen细化算法
int main(){
	
	// Mat img=imread("imgs/straight.jpg");//之后做了灰度变换和腐蚀处理但寻中线midlineNum=midlineDetectWithCurveFitting函数只能使用的img才正确，为什么？
	// Mat img=imread("imgs/twists.jpg");
	Mat img=imread("imgs/0.jpg");
	// Mat img=imread("imgs/p25.bmp");
	if(img.empty()){
		cout<<"image is empty or the path is invalid"<<endl;
		return 1;
	}
	// {
	// 	circle(img,Point{img.cols/2,img.rows/2},1,Scalar(255,0,0),2);
	// }
	
	Mat imgGray=baseImgGrey(img);
	// Mat imgGray=img.clone();
	Mat imgThreshold=otsuThreshold(imgGray);
	// Mat imgThreshold=threshold(imgGray,130);
	Mat imgErode=erode(imgThreshold);


	Point leftEdge[1000];
	int leftEdgeNum=leftEdgeDetectWithBinary(imgErode,leftEdge);
	// int leftEdgeNum=leftEdgeDetectWithDifferenceSum(imgGray,leftEdge,differenceSumThreshold(imgGray,2,20),2);//在这里传入imgGray却不行，为什么？
	
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

	
	// // int midlineNum=showDuration(midlineDetectWithAve,imgErode,leftEdge,leftEdgeNum,midline,1);
	
	{
		cout<<"midlineNum:"<<midlineNum<<endl;
	}
	for(int i=0;i<midlineNum;i++){
		circle(img,midline[i],1,Scalar(0,255,0),2);
	}//画出中线点
	
	// double error=calculateErrors(midline,midlineNum);
	// {
	// 	cout<<"error:"<<error<<endl;
	// }


	
	imshow("img00",imgErode);
	waitKey(0);
	imshow("img0",imgGray);
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
	const Point directions[]={Point(1,0),Point(1,-1),Point(0,-1),Point(-1,-1),Point(-1,0),Point(-1,1),Point(0,1),Point(1,1)};//八邻域，逆时针，从正右开始
	int currentDirection=0;//当前方向
	bool stopFlag=false;//停止标志
	int counts=0;//计数器
	Point rootPoint;//根点
	Point currentPoint;//当前点，x为列，y为行，注意和img.at<>中的顺序不同
	//找到第一个左边界点
	for(int i=img.cols/2;i>0;i--){
		if(img.at<uchar>(img.rows-2,i)==0){//img.at<uchar>(,)，第一个参数是行，第二个参数是列
			leftEdge[leftEdgeNum].x=i;
			leftEdge[leftEdgeNum].y=img.rows-2;
			leftEdgeNum++;
			rootPoint.x=i;
			rootPoint.y=img.rows-2;
			{
				cout<<"rootPoint:"<<rootPoint.x<<","<<rootPoint.y<<endl;
			}

			break;
		}
	}//找到第一个左边界点
	while(!stopFlag&&leftEdgeNum<1000){
		for(counts=0;counts<8;counts++){//循环八次进行跟踪
			if(currentDirection>=8){
				currentDirection-=8;
			}
			if(currentDirection<0){
				currentDirection+=8;
			}//防止数组越界
			currentPoint=Point{rootPoint.x+directions[currentDirection].x,rootPoint.y+directions[currentDirection].y};//跟踪
			cout<<"currentPoint:"<<currentPoint.x<<","<<currentPoint.y<<endl;
			if(currentPoint.x>0&&currentPoint.y>0&&currentPoint.x<img.cols-1&&currentPoint.y<img.rows-1){//在图像内
				if(img.at<uchar>(currentPoint.y,currentPoint.x)==0){//是边界点
					if(currentPoint.x>=img.cols-2||currentPoint.x<=2){//到达图像左边缘，跟踪结束
						stopFlag=true;
						break;
					}
					leftEdge[leftEdgeNum].x=currentPoint.x;
					leftEdge[leftEdgeNum].y=currentPoint.y;
					leftEdgeNum++;
					currentDirection-=2;//防止重复跟踪
					rootPoint=currentPoint;//根点更新
					break;
				}
			}
			currentDirection++;//方向顺时针旋转
		}
		if(counts==8){//跟踪了八次，没有找到边界点，说明跟踪结束，找到了一个左边界
			stopFlag=true;
		}//跟踪结束，找到了一个左边界，跳出循环，开始下一次跟踪
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
int differenceSumThreshold(Mat &imgGray,int period,int diff){
	int threshold=0;
	int thresholdMax=0;
	for(int i=1;i<imgGray.rows-1;i++){
		for(int j=1;j<imgGray.cols-1;j++){
			threshold=differenceSum(imgGray.at<uchar>(1,1),imgGray.at<uchar>(i,j));
			if(thresholdMax<=threshold){
				thresholdMax=threshold;//找到最大差比和
			}
		}
	}
	cout<<"thresholdMax:"<<thresholdMax*0.7<<endl;//输出最大差比和
	return thresholdMax*0.7;//阈值取黑白点的最大差比和的0.7倍
	// for(int i=imgGray.cols/2;i>period;i-=period){
		// if((imgGray.at<uchar>(imgGray.rows-2,i)-imgGray.at<uchar>(imgGray.rows-2,i-period))>=diff){//找到黑白点
		// 	x0=i-period;
		// 	y0=imgGray.rows-2;
		// 	threshold=differenceSum(imgGray.at<uchar>(imgGray.rows-2,i),imgGray.at<uchar>(imgGray.rows-2,i-period))*0.7;//阈值为黑白点的差比和的0.7倍
		// 	{
		// 		cout<<"x0,y0:"<<i<<","<<imgGray.rows-2<<endl;
		// 		cout<<"differenceSumThreshold:"<<threshold<<endl;
		// 	}
		// 	return threshold;
		// }
	// }
	// return threshold;
}
//差比和寻左线
int leftEdgeDetectWithDifferenceSum(Mat &imgGray,Point leftEdge[],int threshold,int period){
	int leftEdgeNum=0;
	//八邻域边缘跟踪寻找左边界，类似于leftEdgeDetectWithBinary函数中的实现，区别是判断方法不同
	int direction=0;//标记方向，避免搜索时重复，1，2，3，4，5，6，7，8分别为1右上，2正上，3左上，4正左，5左下，6正下，7右下，8正右，即从右上角逆时针方向
	Point currentPoint;//当前点
	Point rootPoint;//根点
	bool stopFlag=false;//停止标志
	int counts=0;//计数器
	int currentDirection=0;//当前方向
	const Point directions[]={Point(1,0),Point(1,-1),Point(0,-1),Point(-1,-1),Point(-1,0),Point(-1,1),Point(0,1),Point(1,1)};//八邻域，逆时针，从正右开始
	//找到第一个左边界点
	for(int i=imgGray.cols/2;i>period;i-=period){
		if(differenceSum(imgGray.at<uchar>(imgGray.rows-2,i),imgGray.at<uchar>(imgGray.rows-2,i-period))>=threshold){//大于阈值即为边界点
			cout<<"first left edge:"<<i-period<<endl;//输出第一个左边界点
			leftEdge[leftEdgeNum].x=i-period;
			leftEdge[leftEdgeNum].y=imgGray.rows-2;
			leftEdgeNum++;
			currentPoint.x=i-period;
			currentPoint.y=imgGray.rows-2;
			break;
		}
	}
	//开始边缘跟踪

	while(!stopFlag&&leftEdgeNum<1000){
		for(counts=0;counts<8;counts++){//循环八次进行跟踪
			if(currentDirection>=8){
				currentDirection-=8;
			}
			if(currentDirection<0){
				currentDirection+=8;
			}//防止数组越界
			currentPoint=Point{rootPoint.x+directions[currentDirection].x,rootPoint.y+directions[currentDirection].y};//跟踪
			cout<<"currentPoint:"<<currentPoint.x<<","<<currentPoint.y<<endl;
			if(currentPoint.x>0&&currentPoint.y>0&&currentPoint.x<imgGray.cols-1&&currentPoint.y<imgGray.rows-1){//在图像内
					currentDirection++;//方向顺时针旋转
				if(differenceSum(imgGray.at<uchar>(currentPoint.y,currentPoint.x),imgGray.at<uchar>(rootPoint.y,rootPoint.x))<=threshold){
					if(currentPoint.x>=imgGray.cols-2||currentPoint.x<=2){//到达图像左边缘，跟踪结束
						stopFlag=true;
						break;
					}
					leftEdge[leftEdgeNum].x=currentPoint.x;
					leftEdge[leftEdgeNum].y=currentPoint.y;
					leftEdgeNum++;
					currentDirection-=2;//方向反向，因为是从右边界点开始跟踪的，所以方向要反向，这样才能顺时针旋转，否则会逆时针旋转，找不到左边界
					rootPoint=currentPoint;//根点更新
					break;
				}
			}
			currentDirection++;//方向顺时针旋转
		}
		if(counts==8){//跟踪了八次，没有找到边界点，说明跟踪结束，找到了一个左边界
			stopFlag=true;
		}//跟踪结束，找到了一个左边界，跳出循环，开始下一次跟踪
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
//间隔num找两个点做垂线交右线于另一点，取中点做中线上的点
int midlineDetectWithVertical(Mat &img,Point leftEdge[],int leftEdgeNum,Point midline[],int num){//求左线的两点的垂线，求垂线与右线的交点，求交点的中点，作为中线的点
	int midlineNum=0;
	double k;//左线两点垂线方程的斜率
	double c;//左线两点垂线方程的常数项
	for(int i=0;i<leftEdgeNum-num;i+=num){
		//y-y0=k(x-x0)
		//y=kx+y0-kx0
		//c=y0-kx0
		if(leftEdge[i].y==leftEdge[i+1].y){//垂线不存在，向上寻找交点
			for(int j=leftEdge[i].y;j>0;j--){
				if(j>=img.rows-1){
				break;//与边界的交点不在图像内，跳过
				}
				if(img.at<uchar>(j,leftEdge[i].x)==0&&(leftEdge[i].y-j>=20)){//按垂线找到与右线的交点，交点坐标为(leftEdge[i].x+j,k*(leftEdge[i].x)+c)
					midline[midlineNum].x=leftEdge[i].x;
					midline[midlineNum].y=(leftEdge[i].y+j)/2;
					midlineNum++;
					break;
				}
			}
			continue;
		}
		k=-(double)(leftEdge[i].x-leftEdge[i+1].x)/(double)(leftEdge[i].y-leftEdge[i+1].y);
		c=leftEdge[i].y-k*leftEdge[i].x;
		int x0;
		int y0;
		for(int j=1;j<img.cols-leftEdge[i].x;j++){
			x0=leftEdge[i].x+j;
			y0=k*x0+c;
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
	int midlineNumDel[midlineNum];//记录需要删除的点的序号
	int midlineNumDelNum=0;//记录需要删除的点的数量
	for(int z=1;z<midlineNum-1;z++){
		//去除相邻距离过远的点（因为无法判断法线方向，索性去除）
		//错误的中心点必然距离相邻点过远，所以记录下来，最后删除
		//多增加了一些计算，相比之下降低了效率，减小了误差，但考虑到实际应用中中心点数量(midlineNum)较小，计算次数较少，可以接受
		if((((midline[z].x-midline[z-1].x)*(midline[z].x-midline[z-1].x)+(midline[z].y-midline[z-1].y)*(midline[z].y-midline[z-1].y))>150)&&(((midline[z].x-midline[z+1].x)*(midline[z].x-midline[z+1].x)+(midline[z].y-midline[z+1].y)*(midline[z].y-midline[z+1].y))>150)){
			midlineNumDel[midlineNumDelNum]=z;
			midlineNumDelNum++;
		}
	}
	for(int z=1;z<midlineNumDelNum-1;z++){//删除错误的中心点，即赋值为相邻点的中点
		midline[midlineNumDel[z]].x=(midline[midlineNumDel[z]-1].x+midline[midlineNumDel[z]+1].x)/2;
		midline[midlineNumDel[z]].y=(midline[midlineNumDel[z]-1].y+midline[midlineNumDel[z]+1].y)/2;
	}
	return midlineNum;
}

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
		if((num*m1-m2*m3)==0){//垂线不存在，向上寻找交点
			for(int j=leftEdge[i].y;j>0;j--){
				if(j>=img.rows-1){
				break;//与边界的交点不在图像内，跳过
				}
				if(img.at<uchar>(j,leftEdge[i].x)==0&&(leftEdge[i].y-j>=20)){//按垂线找到与右线的交点，交点坐标为(leftEdge[i].x+j,k*(leftEdge[i].x)+c)
					midline[midlineNum].x=leftEdge[i].x;
					midline[midlineNum].y=(leftEdge[i].y+j)/2;
					midlineNum++;
					break;
				}
			}
			continue;
		}
		k=-(double)(num*m4-m5)/(double)(num*m1-m2*m3);//拟合直线的垂线的斜率
		c=(m3-k*m2)/num;//拟合直线的曲线的垂线的常数项
		int x0;
		int y0;
		for(int j=leftEdge[i].x+1;j<img.cols;j++){
			x0=j;
			y0=k*x0+c;
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
	int midlineNumDel[midlineNum];//记录需要删除的点的序号
	int midlineNumDelNum=0;//记录需要删除的点的数量
	for(int z=1;z<midlineNum-1;z++){
		//去除相邻距离过远的点（因为无法判断法线方向，索性去除）
		//错误的中心点必然距离相邻点过远，所以记录下来，最后删除
		//多增加了一些计算，相比之下降低了效率，减小了误差，但考虑到实际应用中中心点数量(midlineNum)较小，计算次数较少，可以接受
		if((((midline[z].x-midline[z-1].x)*(midline[z].x-midline[z-1].x)+(midline[z].y-midline[z-1].y)*(midline[z].y-midline[z-1].y))>150)&&(((midline[z].x-midline[z+1].x)*(midline[z].x-midline[z+1].x)+(midline[z].y-midline[z+1].y)*(midline[z].y-midline[z+1].y))>150)){
			midlineNumDel[midlineNumDelNum]=z;
			midlineNumDelNum++;
			cout<<"delete:"<<z<<endl;
		}
	}
	for(int z=1;z<midlineNumDelNum-1;z++){//删除错误的中心点，即赋值为相邻点的中点
		// midline[midlineNumDel[z]].x=midline[midlineNumDel[z]+1].x;
		// midline[midlineNumDel[z]].y=midline[midlineNumDel[z]+1].y;

		midline[midlineNumDel[z]].x=(midline[midlineNumDel[z]-1].x+midline[midlineNumDel[z]+1].x)/2;
		midline[midlineNumDel[z]].y=(midline[midlineNumDel[z]-1].y+midline[midlineNumDel[z]+1].y)/2;
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
