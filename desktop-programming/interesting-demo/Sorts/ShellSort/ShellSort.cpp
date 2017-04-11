// ShellSort.cpp : 定义控制台应用程序的入口点。
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



void shell_sort(data_t *seq, int size)
{
	int i, j, Increment;
	data_t Tmp={0};
	
	for(Increment = size / 2; Increment > 0; Increment /= 2)
		for(i = Increment; i < size; i++)
		{
			
			
			Tmp.value_t = seq[i].value_t;
			Tmp.top_y   = seq[i].top_y;
			
			for(j = i; j >= Increment; j-= Increment){
				
				
				
				if(Tmp.value_t < seq[j - Increment].value_t){
					
					setcolor(BLUE);
					line(seq[j].top_x,0,seq[j].top_x,SCREEN_HEIGHT);

					delay_ms(1);

					
					
					
					
					seq[j].value_t = seq[j - Increment].value_t;
				    seq[j].top_y   = seq[j - Increment].top_y;

					
				
					setcolor(WHITE);

					line(seq[j].top_x,0,seq[j].top_x,SCREEN_HEIGHT);

					setcolor(BLACK);
					line(seq[j].top_x,seq[j].top_y,seq[j].top_x,SCREEN_HEIGHT);

					

					
					

				
					
				}
				else
					break;
			}
			
			

			setcolor(RED);
			line(seq[j].top_x,0,seq[j].top_x,SCREEN_HEIGHT);

		
			//delay_ms(1);
			
			
			seq[j].value_t = Tmp.value_t;
			seq[j].top_y   = Tmp.top_y;


			setcolor(WHITE);

			line(seq[j].top_x,0,seq[j].top_x,600);

			setcolor(BLACK);
			line(seq[j].top_x,seq[j].top_y,seq[j].top_x,600);

			
			



		
		}
}



int main(int argc, char* argv[]){

	initgraph(SCREEN_WIDTH,SCREEN_HEIGHT);
	setcolor(YELLOW);
	outtextxy(20,20,"Shell sort demo, Press any key to start.");
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

	shell_sort(data, SCREEN_WIDTH);

	outtextxy(20,20,"Shell sort demo,Sorting finished.");
	getch();
	closegraph();


	


	
	return 0;
}

