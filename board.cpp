#include "board.h"
#include "pins.h"
#include "common.h"

#define BAUD_RATE 9600

board::board() :
    pps(200),
    is_sleeping(false),
    cmd(CMD_NUL),
    nb_step(0),
    step(PIN_STEP),
    step_cntr(PIN_STEP_COUNTER),
    sleep_n(PIN_SLEEP, 1),
    dir(PIN_DIR, DIR_LEFT),
    rst_n(PIN_RST, 1),
    en_n{{PIN_EN1,1},{PIN_EN2,1},{PIN_EN3,1},{PIN_EN4,1}},
    limitsw{{PIN_LIMITSW1},{PIN_LIMITSW2},{PIN_LIMITSW3},{PIN_LIMITSW4}},
    printer(PIN_TX,NC,BAUD_RATE),
    led(PIN_LED,0)
{   
    // Setup PWM
    step.period_us(1000000/(pps*USTEP_RES));
    step = 0.5;  //Duty cycle of 50
    is_pwm_enabled = true;

    suspend_motors();
    
    // Setup Interrupts
    for(int i = 0; i < NB_MOTOR; i += 1) {
        limitsw[i].fall(callback(this,&board::limitsw_isr));
    }
    
    step_cntr.rise(callback(this,&board::step_cntr_isr));
}


void board::enable_all_motors(void)
{
    for(int i = 0; i < NB_MOTOR; i += 1)
        enable_motor(i);
}


void board::disable_all_motors(void)
{
    for(int i = 0; i < NB_MOTOR; i += 1)
        disable_motor(i);
}


void board::enable_motor(unsigned motor)
{
    en_n[motor % NB_MOTOR] = 0;
}


void board::disable_motor(unsigned motor)
{
    en_n[motor % NB_MOTOR] = 1;
}


void board::disable_pwm(void)
{
    if(!is_pwm_enabled)
        return;

    step.suspend();
    nb_step = 0;
    is_pwm_enabled = false;
}


void board::enable_pwm(void)
{
    if(is_sleeping || is_pwm_enabled)
        return;
     
    step.resume();
    is_pwm_enabled = true;
}

 void board::reset_pwm_cntr(void)
 {
     if(is_pwm_enabled && !is_sleeping) {
         step = 0;
         nb_step = 0;
         step = 0.5;
     } else {
         nb_step = 0;
     }
 }


void board::suspend_motors(void)
{
    if(is_sleeping)
        return;
    
    disable_all_motors();
    disable_pwm();
    
    sleep_n = 0;
    
    is_sleeping = true;
}


void board::wakeup_motors(void)
{
    if(!is_sleeping)
        return;

    sleep_n = 1;
    
    // Sleep for at least 1 ms as specified by datasheet
    // before issuing step cmd (making sure here)
    ThisThread::sleep_for(1s);
    is_sleeping = false;
}


void board::reset_position(void)
{
    if(is_sleeping) 
        return; // Need the pwm to reset position
    
    disable_all_motors();
    
    rst_n = 0;
    //wait for rising edge, maybe better way
    int last_step_cntr = step_cntr;
    while(last_step_cntr == step_cntr) {
        ThisThread::sleep_for(1s);
    }
    
    rst_n = 1;
}
