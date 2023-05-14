#include "mbed.h"
#include "Motor.h"
#include "TextLCD.h"


I2C i2c_lcd(I2C_SDA, I2C_SCL);
TextLCD_I2C lcd(&i2c_lcd, 0x4e, TextLCD::LCD20x4);

// DECLARANDO BOTOES DE INTERAÇÃO
InterruptIn botaoCONTINUA(PB_13);
InterruptIn botaoCONFIRMACAO(PB_15);
InterruptIn botaoVOLTAR(PB_14);

// BOTAO DE EMERGENCIA
InterruptIn botaoEMERGENCIA(PB_2);

// DECLARANDO TODOS OS FIM DE CURSO(EIXOS X, Y E Z)
InterruptIn FimDeCursoX1(D9);
InterruptIn FimDeCursoX2 (D8);
InterruptIn FimDeCursoY1(D7);
InterruptIn FimDeCursoY2(D6);
InterruptIn FimDeCursoZ1(D5);
InterruptIn FimDeCursoZ2(D4);


//DECLARANDO JOYSTICK
AnalogIn JoyStick(A0);

// DECLARANDO BOTAO DE CONFIRMAR
InterruptIn botaoX(D13);
InterruptIn botaoY(D11);
InterruptIn botaoZ(D12);

// DigitalOut reset_motors(PB_1);

// DECLARANDO MOTORES
Motor MotorX(PA_12);
Motor MotorY(PA_11);
Motor MotorZ(PB_12);

// DECLARANDO ENABLES
DigitalOut EnableMotorX(PC_8);
DigitalOut EnableMotorY(PC_6);
DigitalOut EnableMotorZ(PC_5);

// PIPETA
DigitalOut AcionamentoPipeta(PC_4);

// DECLARANDO STEP
DigitalOut StepDriver(PB_11);

// VARIAVEIS DO DO REFERENCIAMENTO
int tempo = 2;
int AutomatizacaoMotor = 0;

bool Referenciamento = 0;
int CounterX = 0;
int CounterY = 0;
int CounterZ= 0;
int Counter = 0;
int ValorInicialX = 0;
int ValorFinalX = xx;
int PosicaoX;
int CounterAutoZ;
int CounterAutoX;
int CounterAutoY;
int ValorInicialZ = 0;
int ValorFinalZ = xx;
int PosicaoZ;
int ValorInicialY = 0;
int ValorFinalY = xx;
int PosicaoY;
int etapa = 2;
int Referenciamento_Done = 0;
int ContaIdex;
int UltimoX;
int UltimoY;
int UltimoZ;
int tamanho;
int y;

int listaPosX[10];
int listaPosZ[10];
int listaPosY[10];
int lista_ml[10];

int SinaljogManual = 1;
int SinaljogAutomatico = 1;
int SinalReferenciamentoX = 0;
int SinalReferenciamentoY = 0;
int SinalReferenciamentoZ = 0;

int eixo_x_finalizado = 0;
int eixo_y_finalizado = 0;
int flag_emergencia = 0;

int execucao = 0;
int NSoltar = 1;
int conta_posicoes = 0;
int mililitros = 0;
int quantidade_mls = 0;
int etapa_pos_setup = 100;
bool autoriza_etapa = 0;

Timer debounce;

void lcd_show(int estado) {
    lcd.cls();

    if (estado == 0) {
        lcd.printf("Olá! \n");
        lcd.printf("Pressione 'ENTER' \n");
        lcd.printf("para iniciar o \n");
        lcd.printf("referenciamento");
        execucao = 1;
    } else if (estado == 1) {
        lcd.printf("Referenciamento \n");
        lcd.printf("Em \n");
        lcd.printf("Execucao... \n");
    } else if (estado == 2) {
        lcd.printf("Referenciamento \n");
        lcd.printf("concluido \n");
        lcd.printf("Deseja selecionar a posicao de pega ? \n");
    } else if (estado == 3) {
        lcd.printf("Botao de emergencia!! beep  \n");
    } else if (estado == 4) {
        lcd.printf("X de coleta:%4d\n", PosicaoX);
        lcd.printf("Y de coleta:%4d\n", PosicaoY);
        lcd.printf("Z de coleta:%4d\n", PosicaoZ);
        lcd.printf("Pressione 'Continua'");
    } else if (estado == 5) {
        lcd.printf("Qtd de pontos\n");
        lcd.printf("De despejo: %3d\n", NSoltar);
        lcd.printf("Pressione 'Confirma' \n");
        lcd.printf("Para confirmar");
    } else if (estado == 6) {
        lcd.printf("Aperte 'Confirma' para \n");
        lcd.printf("iniciar a selecao\n");
        lcd.printf("das posicoes de \n");
        lcd.printf("SOLTA \n");
    } else if (estado == 7) {
        lcd.printf("Aperte 'Confirma' para \n");
        lcd.printf("iniciar o ciclo\n");
        lcd.printf("automatico \n");    
    } else if (estado == 8) {
        lcd.printf("Voltando para \n");
        lcd.printf("Tela de Inicio \n");

    } else if (estado == 9) {
        lcd.printf("Iniciando ciclo\n");
        lcd.printf("automatico \n");
    } else if (estado == 10) {
        lcd.printf("Qts ml para a\n");
        lcd.printf("posicao: %3d\n", mililitros);
    }
}

