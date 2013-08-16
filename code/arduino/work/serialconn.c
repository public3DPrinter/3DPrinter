#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<termios.h>
#include<errno.h>
#include<sys/types.h>  
#include<sys/stat.h>
#include<stdlib.h>  
#include"serialconn.h"
int main(int argc,char** argv){
	char* device = "/dev/ttyACM0";
	int flag;
	int len = 0;	
	int counter=0;
	if(argc > 1){
		device = argv[1];

	}
	printf("OPENNING SERIAL %s\n",device);
	flag = open(device,O_RDWR | O_NOCTTY | O_NDELAY);
	if(flag==-1){
		perror("ERROR-");
		exit(1);
	}
	printf("[OPEN SUCCEED]\n");
	setOption(flag);
	printf("SETUP SUCCEED");
	communicate_arduino(flag);
	close(flag);
	return 0;
}

void setOption(int flag){
	struct termios option;
	calloc(sizeof(struct termios),1);
	tcgetattr(flag,&option);
	cfsetispeed(&option,DEFAULT_RATE);
	cfsetospeed(&option,DEFAULT_RATE);
	tcsetattr(flag,TCSANOW,&option);
}

void communicate_arduino(int fd){
	int flag=1;
	char send[200];
	char recv[200];
	while (flag){
		printf("PLEASE INPUT COMMAND: \n");
		scanf("%s",send);
		if(!strcmp(send,"exit")){
			flag=0;
		}
		int rc = write(fd,send,sizeof(char)*200);
		if(rc==-1){
			exit(2);	
		}
	}	
}
