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
    bool is_motor_active(unsigned motor);
    unsigned nb_motors_active(void);

    void set_max_step(unsigned long step_limit);
    void disable_pwm(void);
    void enable_pwm(void);
    void reset_pwm_cntr(void);
    

    void suspend_motors(void);
    void wakeup_motors(void);
    
    void reset_position(void);

    bool has_rcv_uart(void);
    const struct uart_frame& read_uart(void);
    int uart_send(char c);

    /* Public pins */
    DigitalOut          dir;
    DigitalOut          led;

    private:    
    /* Functions */
    void limitsw_isr(void);
    void rx_isr(void);
    void step_cntr_isr(void);

    void set_printer_uart_state(enum UART_STATE state);

    /* Variables */
    unsigned int pps;
    bool is_sleeping;
    bool is_pwm_enabled;
    
    unsigned long step_limit;
    unsigned long nb_step;
    
    enum UART_STATE ustate;
    
    struct uart_frame uframe;
    uint32_t uart_data;
    enum UART_CMD ucmd;

    bool cmd_valid;
    unsigned frame_cntr;
    bool rx_done;

    /* Private pins */
    InterruptIn         limitsw[NB_MOTOR];
    DigitalOut          en_n[NB_MOTOR];
    DigitalOut          sleep_n;
    DigitalOut          rst_n;
    PwmOut              step;
    InterruptIn         step_cntr;

    UnbufferedSerial    *printer;
    DigitalIn           *reserved_uart;
};

#endif