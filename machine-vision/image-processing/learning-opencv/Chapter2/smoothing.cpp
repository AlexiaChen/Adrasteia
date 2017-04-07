#include <opencv/highgui.h>
#include <opencv/cv.h>

//load image and smoothing
static void smoothing(IplImage* img){
	cvNamedWindow("Smoothing in");
	cvNamedWindow("Smoothing out");
	cvShowImage("Smoothing in",img);
	
	IplImage* img_out = cvCreateImage(cvGetSize(img),IPL_DEPTH_8U,3);

	cvSmooth(img,img_out,CV_GAUSSIAN,3,3);
	cvShowImage("Smoothing out",img_out);
	cvReleaseImage(&img_out);

	cvWaitKey(0);
	cvDestroyWindow("Smoothing out");
	cvDestroyWindow("Smoothing in");

}
int main(int argc, char* argv[]){

	IplImage *img = cvLoadImage(argv[1]);
	smoothing(img);
	/*cvNamedWindow("Show",CV_WINDOW_AUTOSIZE);
	cvShowImage("Show",img);
	cvWaitKey(0);
	cvReleaseImage(&img);
	cvDestroyWindow("Show");*/
	return 0;
}