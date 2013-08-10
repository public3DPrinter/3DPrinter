#ifndef CONTROLLER_H_
#define COMTROLLER_H_
#include<string.h>
#define SIZE 64
#define PORT_RATE 19200
#define LEFT1 4
#define LEFT2 5
#define RIGHT1 6
#define RIGHT2 7
int state;

void recv_data(char[] _RECV){
  memset(_RECV,0,sizeof(_RECV));
  if(Serial.available()){
    while(Serial.available()>0){
      _RECV[counter]=(char)Serial.read();
  }
}

int state_machine(char[] _RECV){
    if(strcmp(_RECV,"forward"))
        return 0;
    else if(strcmp(_RECV,"backward"))
        return 1;
    else
        return 9;
}

void setupServo(){
    pinMode(LEFT1,OUTPUT);
    pinMode(LEFT2,OUTPUT);
    pinMode(RIGHT1,OUTPUT);
    pinMode(RIGHT2,OUTPUT);
}

void controlServo(int state){
    double range=0.0;
    switch(sate){
      case 0:forward(range);
             break;
      case 1:backward(range);
             break;
      case 9:
             break;
    }
}

void forward(double range){
    for(int i=0;i<100;i++){
      digitalWrite(LEFT1,LOW);
      digitalWrite(LEFT2,HIGH);
      digitalWrite(RIGHT1,HIGH);
      digitalWrite(RIGHT2,HIGH);
    }
}

void backward(double range){
    for(int i=0;i<100;i++){
      digitalWrite(LEFT1,LOW);
      digitalWrite(LEFT2,LOW);
      digitalWrite(RIGHT1,LOW);
      digitalWrite(RIGHT2,HIGH);
    }
}
#endif
