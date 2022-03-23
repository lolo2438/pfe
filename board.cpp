#include "board.h"
#include "pins.h"
#include "common.h"

#define BAUD_RATE 9600

board::board() :
    pps(100),
    is_sleeping(false),
    ucmd(CMD_NUL),
    printer(nullptr),
    reserved_uart(nullptr),
    cmd_valid(false),
    rx_done(false),
    frame_cntr(0),
    uframe{CMD_NUL,0,0,0,0},
    ustate(TX),
    uart_data(0),
    nb_step(0),
    step_limit(0),
    step(PIN_STEP),
    step_cntr(PIN_STEP_COUNTER),
    sleep_n(PIN_SLEEP, 1),
    dir(PIN_DIR, DIR_LEFT),
    rst_n(PIN_RST, 1),
    en_n{{PIN_EN1,1},{PIN_EN2,1},{PIN_EN3,1},{PIN_EN4,1}},
    limitsw{{PIN_LIMITSW1},{PIN_LIMITSW2},{PIN_LIMITSW3},{PIN_LIMITSW4}},
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

    set_printer_uart_state(RX);
}

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

bool board::has_rcv_uart(void)
{
    return cmd_valid;
}

const struct uart_frame& board::read_uart(void)
{
    cmd_valid = false;
    rx_done = false;
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
        wakeup_motors();
     
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
    ThisThread::sleep_for(1ms);
    is_sleeping = false;
}


bool board::is_motor_active(unsigned motor)
{
    if(en_n[motor].read() == 0)
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


void board::set_max_step(unsigned long step_limit)
{
    this->step_limit = step_limit;
}


void board::reset_position(void)
{
    disable_all_motors();
    enable_pwm();

    rst_n = 0;
    //wait for rising edge, maybe better way
    int last_step_cntr = step_cntr;
    while(last_step_cntr == step_cntr) {
        ThisThread::sleep_for(1ms);
    }
    
    rst_n = 1;

    disable_pwm();
}
