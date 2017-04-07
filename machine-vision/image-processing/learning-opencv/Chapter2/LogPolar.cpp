/*read a RGB video and ouput the gray scale of images sequence to a new file*/

#include <stdlib.h>

#include <opencv/highgui.h>
#include <opencv/cv.h>




int main(int argc, char* argv[]){

	CvCapture* capture = NULL;
	capture = cvCreateFileCapture(argv[1]);
	if(!capture){
		fprintf(stderr,"cvCreateFileCapture failed\n");
		exit(1);
	}

	IplImage* bgr_frame = cvQueryFrame(capture);
	double fps = cvGetCaptureProperty(capture,CV_CAP_PROP_FPS);

	CvSize size = cvSize(
		(int)cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH),
		(int)cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT)
		);

	CvVideoWriter* writer = cvCreateVideoWriter(argv[2],
		CV_FOURCC('M','J','P','G'),
		fps,
		size
		);

	IplImage* logpolar_frame = cvCreateImage(size,IPL_DEPTH_8U,3);

	while( (bgr_frame = cvQueryFrame(capture)) != NULL){
		cvLogPolar(bgr_frame,
			logpolar_frame,
			cvPoint2D32f(bgr_frame->width/2,bgr_frame->height/2),//center of LogPolar realted
			40,
			CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS
			);

		cvWriteFrame(writer,logpolar_frame);
	}

	cvReleaseVideoWriter(&writer);
	cvReleaseImage(&logpolar_frame);
	cvReleaseCapture(&capture);
	
	
	return 0;
}