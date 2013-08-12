#ifndef CONTROLLER_H_
#define CONTROLLER_H_
#include<string.h>
#define SIZE 16
#define PORT_RATE 19200
#define LEFT1 6
#define LEFT2 7
#define RIGHT1 8
#define RIGHT2 9
#define DEFAULT_STEPS 200
#define DEFAULT_SPEED 60
#define B_RANGE 800
#define N_RANGE 400
int state;
void setupStepper();
void recv_data(char* _RECV);
int state_machine(char* _RECV);
void controlServo(int state);
void forward(double range);
void backward(double range);

int state_machine(char* _RECV){
    if(!strcmp(_RECV,"B"))
        return 0;
    else if(!strcmp(_RECV,"N"))
        return 1;
    else
        return 9;
}





#endif
