// CombSort.cpp : 定义控制台应用程序的入口点。
//

#include <graphics.h>

const int SCREEN_WIDTH = 1280; //  X-axis. 
const int SCREEN_HEIGHT = 600; //  Y-axis.

typedef struct _DATA{
	int top_x;
	int top_y; 
	int value_t;            //real data for sorting
}data_t;

static data_t data[SCREEN_WIDTH];


void comb_sort(data_t* seq, int size)
{
	const double shrink = 1.25;
	int i, delta = size, noswap = 0;
	
	while(!noswap)
	{
		for(noswap = 1, i = 0; i + delta < size; i++){
			
			if(seq[i].value_t > seq[i + delta].value_t)
			{
				setcolor(BLUE);
				line(seq[i].top_x,0,seq[i].top_x,SCREEN_HEIGHT);
				setcolor(RED);
				line(seq[i + delta].top_x,0,seq[i + delta].top_x,SCREEN_HEIGHT);
				
				delay_ms(2);
				seq[i].value_t         ^=   seq[i + delta].value_t;
				seq[i].top_y           ^=   seq[i + delta].top_y;

				seq[i + delta].value_t ^=   seq[i].value_t;
				seq[i + delta].top_y   ^=   seq[i].top_y;
				
				seq[i].value_t         ^=   seq[i + delta].value_t;
				seq[i].top_y           ^=   seq[i + delta].top_y;


				setcolor(WHITE);
				line(seq[i].top_x,0,seq[i].top_x,SCREEN_HEIGHT);
				line(seq[i + delta].top_x,0,seq[i + delta].top_x,SCREEN_HEIGHT);

				setcolor(BLACK);
				line(seq[i].top_x,seq[i].top_y,seq[i].top_x,SCREEN_HEIGHT);
				line(seq[i + delta].top_x,seq[i + delta].top_y,seq[i + delta].top_x,SCREEN_HEIGHT);
				
				noswap = 0;
			}
		}
			
		if(delta > 1)
		{
			delta /= shrink;
			noswap = 0;
		}		
		
		
	}
}

int main(int argc, char* argv[]){

	initgraph(SCREEN_WIDTH,SCREEN_HEIGHT);
	setcolor(YELLOW);
	outtextxy(20,20,"Comb sort demo, Press any key to start.");
	getch();

	cleardevice();
	//set background
	setbkcolor(WHITE);
	setcolor(BLACK);

	//generates random data for sorting
	randomize();
	for(int i = 0; i < SCREEN_WIDTH; ++i){
		data[i].top_y           =  rand() % SCREEN_HEIGHT;
		data[i].top_x           =  i + 1;
		data[i].value_t         =  SCREEN_HEIGHT - data[i].top_y;
	}

	for(int i = 0; i < SCREEN_WIDTH; ++i)
		line(data[i].top_x ,data[i].top_y,data[i].top_x, SCREEN_HEIGHT);

	comb_sort(data, SCREEN_WIDTH);

	outtextxy(20,20,"Comb sort demo,Sorting finished.");
	getch();
	closegraph();

	return 0;
}

