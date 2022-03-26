#include "board.h"
#include "pins.h"
#include "common.h"

#define BAUD_RATE 9600

/*** === PUBLIC FUNCTIONS === ***/

board::board() :
    pps(100), nb_step(0), step_limit(0),
    is_sleeping(false), interrupted(false),
    printer(nullptr), reserved_uart(nullptr),
    cmd_valid(false), rx_done(false), ustate(TX),
    uframe{CMD_NUL,0,0,0,0},
    step(PIN_STEP), step_cntr(PIN_STEP_COUNTER),
    sleep_n(PIN_SLEEP, 1), rst_n(PIN_RST, 1),
    dir(PIN_DIR, DIR_DOWN),
    en_n{{PIN_EN1,1},{PIN_EN2,1},{PIN_EN3,1},{PIN_EN4,1}},
    limitsw{{PIN_LIMITSW1},{PIN_LIMITSW2},{PIN_LIMITSW3},{PIN_LIMITSW4}},
    motor_pos{},
    led(PIN_LED,0)
{   
    // Setup PWM
    step.period_us(1000000/(pps*USTEP_RES));
    step = 0.5;  //Duty cycle of 50%
    is_pwm_enabled = true;

    //suspend_motors();
    
    // Setup Interrupts
    for(int i = 0; i < NB_MOTOR; i += 1) {
        limitsw[i].fall(callback(this,&board::limitsw_isr));
    }
    
    step_cntr.rise(callback(this,&board::step_cntr_isr));

    set_printer_uart_state(RX);
}


board::~board()
{
    delete printer;
    delete reserved_uart;
}


bool board::uart_has_rcv(void)
{
    return rx_done;
}


struct uart_frame board::uart_read(void)
{
    cmd_valid = false;
    rx_done = false;
    
    uframe = (struct uart_frame){};

    return uframe;
}


int board::uart_send(char c)
{
    set_printer_uart_state(TX);
    c = printer->write(&c, sizeof(char));
    ThisThread::sleep_for(3ms);
    set_printer_uart_state(RX);

    return 1;
}


void board::set_motor_pos(unsigned motor, float pos)
{
    motor_pos[motor % NB_MOTOR] = pos;
}


enum UART_CMD board::home_motors(void)
{
    bool motor_moved = false;
    
    const unsigned long STEP_TRESHOLD = 200; // ADJUST ME

    enable_pwm(false);
    dir = DIR_UP;

    for(unsigned i = 0; i < NB_MOTOR; i += 1){
        // TODO: calculate the number of steps needed to go to the position
        step_limit = 100 * USTEP_RES;
        
        if(step_limit >= STEP_TRESHOLD){
            
            enable_motor(true, i);
            enable_pwm(true);

            while(is_pwm_enabled){
                ThisThread::sleep_for(100ms);
            }

            enable_motor(false, i);
        } 
    }

    step_limit = 0;

    return motor_moved ? RESAMPLE : DONE;
}


enum UART_CMD board::reset_motors(void)
{
    dir = DIR_DOWN; 
    enable_all_motors(true);
    enable_pwm(true);

    while(nb_motors_active()){
        ThisThread::sleep_for(100ms);
    }

    enable_all_motors(false);

    // Reset driver
    rst_n = 0;
    int last_step_cntr = step_cntr;
    while(last_step_cntr == step_cntr);
    
    rst_n = 1;

    enable_pwm(false);

    for(unsigned i=0; i<NB_MOTOR; i+=1)
        motor_pos[i] = 0.0f;

    return DONE;
}


/*** === PRIVATE FUNCTIONS === ***/


void board::set_printer_uart_state(enum UART_STATE state)
{
    if(ustate == RX)
        ThisThread::sleep_for(1ms);

    delete printer;
    printer = nullptr;

    delete reserved_uart;
    reserved_uart = nullptr;

    switch(state) {
        case TX:
            printer = new UnbufferedSerial(PIN_TX,NC,BAUD_RATE);
            reserved_uart = new DigitalIn(PIN_RX, PullNone);
            break;
        case RX:
            printer = new UnbufferedSerial(NC,PIN_RX,BAUD_RATE);
            printer->attach(callback(this,&board::rx_isr));
            reserved_uart = new DigitalIn(PIN_TX, PullNone);
            break;
    }

    ustate = state;
}


void board::enable_all_motors(bool en)
{
    for(int i = 0; i < NB_MOTOR; i += 1)
        enable_motor(en, i);
}


void board::enable_motor(bool en, unsigned motor)
{
    en_n[motor % NB_MOTOR] = !en;
}


void board::enable_pwm(bool en)
{
    if(is_pwm_enabled == en && !is_sleeping)
        return;

    if(en) {
        if(is_sleeping && !interrupted)
            wakeup_motors();

        step.resume();
    } else {
        step.suspend();
        nb_step = 0;
    }

    is_pwm_enabled = en;
}


void board::suspend_motors(void)
{
    if(is_sleeping)
        return;
    
    enable_all_motors(false);
    enable_pwm(false);
    
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
    ThisThread::sleep_for(1ms);
    is_sleeping = false;
}


bool board::is_motor_active(unsigned motor)
{
    if(en_n[motor % NB_MOTOR].read() == 0)
        return true;
    else 
        return false;
}


unsigned board::nb_motors_active(void)
{
    unsigned nb_active = 0;

    for(unsigned i=0; i<NB_MOTOR; i+=1){
        if(en_n[i].read() == 0)
            nb_active += 1;
    }

    return nb_active;
}