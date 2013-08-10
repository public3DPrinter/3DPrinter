#include<stdlib.h>
void setup(){
  Serial.begin(19200);
}

void loop(){
 //Serial.println(recv_data);
  recv_data();
  delay(100);
}

void recv_data(){
  char _RECV[20];
  int counter=0;
  memset(_RECV,0,sizeof(_RECV));
  if(Serial.available()){
    while(Serial.available()>0){
      _RECV[counter]=(char)Serial.read();
      counter++;
    }
    Serial.println(_RECV);
  }
}
