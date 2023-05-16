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

int SinalEMERGENCIA = 0;

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

int SinalJOGManual = 1;
int SinalJOGAutomatico = 1;
int SinalReferenciamentoX = 0;
int SinalReferenciamentoY = 0;
int SinalReferenciamentoZ = 0;



int eixo_x_finalizado = 0;
int eixo_y_finalizado = 0;

int execucao = 0;
int NSoltar = 1;
int conta_posicoes = 0;
int mililitros = 0;


Timer debounce;

void lcd_show(int state) {
    lcd.cls();

    if (state == 0) {
        lcd.printf("Olá! \n");
        lcd.printf("Pressione 'ENTER' \n");
        lcd.printf("para iniciar o \n");
        lcd.printf("referenciamento");
        execucao = 1;
    } else if (state == 1) {
        lcd.printf("Referenciamento \n");
        lcd.printf("Em \n");
        lcd.printf("Execucao... \n");
    } else if (state == 2) {
        lcd.printf("Referenciamento \n");
        lcd.printf("concluido \n");
        lcd.printf("Deseja selecionar a posicao de pega ? \n");
    } else if (state == 3) {
        lcd.printf("Botao de emergencia!! beep  \n");
    } else if (state == 4) {
        lcd.printf("X de pegar:%4d\n", PosicaoX);
        lcd.printf("Y de pegar:%4d\n", PosicaoY);
        lcd.printf("Z de pegar:%4d\n", PosicaoZ);
        lcd.printf("Pressione 'Continua'");
    } else if (state == 5) {
        lcd.printf("Qtd de pontos\n");
        lcd.printf("De despejo: %3d\n", NSoltar);
        lcd.printf("Pressione 'Confirma' \n");
        lcd.printf("Para confirmar");
    } else if (state == 6) {
        lcd.printf("Aperte 'Confirma' para \n");
        lcd.printf("iniciar a selecao\n");
        lcd.printf("das posicoes de \n");
        lcd.printf("SOLTA \n");
    } else if (state == 7) {
        lcd.printf("Aperte 'Confirma' para \n");
        lcd.printf("iniciar o ciclo\n");
        lcd.printf("automatico \n");    
    } else if (state == 8) {
        lcd.printf("Voltando para \n");
        lcd.printf("Tela de Inicio \n");

    } else if (state == 9) {
        lcd.printf("Iniciando ciclo\n");
        lcd.printf("automatico \n");
    } else if (state == 10) {
        lcd.printf("Qts ml para a\n");
        lcd.printf("posicao: %3d\n", mililitros);
    }
}

// Referenciamento de cada um dos eixos, começando pelo Z, uma vez que foi a entrega da APS 5 e portanto o primeiro a ser feito
// Função ReferenciamentoZ: Aciona um motorZ enquanto a variável MotorAcionado for igual a 1.
// Se o sinal de referenciamento Z for 0(não referenciado), altera o estado do driver de passo, aguarda um tempo especificado(2), desativa o motor Z e move o motor no sentido anti-horário.
// Caso contrário, inicia um laço infinito(while) que muda o estado do driver de passo, aguarda um tempo especificado(2), move o motor no sentido horário e incrementa um contador.
// Se o contador ultrapassar 750, ativa o motor Z, define MotorAcionado como 1, define SinalReferenciamentoZ como 1 e encerra o laço.
void ReferenciamentoZ(int MotorAcionado){
    EnableMotorZ = 1;
    while(MotorAcionado == 1){
        if(SinalReferenciamentoZ == 0){
            StepDriver =! StepDriver;
            wait_ms(tempo);
            EnableMotorZ = 0;
            MotorZ.definirDirecao(ANTIHORARIO);
        } else {
            Counter = 0;
            
            while(1){
                StepDriver =! StepDriver;
                wait_ms(tempo);
                MotorZ.definirDirecao(HORARIO);
                Counter ++;
                if(Counter > 750){
                    EnableMotorZ = 1;
                    MotorAcionado = 1;
                    SinalReferenciamentoZ = 1;
                    break;
                }
            } 
        }
    }
}

// Função para validar o início do referenciamento em Z
// Verifica se passaram 200ms desde a última mudança de estado. 
// Se sim, e se o sinal de referenciamento em x for 0, incrementa ao sinal.
// Em seguida, imprime o estado atual do sinal e reinicia o contador de tempo.
void CheckInicioZ(){
    if(debounce.read_ms() > 200){
        if(SinalReferenciamentoZ == 0){
            SinalReferenciamentoZ ++;
        }
        printf("Sinal inicio do x: %d \r", SinalReferenciamentoZ);
    }
    debounce.reset();
}

