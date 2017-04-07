//use widthStep to add value in pixels of the interested rectangle
#include <opencv/highgui.h>
#include <opencv/cv.h>



int main(int argc, char* argv[]){
	
	
	IplImage* src1 = cvLoadImage(argv[1]);
	IplImage* src2 = cvLoadImage(argv[2]);

	if(src1 == NULL || src2 == NULL)  exit(1);

	int x = 0;
	int y = 0;
	int width = 250;
	int height = 250;

	double alpha = (double)atof(argv[3]);
	double beta = (double) atof(argv[4]);

	cvSetImageROI(src1,cvRect(x,y,width,height));
	cvSetImageROI(src2,cvRect(x,y,width,height));

	cvAddWeighted(src1,alpha,src2,beta,0.0,src1);
	cvResetImageROI(src1);

	cvNamedWindow("output",1);
	cvShowImage("output",src1);

	cvWaitKey();




	
	
	return 0;
}