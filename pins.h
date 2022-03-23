#ifndef PINS_H
#define PINS_H

#include "mbed.h"

//Note: All interruptIn must be on PORT_A

#define PIN_LIMITSW1        A1
#define PIN_LIMITSW2        A2
#define PIN_LIMITSW3        A3
#define PIN_LIMITSW4        A4

#define PIN_EN1             D9
#define PIN_EN2             D10
#define PIN_EN3             D11
#define PIN_EN4             D12

#define PIN_DIR             D2
#define PIN_SLEEP           D3
#define PIN_RST             D4

#define PIN_STEP            A6
#define PIN_STEP_COUNTER    A5

#define PIN_TX              D1
#define PIN_RX              D0

#define PIN_LED             D13
#endif