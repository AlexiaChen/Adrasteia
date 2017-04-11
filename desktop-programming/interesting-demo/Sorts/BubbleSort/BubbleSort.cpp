// BubbleSort.cpp : 定义控制台应用程序的入口点。
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

void bubble_sort(data_t *seq, int size){


	int i, j;
	for(i = 0; i < size; ++i)
	{
		for(j = i + 1; j < size ; ++j)
		{
			if(seq[i].value_t > seq[j].value_t)
			{

				setcolor(BLUE);
				line(seq[i].top_x,0,seq[i].top_x,SCREEN_HEIGHT);
				setcolor(RED);
				line(seq[j].top_x,0,seq[j].top_x,SCREEN_HEIGHT);
				
				delay_ms(1);
				//swap two data, Actually, This way is  not faster than normal way, which using var temp  
				seq[i].value_t ^= seq[j].value_t;
				seq[i].top_y ^= seq[j].top_y;
				
				seq[j].value_t ^= seq[i].value_t;
				seq[j].top_y ^= seq[i].top_y;

				seq[i].value_t ^= seq[j].value_t;
				seq[i].top_y ^= seq[j].top_y;

				
				
				

				setcolor(WHITE);
				line(seq[i].top_x,0,seq[i].top_x,SCREEN_HEIGHT);
				line(seq[j].top_x,0,seq[j].top_x,SCREEN_HEIGHT);

                setcolor(BLACK);
				line(seq[i].top_x,seq[i].top_y,seq[i].top_x,SCREEN_HEIGHT);
				line(seq[j].top_x,seq[j].top_y,seq[j].top_x,SCREEN_HEIGHT);

				
			}
		}
	}

}





int main(int argc, char* argv[]){
	
	initgraph(SCREEN_WIDTH,SCREEN_HEIGHT);
	setcolor(YELLOW);
	outtextxy(20,20,"Bubble sort demo, Press any key to start.");
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

	bubble_sort(data, SCREEN_WIDTH);
	
	outtextxy(20,20,"Bubble sort demo,Sorting finished.");
	getch();
	closegraph();
	return 0;
}

