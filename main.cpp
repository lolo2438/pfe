//#include "mbed.h"
#include "board.h"

BufferedSerial pc(USBTX,USBRX,9600);    //USBRX=PA_15, USB_TX=PA_2, no conflict

int main()
{
    board b;
    b.wakeup_motors();

    b.enable_all_motors();
    b.enable_pwm();
    while (true) {

    }
}