// Função para validar o fim do referenciamento em Z, mas verifica se o sinal de referenciamento em x é 1.
// Se sim, incrementa ao sinal. Em seguida, imprime o estado atual do sinal e reinicia o contador de tempo.
void CheckFimZ(){
    if(debounce.read_ms() > 200){
        if(SinalReferenciamentoZ == 1){
            SinalReferenciamentoZ ++;
        }
        printf("Sinal fim do x : %d \r", SinalReferenciamentoZ);
    }

    debounce.reset();
}

// Função ReferenciamentoX: Aciona um motorX enquanto a variável MotorAcionado for igual a 1.
// Se o sinal de referenciamento X for 0(não referenciado), altera o estado do driver de passo, aguarda um tempo especificado(2), desativa o motor X e move o motor no sentido horário.
// Caso contrário, inicia um laço infinito(while) que muda o estado do driver de passo, aguarda um tempo especificado(2), move o motor no sentido anti-horário e incrementa um contador.
// Se o contador ultrapassar 750, ativa o motor X, define MotorAcionado como 2, define SinalReferenciamentoX como 1 e encerra o laço.
void ReferenciamentoX(int MotorAcionado){
    while(MotorAcionado == 1){
        Counter ++;
        if(SinalReferenciamentoX == 0){
            StepDriver =! StepDriver;
            wait_ms(tempo);
            EnableMotorX = 0;
            MotorX.definirDirecao(HORARIO);
            
        } else{
            Counter = 0;
            
            while(1){
                StepDriver =! StepDriver;
                wait_ms(tempo);
                MotorX.definirDirecao(ANTIHORARIO);
                Counter ++;
                if(Counter > 750){
                    SinalReferenciamentoX = 2;
                    EnableMotorX = 1;
                    MotorAcionado = 2;
                    break;
                }   
            }
        }        
    }            
}

// Função para validar o início do referenciamento em X
// Verifica se passaram 200ms desde a última mudança de estado. 
// Se sim, e se o sinal de referenciamento em x for 0, incrementa ao sinal.
// Em seguida, imprime o estado atual do sinal e reinicia o contador de tempo.
void CheckInicioX(){
    if(debounce.read_ms() > 200){
        if(SinalReferenciamentoX == 0){
            SinalReferenciamentoX ++;
        }
        printf("Sinal inicio do x: %d \r", SinalReferenciamentoX);
    }
    debounce.reset();
}

// Função para validar o fim do referenciamento em X, mas verifica se o sinal de referenciamento em x é 1.
// Se sim, incrementa ao sinal. Em seguida, imprime o estado atual do sinal e reinicia o contador de tempo.
void CheckFimX(){
    if(debounce.read_ms() > 200){
        if(SinalReferenciamentoX == 1){
            SinalReferenciamentoX ++;
        }
        printf("Sinal fim do x : %d \r", SinalReferenciamentoX);
    }
    
    debounce.reset();
}

