#include <cmath>
#include <cstdlib>
#include <cstdlib>

#include <graphics.h>

const int SCREEN_WIDTH = 1024, SCREEN_HEIGHT = 1024;

#define DM1 (SCREEN_WIDTH-1)
#define _sq(x) ((x)*(x)) // square
#define _cb(x) abs((x)*(x)*(x)) // absolute value of cube
#define _cr(x) (unsigned char)(pow((x),1.0/3.0)) // cube root

static unsigned char RD(int i,int j){
	return (char)(_sq(cos(atan2((double)(j-512),(double)(i-512))/2))*255);
}
static unsigned char GR(int i,int j){
	return (char)(_sq(cos(atan2((double)(j-512),(double)(i-512))/2-2*acos((double)-1)/3))*255);
}
static unsigned char BL(int i,int j){
	return (char)(_sq(cos(atan2((double)(j-512),(double)(i-512))/2+2*acos((double)-1)/3))*255);
}

void pixel_write(int i, int j){
	static unsigned char color[3];
	ege::color_t col;
	
	color[0] = RD(i,j)&255;
	color[1] = GR(i,j)&255;
	color[2] = BL(i,j)&255;
	
	col = RGB(color[0],color[1],color[2]);
	putpixel_f(i,j,col);
}

int main(){
	
	initgraph(SCREEN_WIDTH,SCREEN_HEIGHT);
	
	for(int j=0;j<SCREEN_WIDTH;j++)
		for(int i=0;i<SCREEN_HEIGHT;i++)
			pixel_write(i,j);
	
	getch();
	closegraph();
	return 0;
}

