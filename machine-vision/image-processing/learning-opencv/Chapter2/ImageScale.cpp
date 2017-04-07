#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <assert.h>

//
static IplImage* dopyrDown(IplImage* img,int filter = IPL_GAUSSIAN_5x5){
	
	//make sure the image is divisible by two
	assert(img->height%2 == 0 && img->width%2 == 0);
	
	IplImage* img_out = cvCreateImage(cvSize(img->width / 2,img->height /2),
	img->depth,img->nChannels);

	cvPyrDown(img,img_out);

	return img_out;

}
int main(int argc,char* argv[]){

	IplImage *img = cvLoadImage(argv[1]);
	
	cvNamedWindow("cvPyrDown in");
	cvNamedWindow("cvPyrDown out");
	cvShowImage("cvPyrDown in",img);
	
	IplImage *out = dopyrDown(img);
	cvShowImage("cvPyrDown out",out);
	cvReleaseImage(&out);

	cvWaitKey(0);
	cvDestroyWindow("cvPyrDown out");
	cvDestroyWindow("cvPyrDown in");
	
	return 0;
}