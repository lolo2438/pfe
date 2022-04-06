#ifndef COMMON_H
#define COMMON_H

// CRTOUCH MAX TOLERANCE = 0.04 mm

#define NB_MOTOR 4

// Constants for formula
#define NB_STEP_PER_REVOLUTION 100

#define GEAR_RATIO 20/34

#define DISTANCE_M4_SCREW_PER_REVOLUTION_MM 0.7f

#define Z_OFFSET_HOME_MM 0.2f;

// Value calculated for tolerance of 0.041176 mm
#define HOME_STEP_TRESHOLD 10 

#define BAUD_RATE 9600

// NOTE: EN CE MOMENT ON PARLE DE LA ROUE ET NON DU BED
enum motor_dir {DIR_UP = 0, DIR_DOWN = 1};


enum UART_STATE {RX = 0, TX = 1};


enum UART_CMD {
    CMD_NUL = 0x00,
    
    //From printer
    DATA0  = 0x90,
    DATA1  = 0x91,
    DATA2  = 0x92,
    DATA3  = 0x93,
    HOME   = 0x44,
    RST_MOTOR  = 0xBC,

    //TO PRINTER
    ERR    = 0xAA,
    ACK     = 0x55,
    DONE    = 0x84,
    RESAMPLE = 0xCC
};


struct uart_frame {
    enum UART_CMD cmd;
    unsigned data_cnt;
    char data[4];
};


enum USTEP_VAL {
    FULL_STEP        = 1,
    HALF_STEP        = 2,
    QUARTER_STEP     = 4,
    EIGHTH_STEP      = 8,
    SIXTEENTH_STEP   = 16
};

#endif