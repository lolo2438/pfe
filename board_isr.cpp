#include "board.h"

void board::limitsw_isr(void)
{
    static unsigned m = 0;

    interrupted = true;

    for(int i = 0; i < NB_MOTOR; i += 1) {
        if(!limitsw[i].read()){
            enable_motor(false, i);
        }
    }

    interrupted = false;
}


void board::rx_isr(void)
{
    interrupted = true;

    char buf = 0x00;
    printer->read(&buf, sizeof(char));

    if(!cmd_valid){
        uframe.cmd = (enum UART_CMD)buf;

        switch(uframe.cmd){
        case DATA0:
        case DATA1:
        case DATA2:
        case DATA3:
            cmd_valid = true;
            break;
        case HOME:
        case RST_MOTOR:
            cmd_valid = true;
            rx_done = true;
            break;
        default:
            break;
            // Send error to printer...
        }
        
    } else if(!rx_done) {
        if(uframe.data_cnt < 4){
            uframe.data[uframe.data_cnt++] = buf;
            if(uframe.data_cnt == 4)
                rx_done = true;
        }
    }

    interrupted = false;
}


void board::step_cntr_isr(void)
{
    interrupted = true;
    nb_step += 1;

    if(step_limit && nb_step >= step_limit)
        enable_pwm(false);

    interrupted = false;
}