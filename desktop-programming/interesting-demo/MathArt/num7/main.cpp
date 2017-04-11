/*
This Program will take  a lot of time
*/


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
	#define D SCREEN_WIDTH
	#define M m[(x+D+(d==0)-(d==2))%D][(y+D+(d==1)-(d==3))%D]
	#define R rand()%D
	#define B m[x][y]
	return(i+j)?256-(BL(i,j))/2:0;
}
static unsigned char GR(int i,int j){
	#define A static int m[D][D],e,x,y,d,c[4],f,n;if(i+j<1){for(d=D*D;d;d--){m[d%D][d/D]=d%6?0:rand()%2000?1:255;}for(n=1
	return RD(i,j);
}
static unsigned char BL(int i,int j){
	A;n;n++){x=R;y=R;if(B==1){f=1;for(d=0;d<4;d++){c[d]=M;f=f<c[d]?c[d]:f;}if(f>2){B=f-1;}else{++e%=4;d=e;if(!c[e]){B=0;M=1;}}}}}return m[i][j];
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

