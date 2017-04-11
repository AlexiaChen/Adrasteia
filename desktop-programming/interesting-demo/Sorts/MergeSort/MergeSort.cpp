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


//合并序列
void Merge(data_t sourceArr[],data_t tempArr[],int startIndex,int midIndex,int endIndex)
{
	int i = startIndex,j = midIndex+1,k = startIndex;
	while(i != midIndex+1 && j != endIndex+1)
	{
		if(sourceArr[i].value_t > sourceArr[j].value_t){
			
			setcolor(BLUE);
			line(sourceArr[k].top_x,0,sourceArr[k].top_x,SCREEN_HEIGHT);

			delay_ms(1);

			setcolor(WHITE);

			line(sourceArr[k].top_x,0,sourceArr[k].top_x,SCREEN_HEIGHT);

			setcolor(BLACK);
			line(sourceArr[k].top_x,sourceArr[k].top_y,sourceArr[k].top_x,SCREEN_HEIGHT);

			tempArr[k].value_t = sourceArr[i].value_t;
			tempArr[k].top_y = sourceArr[i].top_y;

			

			k++;
			i++;
		}
			
		else{
			
			setcolor(RED);
			line(sourceArr[k].top_x,0,sourceArr[k].top_x,SCREEN_HEIGHT);

			delay_ms(1);

			setcolor(WHITE);

			line(sourceArr[k].top_x,0,sourceArr[k].top_x,SCREEN_HEIGHT);

			setcolor(BLACK);
			line(sourceArr[k].top_x,sourceArr[k].top_y,sourceArr[k].top_x,SCREEN_HEIGHT);
			
			tempArr[k].value_t = sourceArr[j].value_t;
			tempArr[k].top_y = sourceArr[j].top_y;

			

			k++;
			j++;
		}
			
	}
	while(i != midIndex+1){

		setcolor(BLUE);
		line(sourceArr[k].top_x,0,sourceArr[k].top_x,SCREEN_HEIGHT);

		delay_ms(1);

		setcolor(WHITE);

		line(sourceArr[k].top_x,0,sourceArr[k].top_x,SCREEN_HEIGHT);

		setcolor(BLACK);
		line(sourceArr[k].top_x,sourceArr[k].top_y,sourceArr[k].top_x,SCREEN_HEIGHT);
		
		tempArr[k].value_t = sourceArr[i].value_t;
		tempArr[k].top_y = sourceArr[i].top_y;

		

		k++;
		i++;
	}
		
	while(j != endIndex+1){

		setcolor(RED);
		line(sourceArr[k].top_x,0,sourceArr[k].top_x,SCREEN_HEIGHT);

		delay_ms(1);


		setcolor(WHITE);

		line(sourceArr[k].top_x,0,sourceArr[k].top_x,SCREEN_HEIGHT);

		setcolor(BLACK);
		line(sourceArr[k].top_x,sourceArr[k].top_y,sourceArr[k].top_x,SCREEN_HEIGHT);
		
		tempArr[k].value_t = sourceArr[j].value_t;
		tempArr[k].top_y = sourceArr[j].top_y;

		

		k++;
		j++;
	}
		
	for(i = startIndex;i <= endIndex;i++){
		sourceArr[i].value_t = tempArr[i].value_t;
		sourceArr[i].top_y = tempArr[i].top_y;
	}

		
}
//分路
void MSort(data_t A[], data_t TmpArray[], int Left, int Right)
{
	int Center;
	if(Left < Right)
	{
		Center = (Left + Right) / 2;
		MSort(A, TmpArray, Left, Center);
		MSort(A, TmpArray, Center + 1, Right);
		Merge(A, TmpArray,Left,Center , Right);
	}
}
//主排序入口
void MergeSort(data_t A[], int N)
{
	data_t *TmpArray;
	TmpArray = (data_t*)malloc(N * sizeof(data_t));
	if(TmpArray != NULL)
	{
		MSort(A, TmpArray, 0, N - 1);
		free(TmpArray);
	}
	else
		exit(1);
}

int main(int argc, char* argv[]){

	initgraph(SCREEN_WIDTH,SCREEN_HEIGHT);
	setcolor(YELLOW);
	outtextxy(20,20,"Merge sort demo, Press any key to start.");
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

	MergeSort(data, SCREEN_WIDTH);

	outtextxy(20,20,"Merge sort demo,Sorting finished.");
	getch();
	closegraph();

	return 0;
}

