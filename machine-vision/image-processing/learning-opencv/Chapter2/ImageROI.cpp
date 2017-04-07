//use Image ROI to add value in pixel
#include <opencv/highgui.h>
#include <opencv/cv.h>



int main(int argc, char* argv[]){
	
	
	IplImage* src = cvLoadImage(argv[1]);
	if(argc == 7 && src != NULL){
		int x = atoi(argv[2]);
		int y = atoi(argv[3]);
		int width = atoi(argv[4]);
		int height = atoi(argv[5]);

		int add_t =atoi(argv[6]);;
		cvSetImageROI(src,cvRect(x,y,width,height));
		cvAddS(src,cvScalar(add_t,250),src);//BGR order in image Data,add_t is blue value,250 is green value
		cvResetImageROI(src);
		cvNamedWindow("output",1);
		cvShowImage("output",src);
		cvWaitKey();
	}

	
	
	return 0;
}