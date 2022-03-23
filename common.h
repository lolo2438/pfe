#ifndef COMMON_H
#define COMMON_H

enum motor_dir {DIR_LEFT = 0, DIR_RIGHT = 1};

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
    char data[4];
    char crc;
};

#endif