// Função ReferenciamentoY: Aciona um motorY enquanto a variável MotorAcionado for igual a 1.
// Se o sinal de referenciamento Y for 0(não referenciado), altera o estado do driver de passo, aguarda um tempo especificado(2), desativa o motor Y e move o motor no sentido anti-horário.
// Caso contrário, inicia um laço infinito(while) que muda o estado do driver de passo, aguarda um tempo especificado(2), move o motor no sentido horário e incrementa um contador.
// Se o contador ultrapassar 750, ativa o motor Y, define MotorAcionado como 3, define SinalReferenciamentoY como 1 e encerra o laço.
void ReferenciamentoY(int MotorAcionado){
    while(MotorAcionado == 1){        
        Counter ++;
        if(SinalReferenciamentoY == 0){
            StepDriver =! StepDriver;
            wait_ms(tempo);
            EnableMotorY = 0;
            MotorY.definirDirecao(ANTIHORARIO);
            
        } else{
            Counter = 0;
            
            while(1){
                StepDriver =! StepDriver;
                wait_ms(tempo);
                MotorY.definirDirecao(HORARIO);
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
// Função para validar o início do referenciamento em Y
// Verifica se passaram 200ms desde a última mudança de estado. 
// Se sim, e se o sinal de referenciamento em x for 0, incrementa ao sinal.
// Em seguida, imprime o estado atual do sinal e reinicia o contador de tempo.
void CheckInicioY(){
    if(debounce.read_ms() > 200){
        if(SinalReferenciamentoY == 0){
            SinalReferenciamentoY ++;
        }
        printf("Sinal inicio do x: %d \r", SinalReferenciamentoY);
    }
    debounce.reset();
}

// Função para validar o fim do referenciamento em Y, mas verifica se o sinal de referenciamento em x é 1.
// Se sim, incrementa ao sinal. Em seguida, imprime o estado atual do sinal e reinicia o contador de tempo.

void CheckFimY(){
    if(debounce.read_ms() > 200){
        if(SinalReferenciamentoY == 1){
            SinalReferenciamentoY ++;
        }
        printf("Sinal fim do x : %d \r", SinalReferenciamentoY);
    }
    
    debounce.reset();
}



// A função atualizarEstadoSinalJOGManual é responsável por atualizar o estado do SinalJOGManual, que é usado para indicar o eixo atual.
// Ela verifica se mais de 150 milissegundos se passaram desde a última vez que o estado foi alterado (usando o objeto debounce).
// Se esse for o caso, o estado é atualizado para o novo valor fornecido e o tempo é resetado (usando debounce.reset()).
// A nova situação do SinalJOGManual é então impressa na saída padrão.

void atualizarEstadoSinalJOGManual(int novoEstado) {
    if(debounce.read_ms() > 150){
        SinalJOGManual = novoEstado;
        printf("Estado SinalJOGManual: %d \n \r", SinalJOGManual);
        debounce.reset();
    }
}
// A função alterarParaEixoX é responsável por mudar o estado do SinalJOGManual para 1, que representa o eixo X.
// Ela chama a função atualizarEstadoSinalJOGManual com o novo estado como argumento.
void alterarParaEixoX() {
    atualizarEstadoSinalJOGManual(1);
}
// A função alterarParaEixoY é responsável por mudar o estado do SinalJOGManual para 2, que representa o eixo Y.
// Ela chama a função atualizarEstadoSinalJOGManual com o novo estado como argumento.
void alterarParaEixoY() {
    atualizarEstadoSinalJOGManual(2);
}
// A função alterarParaEixoZ é responsável por mudar o estado do SinalJOGManual para 3, que representa o eixo Z.
// Ela chama a função atualizarEstadoSinalJOGManual com o novo estado como argumento.
void alterarParaEixoZ() {
    atualizarEstadoSinalJOGManual(3);
}


// A função aguardarAjusteMililitros é responsável por ajustar a quantidade de mililitros com base na leitura do joystick.
// Ela recebe um ponteiro para a variável mililitros, o valor atual da leitura do joystick e um valor de incremento.
// Enquanto o valor da leitura do joystick estiver abaixo de 300 ou acima de 600, a função continuará em loop, ajustando a quantidade de mililitros.
// O ajuste é feito adicionando o valor do incremento à variável mililitros a cada 0.3 segundos.
// Se a quantidade de mililitros for menor do que zero, ela é ajustada para zero.

void aguardarAjusteMililitros(int* mililitros, double valorLeituraJoyStick, int incremento) {
    while(valorLeituraJoyStick < 300 || valorLeituraJoyStick > 600){
        wait(0.3);
        valorLeituraJoyStick = JoyStick.read() * 1000;
        *mililitros += incremento;
        printf("Mililitros: %d \n \r", *mililitros);
        if(*mililitros < 0) *mililitros = 0;
    }
}


// A função calcularMililitrosDespejo é a principal função para calcular a quantidade de mililitros a serem despejados.
// Ela inicia limpando o display do LCD e exibindo o valor inicial de mililitros (1).
// Em seguida, entra em um loop infinito, onde lê o valor do joystick e exibe a quantidade atual de mililitros no LCD.
// Se a leitura do joystick for menor que 300, a função aguardarAjusteMililitros é chamada para diminuir a quantidade de mililitros.
// Se a leitura do joystick for maior que 600, a função aguardarAjusteMililitros é chamada para aumentar a quantidade de mililitros.
// Quando a variável prosseguir for verdadeira, a função retorna a quantidade atual de mililitros e sai do loop.

int calcularMililitrosDespejo() {
    lcd.cls();
    lcd_show(10);
    int mililitros = 1;
    bool prosseguir = false;
    double valorLeituraJoyStick;

    while(true){
        valorLeituraJoyStick = JoyStick.read() * 1000;
        lcd.locate(0, 1);
        lcd.printf("para a posicao: %3d\n", mililitros);

        if(valorLeituraJoyStick < 300) {
            aguardarAjusteMililitros(&mililitros, valorLeituraJoyStick, -1);
        } else if(valorLeituraJoyStick > 600) {
            aguardarAjusteMililitros(&mililitros, valorLeituraJoyStick, 1);
        }

        if(prosseguir) {
            return mililitros;
        }
    }   
}