// Referenciamento de cada um dos eixos, começando pelo Z, uma vez que foi a entrega da APS 5
void ReferenciamentoZ(int MotorAcionado){
    EnableMotorZ = 1;
    while(MotorAcionado == 1){
        if(SinalReferenciamentoZ == 0){
            StepDriver = !StepDriver;
            wait_ms(tempo);
            EnableMotorZ = 0;
            MotorZ.SentidoAnti();
        } else {
            Counter = 0;
            
            while(1){
                StepDriver = !StepDriver;
                wait_ms(tempo);
                MotorZ.SentidoHorario();
                Counter ++;
                if(Counter > 750){
                    EnableMotorZ = 1;
                    MotorAcionado = 3;
                    SinalReferenciamentoZ = 1;
                    break;
                }
            } 
        }
    }
}

void CheckInicioZ(){
    if(debounce.read_ms() > 200){
        if(SinalReferenciamentoZ == 0){
            SinalReferenciamentoZ ++;
        }
        printf("Sinal inicio do x: %d \r", SinalReferenciamentoZ);
    }
    debounce.reset();
}
    
void CheckFimZ(){
    if(debounce.read_ms() > 200){
        if(SinalReferenciamentoZ == 1){
            SinalReferenciamentoZ ++;
        }
        printf("Sinal fim do x : %d \r", SinalReferenciamentoZ);
    }
    
    debounce.reset();
}


void ReferenciamentoX(int MotorAcionado){
    while(MotorAcionado == 1){
        Counter ++;
        if(SinalReferenciamentoX == 0){
            StepDriver = !StepDriver;
            wait_ms(tempo);
            EnableMotorX = 0;
            MotorX.SentidoHorario();
            
        } else{
            Counter = 0;
            
            while(1){
                StepDriver = !StepDriver;
                wait_ms(tempo);
                MotorX.SentidoAnti();
                Counter ++;
                if(Counter > 750){
                    EnableMotorX = 1;
                    MotorAcionado = 2;
                    SinalReferenciamentoX = 2;
                    break;
                }   
            }
        }        
    }            
}

void CheckInicioX(){
    if(debounce.read_ms() > 200){
        if(SinalReferenciamentoX == 0){
            SinalReferenciamentoX ++;
        }
        printf("Sinal inicio do x: %d \r", SinalReferenciamentoX);
    }
    debounce.reset();
}
    
void CheckFimX(){
    if(debounce.read_ms() > 200){
        if(SinalReferenciamentoX == 1){
            SinalReferenciamentoX ++;
        }
        printf("Sinal fim do x : %d \r", SinalReferenciamentoX);
    }
    
    debounce.reset();
}

void ReferenciamentoY(int MotorAcionado){
    while(MotorAcionado == 1){        
        Counter ++;
        if(SinalReferenciamentoY == 0){
            StepDriver = !StepDriver;
            wait_ms(tempo);
            EnableMotorY = 0;
            MotorY.SentidoAnti();
            
        } else{
            Counter = 0;
            
            while(1){
                StepDriver = !StepDriver;
                wait_ms(tempo);
                MotorY.SentidoHorario();
                Counter ++;
                if(Counter > 400){
                    EnableMotorY = 1;
                    MotorAcionado = 3;
                    SinalReferenciamentoY = 1;
                    break;
                }   
                }
            }
            
                
    } 
}

void CheckInicioY(){
    if(debounce.read_ms() > 200){
        if(SinalReferenciamentoY == 0){
            SinalReferenciamentoY ++;
        }
        printf("Sinal inicio do x: %d \r", SinalReferenciamentoY);
    }
    debounce.reset();
}
    
void CheckFimY(){
    if(debounce.read_ms() > 200){
        if(SinalReferenciamentoY == 1){
            SinalReferenciamentoY ++;
        }
        printf("Sinal fim do x : %d \r", SinalReferenciamentoY);
    }
    
    debounce.reset();
}


