#include <Stepper.h>

#include "controller.h"

Stepper stepper(DEFAULT_STEPS,LEFT1,LEFT2,RIGHT1,RIGHT2);

void setup(){
  setupStepper();
  Serial.begin(PORT_RATE);
}

void loop(){
  char _RECV[SIZE];
  recv_data(_RECV);
  state = state_machine(_RECV);
  controlServo(state);
  delay(100);
}

void setupStepper(){
  stepper.setSpeed(DEFAULT_SPEED);		
}


void forward(double range){
  stepper.step(range);
  Serial.print("F");
}

void backward(double range){
  stepper.step(-range);
  Serial.print("F");
}

void recv_data(char* _RECV){
  int counter=0;
  memset(_RECV,'\0',SIZE);
  if(Serial.available()){
    _RECV[counter] = (char)Serial.read();
    Serial.println(_RECV);
  }
}

void controlServo(int state){
    double range=0.0;
    switch(state){
      case 0:forward(B_RANGE);
             break;
      case 1:backward(N_RANGE);
             break;
      case 9:
             break;
    }
}





