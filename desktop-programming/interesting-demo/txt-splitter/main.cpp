// *****************************************************************
//
//  Filesplit - txt file splitter 
//  Version 2.0
//  Copyright (C) 2014 MathxH Chen
//  Contact:           
//  GitHub:
//  Author:            MathxH Chen
//  
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
// *****************************************************************


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "splitter.h"

#define VERSION 2.0

static void usage_help()
{
   puts("Usage: file_splitter [-options]\n");
   puts("Where options include:\n");
   puts("   --help                                          print out this message\n");
   puts("   --version                                       print out the build\n");
   puts("  -s <source> <size in KBytes>  [<destination>]    split file into specified size\n");
   puts("  -n <source> <number> [<destination>]             the number of file\n");
}

int main(int argc, char **argv)
{

    if(argc ==  1) // if called with no arguments
	{
	   usage_help();
	   return 0;
	}
    
	else if(argc < 3)// if called with 1 argument
	{
	   if(!strcmp(argv[1],"--version"))
	   {
	       puts("file splitter version is ");
		   printf("%d\n",VERSION);
		   return 0;
	   }
	   
	   if(!strcmp(argv[1],"--help"))
	   {
	      usage_help();
		  return 0;
	   }
	}
	else
	{
	    if(!strcmp(argv[1],"-n"))  //splite with the number of file
		{
		   if(argc >5)
		   {
		      usage_help();
			  return 0;
		   }
		   
		   puts("splitting...\n");
		   
		   splite_file_n(argv[2],atoi(argv[3]),argv[4]);
		   
		   puts("********DONE************\n");
		   
		   return 0;
		}
		else if(!strcmp(argv[1],"-s")) //splite  with chunk_size
		{
		  if(argc > 5)
		  {
		    usage_help();
			return 0;
		  }
		  
		  puts("splitting...\n");
		  
		  splite_file_s(argv[2],atoi(argv[3]),argv[4]);
		  
		  puts("********DONE************\n");
		  
		  return 0;
		  
		  
		}
	}
    return 0;
}