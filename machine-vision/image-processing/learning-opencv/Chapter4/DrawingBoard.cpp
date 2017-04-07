//Create Draw Board
#include <opencv/highgui.h>
#include <opencv/cv.h>

CvRect box;
bool drawing_box = false;

void my_mouse_callback(int event_t,int x, int y,int flags,void *param);

static void draw_box(IplImage* img,CvRect rect){
	cvRectangle(img,cvPoint(box.x,box.y),cvPoint(box.x+ box.width,box.y+box.height),
		cvScalar(0xff,0x00,0x00) //box with BLUE line edge
		);
}

int main(int argc, char* argv[]){
	
	box = cvRect(-1,-1,0,0);
	IplImage* img = cvCreateImage(cvSize(200,200),IPL_DEPTH_8U,3);

	cvZero(img);

	IplImage* temp = cvCloneImage(img);
	cvNamedWindow("Box Example");

	cvSetMouseCallback("Box Example",my_mouse_callback,(void*)img);

	while(true){
		cvCopyImage(img,temp);
		if(drawing_box) {
			draw_box(temp,box);
		}

		cvShowImage("Box Example",temp);

		if(cvWaitKey(15) == 27)  break; //ESC button value is 27
	}

	cvReleaseImage(&img);
	cvReleaseImage(&temp);

	cvDestroyWindow("Box Example");
	
	return 0;
}

void my_mouse_callback(int event_t,int x, int y,int flags,void *param){


	IplImage* img = (IplImage*)param;

	switch(event_t){

	case CV_EVENT_MOUSEMOVE:
		if(drawing_box){
			box.width = x - box.x;
			box.height = y - box.y;
		}
		break;
	case CV_EVENT_LBUTTONDOWN:
		drawing_box = true;
		box = cvRect(x,y,0,0);
	    break;
	case CV_EVENT_LBUTTONUP:
		drawing_box = false;

		if(box.width < 0){
			box.x = box.x + box.width;
			box.width *= -1;
		}

		if(box.height < 0){
			box.y = box.y + box.height;
			box.height *= -1;
		}

		draw_box(img,box);
		break;

	}
}
