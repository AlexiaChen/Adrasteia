#include <cstdio>
#include <cstdlib>
#include <cstring>

const int MAX_BUFF_SIZE = 50000;

static void usage(char* argv[]){

	fprintf(stderr,"\nUsage: \n%s *.bf\n",argv[0]);

}

static char **push(char **sp, char *value_t){
	*sp = value_t;
	return sp + 1;
}

static char **pop(char **sp, char **value_t){
	if (value_t)
		*value_t = *(sp - 1);
	return sp - 1;
}

static char* get(char** sp){
	return *(sp - 1);
}


static char* get_right_matched_bracket(char *begin_t){
	char *p;
	int count = 0;
	for (p = begin_t + 1; *p; p++){
		if (*p == '[')
			count++;
		else if (*p == ']'){
			if (count == 0)
				return p;
			else
				count--;
		}
		 
	}
}


int main(int argc, char* argv[]){

	if(argc != 2){
		 usage(argv);
		 exit(1);
	}
	
	char* instruction = new char  [MAX_BUFF_SIZE];
	char* mem_buffer  = new char  [MAX_BUFF_SIZE];
	char** stack       = new char* [MAX_BUFF_SIZE/2];

	char*  ip  = NULL;        /*pointer to the instruction of the BF lang,like ip pointer in ASM*/
	char*  ptr = mem_buffer;  /* R/W pointer */
	char** sp  = stack;
	
	memset(instruction, 0, MAX_BUFF_SIZE);
	memset(mem_buffer,  0, MAX_BUFF_SIZE);
	memset(stack,       0, MAX_BUFF_SIZE/2);

	FILE* fp = fopen(argv[1],"r");
	if(NULL == fp){
	  fprintf(stderr,"\nCannot Open the Source File\n");
	  exit(1);
	} 

	size_t read_num;
	if((read_num = fread(instruction, sizeof( char ), MAX_BUFF_SIZE - 1,fp)) == 0){
		fprintf(stderr,"\nRead Error\n");
		exit(1);
	}

    instruction[read_num] = '\0';

	/* Parser */
	for(ip = instruction; *ip != '\0'; ip++){

		switch(*ip){
		    case '>':
				     ptr++;
				     break;
			case '<':
				     ptr--;
				     break;
			case '+':
				     (*ptr)++;
					 break;
			case '-':
				     (*ptr)--;
					 break;
			case '.':
				     putchar(*ptr);
					 break;
			case ',':
				     (*ptr)=getchar();
				     break;
			case '[':            /* while(*ptr) { */
				     if(!(*ptr)){
						 ip = get_right_matched_bracket(ip);
						 if (!ip){
							 fprintf(stderr,"\nError:\nMissing token ']'\n");
							 exit(1);
						 }

					 } else{
					     sp = push(sp, ip);
					 } 

				     break;
			case ']':         /* } */
				     if (*ptr){
						 if (sp == stack){
							 fprintf(stderr,"\nError:\nMissing token '['\n");
							 exit(1);
						 }
						        
					    ip = get(sp); /*get the item of the stack top*/
				     } else{
						 if (sp == stack){
							 fprintf(stderr,"\nError:\nMissing token '['\n");
							 exit(1);
						 }
						
					     sp = pop(sp, NULL);  /* if *ptr is equla to zero, then, poping the useless item */
				     }
				     break;
			case '\r':
			case '\n':
			case '\t':
			case ' ':
				     break;

			default:
				fprintf(stderr,"\nError:\ninvalid token: '%c' in your %s\n", *ip,argv[1]);
				exit(1);
		}
	}

	/*clean up*/
	delete [] instruction;
	delete [] stack;
	delete [] mem_buffer;
	
	return 0;
}