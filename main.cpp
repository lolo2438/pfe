#include "board.h"

//BufferedSerial pc(USBTX,USBRX,9600);    //USBRX=PA_15, USB_TX=PA_2, no conflict

//TODO: Add timer to put motors in sleep state

board b;

struct uart_frame d = {};

enum UART_CMD reply;

float pos = 0.0f;

int main()
{
   // printf("========\n");
    while (true) {
        reply = CMD_NUL;
        while(!b.uart_has_rcv());
        d = b.uart_read();

        switch(d.cmd){
        case DATA0:
        case DATA1:
        case DATA2:
        case DATA3:
            // Verify CRC
            pos = *(reinterpret_cast<float*>(d.data));
            b.set_motor_pos(d.cmd - DATA0, pos);
            //printf("%x\n", *((int*)&pos));
            reply = ACK;
            break;
        case HOME:
            reply = b.home_motors();
            break;
        case RST_MOTOR:
            reply = b.reset_motors();
            break;
        default:
            reply = ERR;
            break;
        }

        if(reply != CMD_NUL){
            b.uart_send(reply);
        }
    }
}
