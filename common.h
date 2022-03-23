#ifndef COMMON_H
#define COMMON_H

enum motor_dir {DIR_LEFT = 0, DIR_RIGHT = 1};

enum UART_CMD {
    CMD_NUL = 0x00,
    //From printer
    CONFIG  = 0x99,
    DATA    = 0x43,
    HOME    = 0x72,

    //TO PRINTER
    NACK    = 0xAA,
    ACK     = 0x55,
    DONE    = 0x84
};

#endif