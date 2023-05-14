#ifndef DEVICE_ROTATOR_H
#define DEVICE_ROTATOR_H

#include "mbed.h"

class Motor {
    public:
        Motor(PinName outputPin);
        void SentidoHorario(void);
        void SentidoAnti(void);
    private:
        DigitalOut controlPin;
};

#endif
