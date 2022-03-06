#include "board.h"

void board::limitsw_isr(void)
{
    for(int i = 0; i < NB_MOTOR; i += 1) {
        if(!limitsw[i].read())
            disable_motor(i);
    }
}


void board::step_cntr_isr(void)
{
    nb_step += 1;

    // For PPS = 200 && ustep=8, 100ms = 160 cnt
    if(nb_step % 160 == 0)
        led = !led;

    //Need logit to sync reset
}