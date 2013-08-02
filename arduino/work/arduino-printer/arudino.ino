#include "controller.h"
void setup(){
	setupServo();
  	Serial.begin(PORT_RATE);
}

void loop(){
	char _RECV[SIZE];
  	recv_data(_RECV);
	state_machine(_RECV);
	controlServo(int state);
  	delay(100);
}
