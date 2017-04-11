#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "splitter.h"


static void get_file_name_from_source(char* source, char* file_name)
{
   char* target = ".txt";// splite txt file only,from now
   char* move = NULL;
   


   if((move=strstr(source,target)) == NULL )

   {
     puts("file style must be txt file\n");
     exit(1);
   }
   
   while((*move) != '\\')
   {
     move--;
   }
   
   move++;
   
   strcpy(file_name, move);

   
}

void splite_file_s(char* source, size_t size, char* destination)
{
   FILE *fp_in = NULL;
   FILE *fp_out = NULL;
   long file_size = 0;     //KB
   size_t chunk_size = size; //KB
   long chunk_number = 0;
   char out_file_name[215];
   char in_file_name[50];
   
   memset(out_file_name, 0, sizeof(char)*215);
   memset(in_file_name, 0, sizeof(char)*50);
   
   get_file_name_from_source(source, in_file_name);
   
   
   fp_in = fopen(source, "r");
   
   if(fp_in == NULL)
   {
     puts("source file cannot open\n");
	 exit(1);
   }
   
   //obtain source file size
   fseek(fp_in, 0, SEEK_END);
   file_size = ftell(fp_in);
   file_size = file_size/1024;  // 1KB = 1024 B
   rewind(fp_in);

   
   if(chunk_size < 1)
   {
      puts("error: chunk_size less than 1KB\n");
	  exit(1);
   }
   else if(chunk_size > file_size)
   {
      puts("error: chunk_size is greater than source file size\n");
	  exit(1);
   }
   
   chunk_number = file_size / chunk_size;
   int i = 0;
   
   char c;
   char str_int[20];
   
   for(; i < chunk_number; ++i)
   {
      memset(out_file_name,0,sizeof(char)*215);
      
	  strcpy(out_file_name,destination);
	  strcat(out_file_name,"\\");
	  strcat(out_file_name,"splited");
	   strcat(out_file_name,"_");
	  itoa(i,str_int,10);
	  strcat(out_file_name,str_int);
	  strcat(out_file_name,"_");
      strcat(out_file_name,in_file_name);
	  
	  printf(out_file_name);
	  puts("\n");
	 
	  fp_out = fopen(out_file_name,"w");
	  
	  if(fp_out == NULL)
	  {
	    puts("error: write file open fail\n");
		exit(1);
	  }
	  
	  long size_count = 0;   //Bytes
	  
	  while(size_count <= chunk_size*1024)
      {
		c = fgetc(fp_in);
	    fwrite(&c, sizeof(c), 1, fp_out);
		size_count++;
		
		
	  }	

	  size_count = 0;

      fclose(fp_out);	  
   }
   
   
}


void splite_file_n(char* source, int number, char* destination)
{
   FILE *fp_in = NULL;
   FILE *fp_out = NULL;
   char out_file_name[215];
   char in_file_name[50];
   long file_size = 0;   //not KB, it is Bytes
   size_t chunk_size = 0;
   
   memset(out_file_name, 0, sizeof(char)*215);
   memset(in_file_name, 0, sizeof(char)*50);
   
   get_file_name_from_source(source, in_file_name);
   

   fp_in = fopen(source, "r");
   
   if(fp_in == NULL)
   {
     puts("source file cannot open\n");
	 exit(1);
   }
   
   //obtain source file size
   fseek(fp_in, 0, SEEK_END);
   file_size = ftell(fp_in);
   rewind(fp_in);
   
   chunk_size = file_size / number ;
   
   int i = 0;
   char c;
   char str_int[20];
   
   for(; i < number; ++i)
   {
      memset(out_file_name,0,sizeof(char)*215);
      
	  strcpy(out_file_name,destination);
	  strcat(out_file_name,"\\");
	  strcat(out_file_name,"splited");
	  strcat(out_file_name,"_");
	  itoa(i,str_int,10);
	  strcat(out_file_name,str_int);
	  strcat(out_file_name,"_");
      strcat(out_file_name,in_file_name);
	  
	  
	  fp_out = fopen(out_file_name,"w+");
	  
	  if(fp_out == NULL)
	  {
	    puts("error: write file open fail\n");
		exit(1);
	  }
	  
	  long size_count = 0;   //Bytes

	  while(size_count <= chunk_size*1024)
	  {
		  c = fgetc(fp_in);
		  fwrite(&c, sizeof(c), 1, fp_out);
		  size_count++;


	  }	

	  size_count = 0;
	  
	  fclose(fp_out);	 
	}
}



