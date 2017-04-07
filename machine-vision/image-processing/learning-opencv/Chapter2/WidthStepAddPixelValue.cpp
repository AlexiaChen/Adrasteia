//use widthStep to add value in pixels of the interested rectangle
#include <opencv/highgui.h>
#include <opencv/cv.h>

struct interest_img{

	IplImage* img;
};

struct interest_rect{
	int x;
	int y;
	int width;
	int height;
};

int main(int argc, char* argv[]){
	
	interest_img src;
	interest_rect rect;
	rect.x = atoi(argv[2]);
	rect.y = atoi(argv[3]);
	rect.width = atoi(argv[4]);
	rect.height = atoi(argv[5]);
	
	src.img = cvLoadImage(argv[1]);

	IplImage* sub_image = cvCreateImageHeader(cvSize(rect.width,rect.height),
		src.img->depth,
		src.img->nChannels
		);

	sub_image ->origin = src.img->origin;
	sub_image->widthStep = src.img->widthStep;
	sub_image->imageData = src.img->imageData + rect.y*src.img->widthStep +rect.x*src.img->nChannels;

	cvAddS(sub_image,cvScalar(1),sub_image);

	cvNamedWindow("output",1);
	cvShowImage("output",sub_image);
	cvWaitKey();
	cvReleaseImageHeader(&sub_image);



	
	
	return 0;
}