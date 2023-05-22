#include "mbed.h"
#include "Motor.h"
#include "TextLCD.h"
#include "mbed2/299/drivers/InterruptIn.h"


I2C i2c_lcd(I2C_SDA, I2C_SCL);
TextLCD_I2C lcd(&i2c_lcd, 0x27, TextLCD::LCD20x4);

// DECLARANDO BOTOES DE INTERAÇÃO
InterruptIn botaoZ(D13);
DigitalIn botaoMENOSML(D11);
DigitalIn botaoMAISML(PB_13);
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
bool prosseguir = false;
int Counter = 0;
int ValorInicialX = 0;
int ValorFinalX = 100000;
int PosicaoX;
int CounterAutoZ;
int CounterAutoX;
int CounterAutoY;
int ValorInicialZ = 0;
int ValorFinalZ = 100000;
int PosicaoZ;
int ValorInicialY = 0;
int ValorFinalY = 100000;
int PosicaoY;
int etapa = 2;
int Referenciamento_Done = 0;
int ContaIndex;
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



int SinalFinalizadoX = 0;
int SinalFinalizadoY = 0;

int exec = 0;
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
        exec = 1;
    } else if (state == 1) {
        lcd.printf("Referenciamento \n");
        lcd.printf("Em \n");
        lcd.printf("exec... \n");
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
    if(debounce.read_ms() > 200){
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


// 
// 
// 
// 
// 

void aguardarAjusteMililitros(int* mililitros, int incremento) {
    lcd.cls();
    lcd.locate(0, 1);
    lcd.printf("Mililitros: %d\n", *mililitros);

    if(!(incremento < 0 && *mililitros == 0)) {
        *mililitros += incremento;
    }

    printf("Mililitros: %d \n \r", *mililitros);
}


// 
// 
// 
// 
// 
// 
int calcularMililitrosDespejo() {
    lcd.cls();
    lcd_show(10);
    int mililitros = 1;
    bool prosseguir = false;

    while(!prosseguir) {
        lcd.locate(0, 1);
        lcd.printf("para a posicao: %3d\n", mililitros);

        if(botaoMENOSML.read() == 1) {
            aguardarAjusteMililitros(&mililitros, -1);
        } else if(botaoMAISML.read() == 1) {
            aguardarAjusteMililitros(&mililitros, 1);
        }

        if(botaoCONFIRMACAO.read() == 1) {
            prosseguir = true;
        }
    }

    return mililitros;  
}


/*
 * A função JOGManual é responsável por controlar a operação da maquina de pipetagem 
 * em modo manual. Ela recebe um índice como argumento que é utilizado para
 * determinar o número de execuções do ciclo de controle manual.
 *
 * Quando a variável global 'exec' é maior ou igual a 7, o ciclo de controle manual 
 * começa. O usuário é então solicitado a selecionar uma posição para o dispositivo.
 *
 * O ciclo de controle manual é repetido até que o número de execuções seja igual ao 
 * índice fornecido. Em cada ciclo, o LCD mostra a posição atual.
 *
 */

void JOGManual(int index){

    int ContaIndex = 0;
    int UltimoX = 0;
    int UltimoY = 0;
    int CounterX = 0;
    int CounterY = 0;
    int CounterZ = 0;

    if(exec >= 7){
        ContaIndex = 1;
        lcd.cls();
        lcd.printf("Selecione a \n");
        lcd.printf("posicao[XYZ] %3d \n", ContaIndex);
        wait(3);
    }

    if(exec >= 7){
        while(ContaIndex <= index){
            lcd_show(4);

            // JOG MANUAL NO EIXO X
            /* Durante o JOG manual no eixo X:
            * - A variável CounterX é inicialmente definida como a posição atual no eixo X.
            * - Enquanto a variável global 'SinalJOGManual' for 1, o programa entra em um loop, 
            *   lendo a entrada do joystick e atualizando a posição no eixo X.
            * - Se a entrada do joystick for maior que 700 e 'SinalJOGManual' ainda for 1, o 
            *   programa verifica se CounterX já atingiu 'ValorFinalX'. Se sim, o motor é habilitado 
            *   e CounterX é fixado em 'ValorFinalX'. Se não, o motor é ativado na direção anti-horária 
            *   e CounterX é incrementado.
            * - Se a entrada do joystick for menor que 300 e 'SinalJOGManual' ainda for 1, o 
            *   programa verifica se CounterX já atingiu 'ValorInicialX'. Se sim, o motor é habilitado 
            *   e CounterX é fixado em 'ValorInicialX'. Se não, o motor é ativado na direção horária 
            *   e CounterX é decrementado.
            * - Finalmente, a posição atual no eixo X é atualizada para o valor de CounterX e os motores 
            *   X e Y são habilitados.
            */
            CounterX = PosicaoX;
            while(SinalJOGManual == 1){
                int y = JoyStick.read() * 1000;
                lcd.locate(0, 0);
                lcd.printf("X coletagem: %4d \n", PosicaoX);
                while(y > 700 && SinalJOGManual == 1){
                    y = JoyStick.read() * 1000;
                    if(CounterX >= ValorFinalX){
                        EnableMotorX = 1;
                        CounterX = ValorFinalX;
                    } else {
                        StepDriver = !StepDriver;
                        wait_ms(tempo);
                        EnableMotorX = 0;
                        MotorX.definirDirecao(ANTIHORARIO);
                        CounterX --;
                    }              
                }
                while(y < 300 && SinalJOGManual == 1){
                    y = JoyStick.read() * 1000;
                    if(CounterX <= ValorInicialX){
                        EnableMotorX = 1;
                        CounterX = ValorInicialX;
                    } else {
                        StepDriver = !StepDriver;
                        wait_ms(tempo);
                        EnableMotorX = 0;
                        MotorX.definirDirecao(HORARIO);
                        CounterX ++;
                    }
                }
                PosicaoX = CounterX;
                EnableMotorX = 1;
                EnableMotorY = 1;
            } 


            CounterY = PosicaoY;
            while(SinalJOGManual == 2){
                int y = JoyStick.read() * 1000;
                lcd.locate(0, 1);
                lcd.printf("Y coletagem: %4d \n", PosicaoY);
                while(y > 700 && SinalJOGManual == 2){
                    y = JoyStick.read() * 1000;
                    if(CounterY >= ValorFinalY){
                        EnableMotorY = 1;
                        CounterY = ValorFinalY;
                    } else {
                        StepDriver = !StepDriver;
                        wait_ms(tempo);
                        EnableMotorY = 0;
                        MotorY.definirDirecao(HORARIO);
                        CounterY ++;
                    }              
                }
                while(y < 300 && SinalJOGManual == 2){
                    y = JoyStick.read() * 1000;
                    if(CounterY <= ValorInicialY){
                        EnableMotorY = 1;
                        CounterY = ValorInicialY;
                    } else {
                        StepDriver = !StepDriver;
                        wait_ms(tempo);
                        EnableMotorY = 0;
                        MotorX.definirDirecao(ANTIHORARIO);
                        CounterY --;
                    }
                }
                PosicaoY = CounterY;
                EnableMotorX = 1;
                EnableMotorY = 1;
            }

            CounterZ = PosicaoZ;
            while(SinalJOGManual == 3){
                int y = JoyStick.read() * 1000;
                lcd.locate(0, 2);
                lcd.printf("Z coletagem: %4d \n", PosicaoZ);
                while(y > 700 && SinalJOGManual == 3){
                    y = JoyStick.read() * 1000;
                    if(CounterZ >= ValorFinalZ){
                        EnableMotorZ = 1;
                        CounterZ = ValorFinalZ;
                    } else {
                        StepDriver = !StepDriver;
                        wait_ms(tempo);
                        EnableMotorZ = 0;
                        MotorZ.definirDirecao(HORARIO);
                        CounterZ ++;
                    }              
                }
                while(y < 300 && SinalJOGManual == 2){
                    y = JoyStick.read() * 1000;
                    if(CounterZ <= ValorInicialZ){
                        EnableMotorZ = 1;
                        CounterZ = ValorInicialZ;
                    } else {
                        StepDriver = !StepDriver;
                        wait_ms(tempo);
                        EnableMotorZ = 0;
                        MotorX.definirDirecao(ANTIHORARIO);
                        CounterZ --;
                    }
                }
                 PosicaoZ = CounterZ;
                 EnableMotorX = 1;
                 EnableMotorY = 1;
                 EnableMotorZ = 1;
            }


                while(SinalJOGManual == 4){
                mililitros = calcularMililitrosDespejo();
                listaPosX[ContaIndex] = PosicaoX;
                listaPosY[ContaIndex] = PosicaoY;
                listaPosZ[ContaIndex] = PosicaoZ;
                UltimoX =  PosicaoX;
                UltimoY = PosicaoY;
                UltimoZ = PosicaoZ;
                printf("Salvo x: %d \n \r", listaPosX[ContaIndex]);
                printf("Salvo Y: %d \n \r", listaPosX[ContaIndex]);
                printf("Salvo Z: %d \n \r", listaPosX[ContaIndex]);
                printf("Valor counter %d \n \r", ContaIndex);
                printf("Valor indice %d \n \r", index);
                ContaIndex += 1;
                if(exec >= 7){
                    if(ContaIndex > index){
                        SinalJOGManual = 5;
                        break;
                    } else {
                        SinalJOGManual = 1;
                        lcd.cls();
                        lcd.printf("Agora selecione a posicao %3d \n", ContaIndex);
                        printf("Selecione a proX pos \n \r");
                        wait(2);
                        break;
                    }
                } else {
                    if(ContaIndex >= index){
                        SinalJOGManual = 5;
                        break;
                    } else {
                        SinalJOGManual = 1;
                        lcd.cls();
                        lcd.printf("Agora selecione a posicao %3d \n", ContaIndex);
                        printf("Selecione a prox pos \n \r");
                        wait(2);
                        break;
                    }
                }
                
            }

            if(SinalJOGManual == 5){
                EnableMotorX = 1;
                EnableMotorY = 1;
                EnableMotorZ = 1; 
                printf("JogManual finalizado \n \r");
                break;
            }


            EnableMotorX = 1;
            EnableMotorY = 1;
            EnableMotorZ = 1;

        }
        while(ContaIndex <= index) {        
        lcd_show(4);
        CounterX = PosicaoX;

        while(SinalJOGManual == 1){
            y = JoyStick.read() * 1000;
            lcd.locate(0, 0);
            lcd.printf("X de coleta:%4d\n", PosicaoX);

            if(y > 600 && SinalJOGManual == 1 && CounterX < ValorFinalX){
                StepDriver = !StepDriver;
                wait_ms(tempo);
                EnableMotorX = 0;
                MotorX.definirDirecao(ANTIHORARIO);
                CounterX++;
            } else if(y < 300 && SinalJOGManual == 1 && CounterX > ValorInicialX){
                StepDriver = !StepDriver;
                wait_ms(tempo);
                EnableMotorX = 0;
                MotorX.definirDirecao(HORARIO);
                CounterX--;
            } else {
                EnableMotorX = 1;
            }

            PosicaoX = CounterX;
            EnableMotorX = 1;
            EnableMotorY = 1;
        }

        }

        //EIXO Y
        CounterY = PosicaoY;
        while(SinalJOGManual == 2){
            y = JoyStick.read() * 1000;
            lcd.locate(0, 1);
            lcd.printf("Y de coleta:%4d\n", PosicaoY);

            if(y > 600 && SinalJOGManual == 2 && CounterY < ValorFinalY){
                StepDriver = !StepDriver;
                wait_ms(tempo);
                EnableMotorY = 0;
                MotorY.definirDirecao(HORARIO);
                CounterX--;
            } else if(y < 300 && SinalJOGManual == 2 && CounterY > ValorInicialY){
                StepDriver = !StepDriver;
                wait_ms(tempo);
                EnableMotorY = 0;
                MotorY.definirDirecao(ANTIHORARIO);
                CounterY++;
            } else {
                EnableMotorY = 1;
            }

            PosicaoY = CounterY;
            EnableMotorX = 1;
            EnableMotorY = 1;  
        }

    }
          //EIXO Z
        CounterZ = PosicaoZ;
        while(SinalJOGManual == 3){
            y = JoyStick.read() * 1000;
            lcd.locate(0, 2);
            lcd.printf("Z de coleta:%4d\n", PosicaoZ);

            if(y > 600 && SinalJOGManual == 2 && CounterZ < ValorFinalZ){
                StepDriver = !StepDriver;
                wait_ms(tempo);
                EnableMotorZ = 0;
                MotorZ.definirDirecao(HORARIO);
                CounterZ++;
            } else if(y < 300 && SinalJOGManual == 2 && CounterZ > ValorInicialZ){
                StepDriver = !StepDriver;
                wait_ms(tempo);
                EnableMotorZ = 0;
                MotorZ.definirDirecao(ANTIHORARIO);
                CounterZ--;
            } else {
                EnableMotorZ = 1;
            }

            PosicaoZ = CounterZ;
            EnableMotorX = 1;
            EnableMotorY = 1;
            EnableMotorZ = 1;
        }    


       while(SinalJOGManual == 4) {
            listaPosX[ContaIndex] = PosicaoX;
            listaPosY[ContaIndex] = PosicaoY;
            listaPosZ[ContaIndex] = PosicaoZ;
            UltimoX = PosicaoX;
            UltimoY = PosicaoY;
            UltimoZ = PosicaoZ;

            printf("Posicao X: %d \n \r", listaPosX[ContaIndex]);
            printf("Posicao Y: %d \n \r", listaPosY[ContaIndex]);
            printf("Posicao Z: %d \n \r", listaPosZ[ContaIndex]);
            printf("Valor counter %d \n \r", ContaIndex);
            printf("Valor do index %d \n \r", index);
            
            ContaIndex ++;

            // Handling ContaIndex based on exec value
            if(exec >= 7 || ContaIndex >= index){
                SinalJOGManual = (ContaIndex > index) ? 5 : 1;
            } else {
                SinalJOGManual = (ContaIndex > index) ? 5 : 1;
            }

            if (SinalJOGManual == 1) {
                lcd.cls();
                lcd.printf("Selecione a posicao %3d \n", ContaIndex);
                printf("Selecione next pos \n \r");
                wait(2);
            }

            if(SinalJOGManual == 5) {
                EnableMotorX = 1;
                EnableMotorY = 1;
                EnableMotorZ = 1;
                printf("JogManual finalizado \n \r");
                break;
            }

            EnableMotorX = 1;
            EnableMotorY = 1;
            EnableMotorZ = 1;
        }           
    }



void JogAutomaticoXY(int AlvoX, int AlvoY) {
    CounterAutoX = UltimoX;
    CounterAutoY = UltimoY;

    printf("CounterX %d \n \r", CounterAutoX);
    printf("CounterY %d \n \r", CounterAutoY);
    printf("Alvo atual X %d \n \r", AlvoX);
    printf("Alvo atual Y %d \n \r", AlvoY);

    SinalFinalizadoX = 0;
    SinalFinalizadoY = 0;

    while(!(SinalFinalizadoX && SinalFinalizadoY)) {
        StepDriver = !StepDriver;
        wait_ms(tempo);

        if(CounterAutoX > AlvoX){            
            EnableMotorX = 0;
            MotorX.definirDirecao(HORARIO);
            CounterAutoX--;
        } else if(CounterAutoX < AlvoX){
            EnableMotorX = 0;
            MotorX.definirDirecao(ANTIHORARIO);
            CounterAutoX++;
        } else {
            EnableMotorX = 1;
            UltimoX = CounterAutoX;
            SinalFinalizadoX = 1;
        }

        if(CounterAutoY > AlvoY){
            EnableMotorY = 0;
            MotorY.definirDirecao(ANTIHORARIO);
            CounterAutoY--;
        } else if(CounterAutoY < AlvoY){
            EnableMotorY = 0;
            MotorY.definirDirecao(HORARIO);
            CounterAutoY++;
        } else {
            EnableMotorY = 1;
            UltimoY = CounterAutoY;
            SinalFinalizadoY = 1;
        }
    }
}


void jog_automatico_y(int ValorAlvo) {
    CounterAutoY = UltimoY;
    printf("CounterY %d \n \r", CounterAutoY);

    while(CounterAutoY != ValorAlvo) {
        StepDriver = !StepDriver;
        wait_ms(tempo);
        
        if(CounterAutoY > ValorAlvo){
            EnableMotorY = 0;
            MotorY.definirDirecao(ANTIHORARIO);
            CounterAutoY--;
        } else if(CounterAutoY < ValorAlvo){
            EnableMotorY = 0;
            MotorY.definirDirecao(HORARIO);
            CounterAutoY++;
        }
    }
    
    printf("Posicionado no Alvo Y \n \r");
    EnableMotorY = 1;
    UltimoY = CounterAutoY;
}

void jog_automatico_z(int ValorAlvo) {
    CounterAutoZ = UltimoZ;
    EnableMotorX = 1;
    EnableMotorY = 1;

    printf("CounterZ %d \n \r", CounterAutoZ);

    while(CounterAutoZ != ValorAlvo) {
        StepDriver = !StepDriver;
        wait_ms(tempo);
        
        if(CounterAutoZ > ValorAlvo){
            EnableMotorZ = 0;
            MotorZ.definirDirecao(ANTIHORARIO);
            CounterAutoZ--;
        } else if(CounterAutoZ < ValorAlvo){
            EnableMotorZ = 0;
            MotorZ.definirDirecao(HORARIO);
            CounterAutoZ++;
        }
    }

    printf("Posicionado no Alvo Z \n \r");
    EnableMotorZ = 1;
    UltimoZ = CounterAutoZ;
}

void LidaConfirma(){
    if(debounce.read_ms() > 500){ 
        exec += 1;   
        printf("Var exec %d \n \r", exec);
    }
    prosseguir = true;
    wait(2);
    debounce.reset(); 
}

void LidarVoltar(){
    if(debounce.read_ms() > 150){
        
        if(exec == 3 || exec == 4) {
            SinalReferenciamentoX = 0;
            SinalReferenciamentoY = 0;
            SinalReferenciamentoZ = 0;
            exec = 0;
        } else if(exec == 7 || exec == 8){
            exec = 3;
        } else if (exec > 9){
            exec = 7;
        }
        printf("Valor do exec %d \n \r", exec);

    }
    debounce.reset();
}

void LidaEmergencia(){
    // reset_motors = 0;
    EnableMotorX = 1;
    EnableMotorY = 1;
    EnableMotorZ = 1;
    NVIC_SystemReset();
    
}

int seleciona_pontos_de_solta() {
    lcd.cls();
    lcd_show(5);

    while(exec != 7) {
        y = JoyStick.read() * 1000;
        lcd.locate(0, 1);
        lcd.printf("De despejo: %3d\n", NSoltar);
        
        while(y < 300 && NSoltar < 9){
            wait(0.3);
            y = JoyStick.read() * 1000;
            NSoltar++;
            printf("Num soltas: %d \n \r", NSoltar);
        }

        while(y > 600 && NSoltar > 1){
            wait(0.3);
            y = JoyStick.read() * 1000;
            NSoltar--;
            printf("Num soltas: %d \n \r", NSoltar);
        }

        if(NSoltar >= 9) {
            NSoltar = 9;
            printf("Num de soltas: %d \n \r", NSoltar);
        }

        if(NSoltar <= 1) {
            NSoltar = 1;
            printf("Num soltas: %d \n \r", NSoltar);
        }
    }
}


int main() {
    
    debounce.start();
    FimDeCursoY1.rise(&CheckInicioY);
    FimDeCursoY2.rise(&CheckFimY);
    FimDeCursoZ1.rise(&CheckInicioZ);
    FimDeCursoZ2.rise(&CheckFimZ);
    FimDeCursoX1.rise(&CheckFimX);
    FimDeCursoX2.rise(&CheckInicioX);
 // código para identificar mudança do uso do eixo x pro eixo y
    botaoZ.rise(&alterarParaEixoZ);
    botaoCONFIRMACAO.rise(&LidaConfirma);
    botaoVOLTAR.rise(&LidarVoltar);
    botaoEMERGENCIA.fall(&LidaEmergencia);

    //reset_motors = 1;
    EnableMotorX = 1;
    EnableMotorY = 1;
    EnableMotorZ = 1;
    StepDriver = 1;
    MotorX.definirDirecao(HORARIO);
    MotorY.definirDirecao(HORARIO);
    MotorZ.definirDirecao(HORARIO);
    
    AcionamentoPipeta = 1;

    lcd.setBacklight(TextLCD::LightOn);
    lcd.setCursor(TextLCD::CurOff_BlkOn);

    if(botaoEMERGENCIA == 0){
        lcd_show(3);
        EnableMotorX = 1;
        EnableMotorY = 1;
        EnableMotorZ = 1;
        StepDriver = 0;
        while(botaoEMERGENCIA == 0){}

    }

    while(1){
        EnableMotorX = 1;
        EnableMotorY = 1;
        EnableMotorZ = 1;
        if(exec == 0){
            lcd_show(0);
            exec = 1;
        } 
        else if(exec == 2){
            lcd_show(1);
            ReferenciamentoX(1);
            ReferenciamentoY(1);
            ReferenciamentoZ(1);
            exec = 3;
        }
        else if(exec == 3){
            lcd_show(2);
            EnableMotorX = 1;
            EnableMotorY = 1;
            EnableMotorZ = 1;
            StepDriver = 0;
            exec = 4;
        }
        else if(exec == 5){
            JOGManual(1);
            exec = 6;
        } 
        else if(exec == 6){
            seleciona_pontos_de_solta();
        }
        else if(exec == 7){
            lcd_show(6);
            exec = 8;
        }
        else if(exec == 9){
            SinalJOGManual = 1;
            printf("N de posicoes de solta %d \n \r", NSoltar+1);
            JOGManual(NSoltar);
            etapa = 9 + NSoltar;
            exec = etapa;
        }
        else if(exec == etapa){
            EnableMotorX = 1;
            EnableMotorY = 1;
            EnableMotorZ = 1;
            StepDriver = 1;
            lcd_show(7);
            prosseguir = 0;
            exec = etapa + 1;
        }
        else if(exec == etapa + 1 && prosseguir == 1){
            lcd_show(8);
            jog_automatico_z(ValorInicialZ);
            exec = etapa + 2;
        }
        else if(exec == etapa + 2){
            exec = 14;
            lcd_show(9);
            for(int i = 1; i <= NSoltar; i++){
                lcd.cls();
                lcd.locate(0,0);
                lcd.printf("Ciclo para posicao %d \n", i);

                printf("Ciclo para posicao %d \n \r", i);
                wait(1);
                Counter = 0;
                lcd.locate(0,1);
                lcd.printf("Iteracao numero %d \n", Counter);
                
                if(i >= 0){
                    mililitros = lista_ml[i];
                    while(Counter < mililitros){
                        lcd.locate(0,1);
                        lcd.printf("Iteracao numero %d \n", Counter);
                        printf("Iteração numero %d \n \r", Counter);
                        Jog(ValorInicialZ);
                        JogAutomaticoXY(listaPosX[0], listaPosY[0]);
                        jog_automatico_z(listaPosZ[0]);
                        AcionamentoPipeta = 0;
                        wait(2);
                        AcionamentoPipeta = 1;
                        wait(4);
                        // wait(5);
                        jog_automatico_z(ValorInicialZ);
                        
                        JogAutomaticoXY(listaPosX[i], listaPosY[i]);
                        jog_automatico_z(listaPosZ[i]);

                        AcionamentoPipeta = 0;
                        wait(2);
                        AcionamentoPipeta = 1;
                        wait(4);
                        Counter ++;
                    }
                }             
            }
            jog_automatico_z(ValorInicialZ);
            JogAutomaticoXY(ValorInicialX, ValorInicialY);
            lcd.cls();
            lcd.printf("Ciclos finalizados");
            EnableMotorX = 1;
            EnableMotorY = 1;
            EnableMotorZ = 1;
        }
        else if(exec == 14){
            exec = etapa;
        }
            
        
    }
    
}

