//#include "mbed.h"
#include "board.h"

BufferedSerial pc(USBTX,USBRX,9600);    //USBRX=PA_15, USB_TX=PA_2, no conflict

//TODO: Add timers for overheat protection of motors

board b;

struct uart_frame data = {};

enum UART_CMD reply;

float requested_pos[NB_MOTOR] = {};
float motor_pos[NB_MOTOR] = {};

enum UART_CMD home_motors();
enum UART_CMD reset_motors();


/* Algorithme
* 1. imprimante -> Reset
* 2. Ack -> imprimante
* 3. Imprimante -> P1
* 4. Ack -> imprimante
* 5. REPEAT for all points
* 6. If moved(motors)
*      RESAMPLE
*    else
*      DONE
* ANYTIME: If error communication ERR, else ACK
*/
int main()
{
    b.led = 0;
    while (true) {
        reply = CMD_NUL;

        while(!b.has_rcv_uart());
        data = b.read_uart();

        switch(data.cmd){
        case DATA0:
        case DATA1:
        case DATA2:
        case DATA3:
            // Verify CRC
            requested_pos[data.cmd - DATA0] = *((float*)data.data);
            reply = ACK;
            break;
        case HOME:
            reply = home_motors();
        case RST_MOTOR:
            reply = reset_motors();
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


/**
* @fn home_motors()
* @return RESAMPLE when motors have moved to the requested positino
*         DONE     when no motors have moved since they are all stable
* @brief
*   Sets the motor position to the requested position by the
*   printer.
*/
enum UART_CMD home_motors()
{
    bool motor_moved = false;
    
    const unsigned long STEP_TRESHOLD = 200; // ADJUST ME

    b.disable_pwm();

    for(unsigned i=0; i<NB_MOTOR; i+=1){
        // TODO: calculate the number of steps needed to go to the position
        unsigned long nb_step;
        
        if(nb_step >= STEP_TRESHOLD){
            b.set_max_step(nb_step);
            b.dir = DIR_RIGHT;
            b.enable_motor(i);
            b.enable_pwm();

            while(b.is_motor_active(i)){
                ThisThread::sleep_for(100ms);
            }
            
            motor_moved = true;
        } 
    }
    
    return motor_moved ? RESAMPLE : DONE;
}


/**
* @fn reset_motors()
* @return DONE when all motors are offline
* @brief 
*   Sets the direction to LEFT and enables all motors.
*   Then sleeps and wait until all motors are deactivated by
*   the interrupt routine of the limit switches.
*/
enum UART_CMD reset_motors()
{
    b.dir = DIR_LEFT;
    b.enable_all_motors();
    b.enable_pwm();

    while(b.nb_motors_active()){
        ThisThread::sleep_for(100ms);
    }

    b.disable_pwm();
    b.reset_position();

    for(unsigned i=0; i<NB_MOTOR; i+=1)
        motor_pos[i] = 0.0f;

    return DONE;
}
