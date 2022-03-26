#ifndef BOARD_H
#define BOARD_H

#include "mbed.h"
#include "common.h"

#define NB_MOTOR 4
#define USTEP_RES 8

class board {
    public:

    
    /**
    * @fn Constructor
    */
    board();


    /**
    * @fn Destructor
    */
    ~board();


    /**
    * @fn uart_has_rcv
    * @return true: a new uart frame has been received
    *         false: no uart frames were received
    */
    bool uart_has_rcv(void);


    /**
    * @fn read_uart
    * @return the uart frame that has been received
    * @brief This functions should be called right after the
    *        @fn 
    */
    struct uart_frame uart_read(void);


    /**
    * @fn uart_send
    * @param c a character to send over the uart bus
    * @brief sets the uart in TX mode and send a character,
    *        Then sets the uart back in RX mode
    * @note 
    */
    int uart_send(char c);


    /**
    * @fn set_motor_pos
    * @param motor the value of the motor
    * @param pos the position of the motor in the z axis
    * @brief Sets the value of the motor for the calculation in the @fn home_motors 
    * 
    */
    void set_motor_pos(unsigned motor, float pos);


    /**
    * @fn home_motors()
    * @return RESAMPLE when motors have moved to the requested positino
    *         DONE     when no motors have moved since they are all stable
    * @brief
    *   Sets the motor position to the requested position by the
    *   printer.
    */
    enum UART_CMD home_motors(void);


    /**
    * @fn reset_motors()
    * @return DONE when all motors are offline
    * @brief 
    *   Sets the direction to LEFT and enables all motors.
    *   Then sleeps and wait until all motors are deactivated by
    *   the interrupt routine of the limit switches.
    */
    enum UART_CMD reset_motors(void);


    private:


    /*** === MOTOR FUNCTIONS === ***/


    /**
    * @fn enable_all_motors
    * @param en true: enable all motors
    *           false: disable all motors
    */
    void enable_all_motors(bool en);


    /**
    * @fn enable_motor
    * @param en true: enable motor
    *           false: disable motor
    * @param motor The motor to enable
    * @note the parameter motor is protected by doing:
    *       motor % NB_MOTOR
    */
    void enable_motor(bool en, unsigned motor);


    /**
    * @fn is_motor_active
    * @param motor the motor to verify
    * @return true if the motor is activated else
    *         false
    * @note the parameter motor is protected by doing:
    *       motor % NB_MOTOR
    */
    bool is_motor_active(unsigned motor);


    /**
    * @fn nb_motors_active
    * @return the number of motors that are active
    * @note the max value is NB_MOTOR
    */
    unsigned nb_motors_active(void);


    /**
    * @fn suspend_motors
    * @brief sets the drivers in sleep state
    *        This function disables all the motors
    *             stops the pwm
    *        And sets in sleep state
    *
    *       To wake-up one must call @fn wakeup_motors
    *       before calling any functions with PWM or MOTOR in the name
    */
    void suspend_motors(void);


    /**
    * @fn wakeup_motors
    * @brief Wakes up the motor from sleeping state
    *        Must be called after @fn suspend_motors to
    *        restart operation.
    * @note this functions sleeps
    */
    void wakeup_motors(void);


    /**
    * @fn enable_pwm
    * @param en true: enable the pwm
    *           false: disable the pwm
    * @note This function sleeps
    */
    void enable_pwm(bool en);


    /*** === UART FUNCTIONS === ***/


    /**
    * @fn set_printer_uart_state
    * @param state TX: set in transmit mode
    *              RX: set in receive mode
    * @brief changes the state of the uart for one wire half duplex
    *        communication between printer and nucleo.
    *
    * @note this function sleeps. Delays must be validated to make sure there is
    *        no contensions.
    */
    void set_printer_uart_state(enum UART_STATE state);

  
    /*** === ISR FUNCTIONS === ***/
    
    /**
    * @fn limit_isr
    * @brief Stops the motors when they hit the limit switches.
    */
    void limitsw_isr(void);


    /**
    * @fn rx_isr
    * @brief handles the communication comming from the printer
    */
    void rx_isr(void);


    /**
    * @fn step_cntr_isr
    * @brief counts the number of steps since the start of the pwm
    */
    void step_cntr_isr(void);


    /*** === VARIABLES === ***/

    // Flag to indicates that we are in an interruption routine
    // Used to prevent sleeping functions to be executed
    bool interrupted;

    // Flag to indicate that the drivers are sleeping and
    // need to be woken up before enabling PWM
    bool is_sleeping;

    // Flag to indicate if the pwm is enabled
    bool is_pwm_enabled;

    // Flag to indicate if the command that was received via UART is valid
    bool cmd_valid;

    // Flag that indicates that the uart is done receiving the frame
    bool rx_done;
    
    // The Z position of the motors
    float motor_pos[NB_MOTOR];

    // The number of pulses per second for the PWM
    unsigned int pps;

    // Holds the number of steps executed since the pwm was enabled
    unsigned long nb_step;

    // The number of step to execute until the step counter interrupt
    // shuts down the PWM. A value of 0 indicates that there are no limits
    unsigned long step_limit;

    // Holds the present state of the single half duplex uart
    enum UART_STATE ustate;

    // Holds the received frame of the uart
    struct uart_frame uframe;


    /*** === PINS === ***/
    InterruptIn         limitsw[NB_MOTOR];
    DigitalOut          en_n[NB_MOTOR];
    DigitalOut          sleep_n;
    DigitalOut          rst_n;
    PwmOut              step;
    InterruptIn         step_cntr;
    DigitalOut          dir;
    DigitalOut          led;
    UnbufferedSerial    *printer;
    DigitalIn           *reserved_uart;
};

#endif