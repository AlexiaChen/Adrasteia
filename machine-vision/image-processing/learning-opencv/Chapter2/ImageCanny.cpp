#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <assert.h>


static IplImage* dopyrDown(IplImage* img,int filter = IPL_GAUSSIAN_5x5){

	//make sure the image is divisible by two
	//assert(img->height%2 == 0 && img->width%2 == 0);

	IplImage* img_out = cvCreateImage(cvSize(img->width / 2,img->height /2),
		img->depth,img->nChannels);

	cvPyrDown(img,img_out);

	return img_out;

}

static  IplImage* doCanny(IplImage* img, double lowThresh,double highThresh,
	double aperture){
		if(img->nChannels != 1){
			return NULL;
		}
	
		IplImage* img_out = cvCreateImage(cvGetSize(img),IPL_DEPTH_8U,1);

		if(img_out == NULL){
			fprintf(stderr,"create image failure!\n");
		}

		cvCanny(img,img_out,lowThresh,highThresh,aperture);

		return img_out;

}
int main(int argc,char* argv[]){

	IplImage *img = cvLoadImage(argv[1]);
	IplImage *img1 = dopyrDown(img,IPL_GAUSSIAN_5x5);
	IplImage *img2 =  dopyrDown(img1,IPL_GAUSSIAN_5x5);
	IplImage *gray = cvCreateImage(cvSize(img2->width,img2->height),8,1);
	
	cvNamedWindow("cvdoCanny in");
	cvNamedWindow("cvdoCanny out");
	cvShowImage("cvdoCanny in",img);

	cvCvtColor(img2,gray,CV_RGB2GRAY);
	
	IplImage *out = doCanny(gray,10,100,3);
	cvShowImage("cvdoCanny out",out);
	cvReleaseImage(&out);

	cvWaitKey(0);
	cvDestroyWindow("cvdoCanny out");
	cvDestroyWindow("cvdoCanny in");
	
	return 0;
}