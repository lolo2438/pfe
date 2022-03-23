#ifndef BOARD_H
#define BOARD_H

#include "mbed.h"
#include "common.h"

#define NB_MOTOR 4
#define USTEP_RES 8

class board {
    public:
    board();
    
    void enable_all_motors(void);
    void disable_all_motors(void);
    void enable_motor(unsigned motor);
    void disable_motor(unsigned motor);
    
    void disable_pwm(void);
    void enable_pwm(void);
    void reset_pwm_cntr(void);
    
    void suspend_motors(void);
    void wakeup_motors(void);
    
    void reset_position(void);

    private:    
    /* Functions */
    void limitsw_isr(void);
    void rx_isr(void);
    void step_cntr_isr(void);

    /* Variables */
    unsigned int pps;
    bool is_sleeping;
    bool is_pwm_enabled;
    unsigned int nb_step;
    
    enum UART_CMD cmd;
    //struct pos to be defined
    
    /* Pins Functions */
    InterruptIn         limitsw[NB_MOTOR];
    DigitalOut          en_n[NB_MOTOR];
    DigitalOut          sleep_n;
    DigitalOut          dir;
    DigitalOut          rst_n;
    PwmOut              step;
    InterruptIn         step_cntr;
    UnbufferedSerial    printer;
    DigitalOut          led;
};

#endif