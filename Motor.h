#ifndef DEVICE_ROTATOR_H
#define DEVICE_ROTATOR_H

#include "mbed.h"

enum Direcao {
    HORARIO,
    ANTIHORARIO
};

class Motor {
    public:
        Motor(PinName pinoSaida);
        void definirDirecao(Direcao direcao);
    private:
        DigitalOut pinoControle;
};

#endif
