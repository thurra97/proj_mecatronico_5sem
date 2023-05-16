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

Motor::Motor(PinName pinoSaida) : pinoControle(pinoSaida) {
    pinoControle = 0;
}

void Motor::definirDirecao(Direcao direcao) {
    switch(direcao) {
        case HORARIO:
            pinoControle = 0;
            break;
        case ANTIHORARIO:
            pinoControle = 1;
            break;
    }
}
