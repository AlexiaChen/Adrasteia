//Check the IPP library if installed and works fine. if print Modules 
//is none,the IPP not installed.
#include <opencv/highgui.h>
#include <opencv/cv.h>



int main(int argc, char* argv[]){
	
	 const char* libraries;
	 const char* modules;

	 cvGetModuleInfo(0,&libraries,&modules);

	 printf("Libraries: %s \n Modules: %s \n",libraries,modules);
	
	return 0;
}