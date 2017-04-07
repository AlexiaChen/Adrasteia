//Threshhold
// Evaluate the sum of the image 3 channels
#include <cstdio>
#include <opencv/highgui.h>
#include <opencv/cv.h>

static void sum_rgb(IplImage* src,IplImage* dst){
	
	IplImage* r = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	IplImage* g = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	IplImage* b = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);

	//splite the image onto color planes
	cvSplit(src,r,g,b,NULL);

	IplImage* s = cvCreateImage(cvGetSize(src),IPL_DEPTH_32F,1);

	cvZero(s);

	cvAcc(b,s);
	cvAcc(g,s);
	cvAcc(r,s);
	

	cvThreshold(s,s,100,100,CV_THRESH_TRUNC);

	cvConvertScale(s,dst,1,0);

	cvReleaseImage(&r);
	cvReleaseImage(&g);
	cvReleaseImage(&b);
	cvReleaseImage(&s);

}

int main(int argc, char* argv[]){
	
	cvNamedWindow("ThreshHold",1);
	IplImage* src = cvLoadImage(argv[1]);
	IplImage* dst = cvCreateImage(cvGetSize(src),src->depth,1);

	sum_rgb(src,dst);

	cvShowImage("ThreshHold",dst);

	while(true){
		if(cvWaitKey(10) == 27)  break;
	}

	cvDestroyWindow("ThreshHold");
	cvReleaseImage(&src);
	cvReleaseImage(&dst);
	
	return 0;
}

