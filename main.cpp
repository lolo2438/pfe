#include "board.h"

board b;

struct uart_frame d = {};

enum UART_CMD reply;

float pos = 0.0f;

int main()
{
    while (true) {
        reply = CMD_NUL;
        while(!b.uart_has_rcv());
        d = b.uart_read();

        switch(d.cmd){
        case DATA0:
        case DATA1:
        case DATA2:
        case DATA3:
            pos = *(reinterpret_cast<float*>(d.data));
            b.set_motor_pos(d.cmd - DATA0, pos);
            reply = ACK;
            break;
        case HOME:
            reply = b.home_motors();
            break;
        case RST_MOTOR:
            reply = b.reset_motors();
            break;
        default:
            reply = d.cmd;
            break;
        }

        if(reply != CMD_NUL){
            b.uart_send(reply);
        }
    }
}
