#include <cmath>
#include <cstdlib>
#include <cstdlib>

#include <graphics.h>

const int SCREEN_WIDTH = 1024, SCREEN_HEIGHT = 1024;

#define DM1 (SCREEN_WIDTH-1)
#define _sq(x) ((x)*(x)) // square
#define _cb(x) abs((x)*(x)*(x)) // absolute value of cube
#define _cr(x) (unsigned char)(pow((x),1.0/3.0)) // cube root

static unsigned char BL(int i,int j);

static unsigned char RD(int i,int j){
	#define A float a=0,b,k,r,x
	#define B int e,o
	#define C(x) x>255?255:x
	#define R return
	#define D SCREEN_WIDTH
	R BL(i,j)*(D-i)/D;
}
static unsigned char GR(int i,int j){
	#define E DM1
	#define F static float
	#define G for(
	#define H r=a*1.6/D+2.4;x=1.0001*b/D
	R BL(i,j)*(D-j/2)/D;
}
static unsigned char BL(int i,int j){
	F c[D][D];if(i+j<1){A;B;G;a<D;a+=0.1){G b=0;b<D;b++){H;G k=0;k<D;k++){x=r*x*(1-x);if(k>D/2){e=a;o=(E*x);c[e][o]+=0.01;}}}}}R C(c[j][i])*i/D;
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

