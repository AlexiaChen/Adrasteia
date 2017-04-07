//if have a config file,config file(XML or YAML file) tells us How many frames (10) the video holds,
//the video resolution (320*240) and apply it to 3*3 color transition matrix
#include <opencv/highgui.h>
#include <opencv/cv.h>



int main(int argc, char* argv[]){
	
	float Array[9]={1,2,3,4,5,6,7,8,9};

	CvMat cmatrix = cvMat(1,3,CV_32FC3,Array);//创建1行3列3通道矩阵

	//wirte config file to disk

	CvFileStorage* fs = cvOpenFileStorage( "cfg.xml",0,CV_STORAGE_WRITE);
	if(fs == NULL) exit(1);
	cvWriteInt(fs,"frame_count",10);
	
	cvStartWriteStruct(fs,"frame_size",CV_NODE_SEQ);
	cvWriteInt(fs,0,320);
	cvWriteInt(fs,0,240);
	cvEndWriteStruct(fs);

	cvWrite(fs,"color_cvt_matrix",&cmatrix);
	cvReleaseFileStorage(&fs);
	
	//read config file from disk
	 fs = cvOpenFileStorage( "cfg.xml",0,CV_STORAGE_READ);
	if(fs == NULL) exit(1);
	
	int frame_count = cvReadIntByName(fs,0,"frame_count",5);
	CvSeq * s = cvGetFileNodeByName(fs,0,"frame_size")->data.seq;

	int frame_width = cvReadInt((CvFileNode*)cvGetSeqElem(s,0));
	int frame_height = cvReadInt((CvFileNode*)cvGetSeqElem(s,1));

	CvMat* color_cvt_matrix = (CvMat*)cvReadByName(fs,0,"color_cvt_matirx");
	cvReleaseFileStorage(&fs);





	
	
	return 0;
}