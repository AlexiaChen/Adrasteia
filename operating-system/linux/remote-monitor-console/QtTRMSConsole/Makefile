###########################################  
#Makefile for simple programs  
###########################################  
   
INC= 
LIB=  -pthread
      
CC=g++  
CC_FLAG=  -pthread
      
PRG=qt-console  
OBJ=GenTab.o TCPSocket.o TRMSConsole.o Public.o Utils.o main.o

$(PRG):$(OBJ)  
	$(CC) $(INC) $(LIB) -o $@ $(OBJ)  
          
.SUFFIXES: .c .o .cpp  
.cpp.o:  
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o  $(LIB)
      
.PRONY:clean  
clean:  
	@echo "Removing linked and compiled files......"  
	rm -f $(OBJ) $(PRG)  