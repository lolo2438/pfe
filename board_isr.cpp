#include "board.h"

void board::limitsw_isr(void)
{
    for(int i = 0; i < NB_MOTOR; i += 1) {
        if(!limitsw[i].read()){

            switch(i){
            case 0:
                dir = DIR_LEFT;
                break;
            case 1:
                dir = DIR_RIGHT;
                break;
            case 2:
                disable_pwm();
                break;
            case 3:
                enable_pwm();
                break;
            }

            //disable_motor(i);
        }
    }    
}

void board::rx_isr(void)
{
    char buf;
    printer->read(&buf, sizeof(char));

    // COMPATIBILITY FOR TESTS
    //ucmd = (enum UART_CMD)buf;
    //cmd_valid = true;

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
        if(frame_cntr < 4){
            uframe.data[frame_cntr++] = buf;
        } else {
            uframe.crc = buf;
            rx_done = true;
        }       
    }
}

void board::step_cntr_isr(void)
{
    nb_step += 1;

    // Step_limit can be set to 0;
    if(step_limit && nb_step >= step_limit)
        disable_pwm();

    //Need logit to sync reset
}