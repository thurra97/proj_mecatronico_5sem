#include "mbed.h"
#include "Motor.h"
#include "TextLCD.h"



//DECLARANDO AS ENTRADAS DO MODULO I2C QUE CONTROLA O LCD 20X4 UTILIZADO NA IHM
I2C i2c(PB_9, PB_8);
TextLCD_I2C lcd(&i2c, 0x4e, TextLCD::LCD20x4);

// DECLARANDO BOTOES DE INTERAÇÃO
InterruptIn botaoMENOSML(PA_12);
InterruptIn botaoMAISML(PC_5);
InterruptIn botaoCONFIRMACAO(PC_8);
InterruptIn botaoVOLTAR(PC_6); 
InterruptIn botaoUSER(PC_13);

// BOTAO DE EMERGENCIA
InterruptIn botaoEMERGENCIA(PD_2);

// DECLARANDO TODOS OS FIM DE CURSO(EIXOS X, Y E Z)
InterruptIn FimDeCursoX1(PB_5);
InterruptIn FimDeCursoX2 (PB_3);
InterruptIn FimDeCursoY1(PB_10); //FDC(Y) = NF = GND    , QUANDO ABERTO = 3,3V
InterruptIn FimDeCursoY2(PB_4);
InterruptIn FimDeCursoZ1(PA_6);
InterruptIn FimDeCursoZ2(PA_7);
DigitalOut led(LED1);

//DECLARANDO JOYSTICK E BOTAO JOYSTICK
AnalogIn EixoXJoyStick(PC_2);
AnalogIn EixoYJoyStick(PC_3);
InterruptIn botaoPOS(PB_6);


// DECLARANDO MOTORES
Motor MotorX(PC_11);
Motor MotorY(PA_15);
Motor MotorZ(PA_9);

// DECLARANDO ENABLES
DigitalOut EnableMotorX(PC_10);
DigitalOut EnableMotorY(PB_7);
DigitalOut EnableMotorZ(PA_8);

// DECLARANDO PIPETA/RELE
DigitalOut AcionamentoPipeta(PB_2);

// DECLARANDO STEP/ CLOCK
DigitalOut StepDriverXY(PC_12);
DigitalOut StepDriverZ(PC_7);


// VARIAVEIS UTILIZADAS NAS FUNCOES
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
int ValorFinalX = 1000000;
int PosicaoX;
int CounterAutoZ;
int CounterAutoX;
int CounterAutoY;
int ValorInicialZ = 0;
int ValorFinalZ = 1000000;
int PosicaoZ;
int ValorInicialY = 0;
int ValorFinalY = 1000000;
int PosicaoY;
int etapa = 2;
int ContaIndex;
int UltimoX;
int UltimoY;
int UltimoZ;
int tamanho;
float y;
float x;
float z;

//LISTAS DE ARMAZENAMENTO DAS POSICOES DE PEGA/SOLTA E MILILITROS
int listaPosX[10];
int listaPosZ[10];
int listaPosY[10];
int lista_ml[10];

//VARIAVEIS UTILIZADAS NAS FUNCOES DE MILILITROS/DESEPEJAR/PEGAR
int NSoltar = 1;
int mililitros = 0;

// FLAGS UTILIZADAS NAS FUNCOES
bool SinalJOGManual_Z = false;
int SinalJOGAutomatico = 1;
bool SinalReferenciamentoX = 0;
bool SinalReferenciamentoY = 0;
bool SinalReferenciamentoZ = 0;
int SinalFinalizadoX = 0;
int SinalFinalizadoY = 0;
volatile int SinalJOGManual = 1; // Global variable



// VARIAVEIS QUE CONTROLAM A CHAMDA DAS FUNCOES E IHM NA FUNCAO MAIN
int exec = 0;
int etapa_real = 100;


Timer debounce;

//FUNCAO QUE ESTABELECE AS TELAS DA IHM A DEPENDER DO EXEC NO QUAL ESTA INSERIDO NA FUNCAO MAIN
void lcd_show(int state) {
    switch(state) {
        case 0:
        lcd.cls();
        lcd.locate(0,0);
        lcd.printf("Ola \n");
        lcd.printf("Pressione 'CNFRM' \n");
        lcd.printf("para iniciar o \n");
        lcd.printf("referenciamento");
        break;
        
        case 1: 
        lcd.cls();
        lcd.printf("Referenciamento \n");
        lcd.printf("Em \n");
        lcd.printf("exec... \n");
        break;

        case 2:
        lcd.cls();
        lcd.printf("Referenciamento \n");
        lcd.printf("concluido \n");
        lcd.printf("Deseja selecionar a posicao de pega? \n");
        break;

        case 3:
        lcd.cls();
        lcd.printf("Botao de emergencia  \n");
        lcd.printf("Travamento   \n");
        lcd.printf("Automatico   \n");
        break;

        case 4:
        lcd.cls();
        lcd.printf("Coleta X:%4d\n", PosicaoX);
        lcd.printf("Coleta Y:%4d\n", PosicaoZ);
        lcd.printf("Coleta Z:%4d\n", PosicaoY);
        // lcd.printf("Pressione 'CNFRM'");
        break;

        case 5:
        lcd.cls();
        lcd.printf("Qtd de pontos\n");
        lcd.printf("De despejo: %3d\n", NSoltar);
        lcd.printf("Pressione 'CNFRM' \n");

 
        break;

        case 6:
        lcd.cls();
        lcd.printf("Aperte 'CNFRM' p \n");
        lcd.printf("iniciar selecao\n");
        lcd.printf("das pos de \n");
        lcd.printf("SOLTA \n");
        break;

        case 7:
        lcd.cls();
        lcd.printf("Aperte 'CNFRM' para \n");
        lcd.printf("iniciar o ciclo\n");
        lcd.printf("automatico \n");
        break;

        case 8:
        lcd.cls();
        lcd.printf("Voltando para \n");
        lcd.printf("Menu \n");
        break;

        case 9:
        lcd.cls();
        lcd.printf("Iniciando ciclo\n");
        lcd.printf("automatico \n");
        break;

        case 10:
        lcd.cls();
        lcd.printf("Qts ml para a\n");
        lcd.printf("posicao: %3d\n", mililitros);
        break;
        
        case 11:
        lcd.cls();
        lcd.printf("Aperte 'CNFRM' p \n");
        lcd.printf("iniciar selecao\n");
        lcd.printf("das pos de \n");
        lcd.printf("SOLTA \n");
        break;

    }
}

//FUNCAO DE REFRENCIAMENTO DO EIXO Z
void ReferenciamentoZ(int MotorAcionado){
    EnableMotorZ = 1; // Desabilita o motor
    while(MotorAcionado == 1){ // Enquanto o motor Z estiver acionado
        if(SinalReferenciamentoZ == 0){ // Se o sinal de referenciamento estiver desligado
            if(FimDeCursoZ1 == 0){ // Se o motor atingir o fim do curso Z1
                SinalReferenciamentoZ = 1;  // Ativa o sinal de referenciamento de Z = 1
                break;  // Sai do loop
            }
            StepDriverZ =! StepDriverZ; // Inverte o estado do driver de passo infinitamente
            wait_ms(tempo); // Espera tempo
            EnableMotorZ = 0; // Habilita o motor
            MotorZ.definirDirecao(HORARIO); // Define a direção do motor para o sentido horário

        } else {
            Counter = 0; // Reseta o contador
            
            while(1){ // Loop infinito
                StepDriverZ =! StepDriverZ; // Inverte o estado do driver de passo
                wait_ms(tempo); // Espera um período especificado
                MotorZ.definirDirecao(ANTIHORARIO); // Define a direção do motor para o sentido anti-horário
                Counter +=1; // Incrementa o contador
                if(Counter >= 200){ // Se o contador atingir 200
                    EnableMotorZ = 1; // Desabilita o motor
                    MotorAcionado = 1; // Define que o motor Z está acionado
                    SinalReferenciamentoZ = 1; // Ativa o sinal de referenciamento Z = 1
                    break; // Sai do loop
                }
            } 
        }
    }
}



//FUNCAO QUE COLOCA DEBOUNCE PARA ALTERAR O VALOR DA VARIAVEL SINALREFERENCIAMENTOZ
void CheckInicioZ(){
    if(debounce.read_ms() > 50){  // Se a leitura do tempo de debounce for maior que 50ms
        if(SinalReferenciamentoZ == 0){ // Se o sinal de referenciamento estiver desligado
            SinalReferenciamentoZ +=1; // Ativa o sinal de referenciamento
        }
    }
    debounce.reset(); // Reseta o tempo de debounce
}
//FUNCAO QUE COLOCA DEBOUNCE PARA ALTERAR O VALOR DA VARIAVEL SINALREFERENCIAMENTOZ

void CheckFimZ(){
    if(debounce.read_ms() > 50){ // Se a leitura do tempo de debounce for maior que 50ms
        if(SinalReferenciamentoZ == 1){ // Se o sinal de referenciamento estiver ligado
            SinalReferenciamentoZ +=1; // Incrementa o sinal de referenciamento
        }
    }

    debounce.reset(); // Reseta o tempo de debounce
}

//FUNCAO DE REFRENCIAMENTO DO EIXO X
void ReferenciamentoX(int MotorAcionado){
    while(MotorAcionado == 1){ // Enquanto o motor estiver acionado
        Counter +=1; // Incrementa o contador
        if(SinalReferenciamentoX == 0){ // Se o sinal de referenciamento estiver desligado
         if(FimDeCursoX1 == 0){ // Se o motor atingir o fim do curso X1
                SinalReferenciamentoX = 1;  // Ativa o sinal de referenciamento de X = 1
                EnableMotorX = 1;
                break;  // Sai do loop
         }
            StepDriverXY =! StepDriverXY; // Inverte o estado do driver de passo
            wait_us(1500); // Espera tempo
            EnableMotorX = 0; // Habilita o motor
            MotorX.definirDirecao(HORARIO); // Define a direção do motor para o sentido horário

        } else{
            Counter = 0; // Reseta o contador
            while(1){ // Loop infinito
                StepDriverXY =! StepDriverXY; // Inverte o estado do driver de passo
                wait_ms(tempo); // Espera um período especificado
                MotorX.definirDirecao(ANTIHORARIO); // Define a direção do motor para o sentido anti-horário
                Counter +=1; // Incrementa o contador
                if(Counter >= 200){ // Se o contador atingir 200
                    SinalReferenciamentoX = 1; // Ativa o sinal de referenciamento
                    EnableMotorX = 1; // Desabilita o motor
                    MotorAcionado = 2; // Define o estado do motor como 2
                    break; // Sai do loop
                }   
            }
        }        
    }            
}

//FUNCAO QUE COLOCA DEBOUNCE PARA ALTERAR O VALOR DA VARIAVEL SINALREFERENCIAMENTOX
void CheckInicioX(){
    if(debounce.read_ms() > 50){  // Se a leitura do tempo de debounce for maior que 50ms
        if(SinalReferenciamentoX == 0){ // Se o sinal de referenciamento estiver desligado
            SinalReferenciamentoX +=1; // Ativa o sinal de referenciamento
        }
    }
    debounce.reset(); // Reseta o tempo de debounce
}

//FUNCAO QUE COLOCA DEBOUNCE PARA ALTERAR O VALOR DA VARIAVEL SINALREFERENCIAMENTOX
void CheckFimX(){
    if(debounce.read_ms() > 50){ // Se a leitura do tempo de debounce for maior que 50ms
        if(SinalReferenciamentoX == 1){ // Se o sinal de referenciamento estiver ligado
            SinalReferenciamentoX +=1; // Incrementa o sinal de referenciamento
        }
    }

    debounce.reset(); // Reseta o tempo de debounce
}


//FUNCAO DE REFRENCIAMENTO DO EIXO Y
void ReferenciamentoY(int MotorAcionado){
    while(MotorAcionado == 1){ // Enquanto o motor estiver acionado
        Counter +=1; // Incrementa o contador
        if(SinalReferenciamentoY == 0){ // Se o sinal de referenciamento estiver desligado
            StepDriverXY =! StepDriverXY; // Inverte o estado do driver de passo
            wait_ms(tempo); // Espera tempo
            EnableMotorY = 0; // Habilita o motor
            MotorY.definirDirecao(HORARIO); // Define a direção do motor para o sentido horário

        } else{
            Counter = 0; // Reseta o contador
            while(1){ // Loop infinito
                StepDriverXY =! StepDriverXY; // Inverte o estado do driver de passo
                wait_ms(tempo); // Espera um período especificado
                MotorY.definirDirecao(ANTIHORARIO); // Define a direção do motor para o sentido anti-horário
                Counter +=1; // Incrementa o contador
                if(Counter >= 200){ // Se o contador atingir 200
                    SinalReferenciamentoY = 1; // Ativa o sinal de referenciamento
                    EnableMotorY = 1; // Desabilita o motor
                    MotorAcionado = 3; // Define o estado do motor como 2
                    break; // Sai do loop
                }   
            }
        }        
    }            
}



//FUNCAO QUE COLOCA DEBOUNCE PARA ALTERAR O VALOR DA VARIAVEL SINALREFERENCIAMENTOY
void CheckInicioY(){
    if(debounce.read_ms() > 50){  // Se a leitura do tempo de debounce for maior que 50ms
        if(SinalReferenciamentoY == 0){ // Se o sinal de referenciamento estiver desligado
            SinalReferenciamentoY +=1; // Ativa o sinal de referenciamento
        }
    }
    debounce.reset(); // Reseta o tempo de debounce
}

//FUNCAO QUE COLOCA DEBOUNCE PARA ALTERAR O VALOR DA VARIAVEL SINALREFERENCIAMENTOY
void CheckFimY(){
    if(debounce.read_ms() > 50){ // Se a leitura do tempo de debounce for maior que 50ms
        if(SinalReferenciamentoY == 1){ // Se o sinal de referenciamento estiver ligado
            SinalReferenciamentoY +=1; // Incrementa o sinal de referenciamento
        }
    }

    debounce.reset(); // Reseta o tempo de debounce
}


// FUNCAO RESPONSAVEL POR AJUSTAR A QUANTIDADE DE MILILITROS DE ACORDO COM INCREMENTO E EXIBIR ESSE VALOR DE MILILITROS NO LCD
void aguardarAjusteMililitros(int* mililitros, int incremento) {
    lcd.cls(); // Limpa o display LCD
    lcd.locate(0, 1); // Posiciona o cursor na linha 1, coluna 0
    lcd.printf("Mililitros: %d\n", *mililitros); // Imprime a quantidade de mililitros no display LCD

    // Se o incremento for positivo, ou se for negativo e os mililitros forem maiores que zero
    if(!(incremento < 0 && *mililitros == 0)) {
        *mililitros += incremento; // Ajusta a quantidade de mililitros pelo valor do incremento
    }
}

 //FUNCAO RESPONSAVEL POR CALCULAR A QUANTIDADE DE MILILITROS QUE SERAQ DESPEJADA
int calcularMililitrosDespejo() {
    lcd.cls(); // Limpa o display LCD
    lcd_show(10); // Mostra a tela do caso 10 da funcao lcd_show
    int mililitros = 1; // Define a quantidade inicial de mililitros
    bool prosseguir = false; // Define uma flag para controlar o loop

    while (!prosseguir) { // Enquanto não for para prosseguir
        lcd.locate(0, 0); // Posiciona o cursor na linha 1, coluna 0
        lcd.printf("Qts ml para a \n \r");
        lcd.printf("posicao: %3d\n", mililitros); // Imprime a quantidade de mililitros no LCD

        // Se o botão de diminuir mililitros for pressionado
        if (botaoMENOSML.read() == 0) {
            aguardarAjusteMililitros(&mililitros, -1); // Diminui a quantidade de mililitros
            debounce.reset();
        }

        // Se o botão de aumentar mililitros for pressionado
        if (botaoMAISML.read() == 0) {
            aguardarAjusteMililitros(&mililitros, 1); // Aumenta a quantidade de mililitros
            debounce.reset();
        }

        // Se o botão de confirmação for pressionado
        if (botaoCONFIRMACAO.read() == 0) {
            prosseguir = true; // Sinaliza para prosseguir e sair do loop
        }
    }

    return mililitros;  // Retorna a quantidade de mililitros
}


void JOGManual(int index){

// Inicialização de contadores
    ContaIndex = 0;
    // UltimoX = 0;
    // UltimoY = 0;
    CounterX = 0;
    CounterY = 0;
    CounterZ = 0;

// Se o valor da variável 'exec' for maior ou igual a 7, limpa o display LCD e imprime uma mensagem
    if(exec >= 7){
        ContaIndex = 1;
        lcd.cls();
        lcd.printf("Agora selecione a \n");
        lcd.printf("posicao %3d \n", ContaIndex);
        wait(2);
    }

    // Este loop principal é executado até que 'ContaIndex' seja maior que o 'index' passado para a função
    // Este código move o eixo X para a direção desejada com base na leitura do joystick
    // e nos limites físicos (FimDeCursoX1 e FimDeCursoX2)
    // O processo é similar para os eixos Y e Z

        while(ContaIndex <= index){
        
        lcd_show(4);
        
        // Código para controle manual do eixo X
        CounterX = PosicaoX;
        while(SinalJOGManual == 1){
            // lcd.locate(10, 2);
            // lcd.printf("X:  %d\n", SinalJOGManual);
            x = EixoYJoyStick.read() * 1000;
            lcd.locate(0, 0);
            lcd.printf("eixo X: %4d\n", PosicaoX);
            while(x > 600 && SinalJOGManual == 1){
                x = EixoYJoyStick.read() * 1000;
                if(FimDeCursoX2 == 0){
                    EnableMotorX = 1;
                    // printf("Posicao X = %4d \n \r", CounterX);
                    continue;
                } else {
                    StepDriverXY = !StepDriverXY;
                    wait_ms(tempo);
                    EnableMotorX = 0;
                    MotorX.definirDirecao(ANTIHORARIO);
                    CounterX += 1;
                    // printf("Posicao X = %4d \n \r", CounterX); 
                }
                
            }
            while(x < 300 && SinalJOGManual == 1){
                x = EixoYJoyStick.read() * 1000;
                 if(FimDeCursoX1 == 0){
                    EnableMotorX = 1;
                    continue;
                } else {
                    StepDriverXY = !StepDriverXY;
                    wait_ms(tempo);
                    EnableMotorX = 0;
                    MotorX.definirDirecao(HORARIO);
                    CounterX -= 1;
                    // printf("Posicao X = %4d \n \r", CounterX);
                }
            }
            PosicaoX = CounterX;
            EnableMotorX = 1;
            EnableMotorY = 1;
        if(botaoMAISML.read() == 0){
            SinalJOGManual = 4;
        }
        }

        // Código para controle manual do eixo Y
        CounterY = PosicaoY;
        while(SinalJOGManual == 2){
            // lcd.locate(10, 2);
            // lcd.printf("Y: %d\n", SinalJOGManual);
            y = EixoXJoyStick.read() * 1000;
            lcd.locate(0, 1);
            lcd.printf("eixo Y: %4d\n", PosicaoY);
            
            while(y > 600 && SinalJOGManual == 2){
                y = EixoXJoyStick.read() * 1000;
                 if(FimDeCursoY2 == 0){
                    EnableMotorY = 1;
                    continue;
                } else {
                    StepDriverXY = !StepDriverXY;
                    wait_ms(tempo);
                    EnableMotorY = 0;
                    MotorY.definirDirecao(ANTIHORARIO);
                    CounterY += 1;
                    // printf("Posicao Y = %4d \n \r", CounterY); 
                }
                
            }
            while(y <  300 && SinalJOGManual == 2){
                y = EixoXJoyStick.read() * 1000;
                if(FimDeCursoY1 == 0){
                    EnableMotorY = 1;
                    continue;
                } else {
                    StepDriverXY = !StepDriverXY;
                    wait_ms(tempo);
                    EnableMotorY = 0;
                    MotorY.definirDirecao(HORARIO);
                    CounterY -= 1;
                    // printf("Posicao Y = %4d \n \r", contador);
                }
            }
        if(botaoMAISML.read() == 0){
            SinalJOGManual = 4;
        }

            PosicaoY = CounterY;
            EnableMotorX = 1;
            EnableMotorY = 1;
        }

        // Código para controle manual do eixo Z
        CounterZ = PosicaoZ;
        while(SinalJOGManual == 3){
            // lcd.locate(10, 2);
            // lcd.printf("Z: %d\n", SinalJOGManual);
            y = EixoYJoyStick.read() * 1000;
            
            lcd.locate(0, 2);
            lcd.printf("eixo Z: %4d\n", PosicaoZ);
            
            while(y > 600 && SinalJOGManual == 3){
                y = EixoYJoyStick.read() * 1000;
                if(FimDeCursoZ1 == 0){
                    EnableMotorZ = 1;
                    continue;
                } else {

                    StepDriverZ = !StepDriverZ;
                    wait_ms(tempo);
                    EnableMotorZ = 0;
                    MotorZ.definirDirecao(HORARIO);
                    CounterZ -= 1;
                    // printf("Posicao Y = %4d \n \r", CounterY); 
                }
                
            }
            while(y < 300 && SinalJOGManual == 3){
                y = EixoYJoyStick.read() * 1000;
                 if(FimDeCursoZ2 == 0){
                    EnableMotorZ = 1;
                    continue;
                 }else {
                    StepDriverZ = !StepDriverZ;
                    wait_ms(tempo);
                    EnableMotorZ = 0;
                    MotorZ.definirDirecao(ANTIHORARIO);
                    CounterZ += 1;
                    // printf("Posicao Y = %4d \n \r", contador);
                }
                
            }
            PosicaoZ = CounterZ;
            EnableMotorX = 1;
            EnableMotorY = 1;
            EnableMotorZ = 1;
        if(botaoMAISML.read() == 0){
            SinalJOGManual = 4;
        }
        }

        // Quando o botão 'MAISML' é pressionado (lido como 0), a posição atual e a quantidade de 'mililitros' são salvas
        // nas respectivas listas (array) para cada eixo e 'mililitros'
        while(SinalJOGManual == 4){
            mililitros = calcularMililitrosDespejo();
            listaPosX[ContaIndex] = PosicaoX;
            listaPosY[ContaIndex] = PosicaoY;
            listaPosZ[ContaIndex] = PosicaoZ;
            lista_ml[ContaIndex] = mililitros;
            UltimoX = PosicaoX;
            UltimoY = PosicaoY;
            UltimoZ = PosicaoZ;
            // A posição salva e o valor de 'ContaIndex' são impressos para depuração
            printf("Posicao salva eixo X: %d \n \r", listaPosX[ContaIndex]);
            printf("Posicao salva eixo Y: %d \n \r", listaPosY[ContaIndex]);
            printf("Posicao salva eixo Z: %d \n \r", listaPosZ[ContaIndex]);
            printf("Mililitros salvos: %d \n \r", lista_ml[ContaIndex]);
            printf("Valor do counter %d \n \r", ContaIndex);
            printf("Valor do indice %d \n \r", index);
            ContaIndex += 1;
             // Se o 'exec' for maior ou igual a 7 e ContaIndex maior que 'index' ou se 'ContaIndex' for maior ou igual a 'index'
            // então SinalJOGManual é definido como 5 e o processo é interrompido
            // Caso contrário, SinalJOGManual é definido como 1 e a próxima posição é solicitada
            
            if(exec >= 7){
                if(ContaIndex > index){
                    SinalJOGManual = 5;
                    break;
                } else {
                    SinalJOGManual = 1;
                    lcd.cls();
                    lcd.printf("Agora selecione a\n" );
                    lcd.printf("posicao %3d \n", ContaIndex);
                    printf("Selecione a proxima posicao \n \r");
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
                    // printf("Selecione a proxima posicao \n \r");
                    wait(2);
                    break;
                 }
            }
            
        }
        // Quando 'SinalJOGManual' é 5, os motores são desativados e uma mensagem de conclusão é impressa
        if(SinalJOGManual == 5){
            EnableMotorX = 1;
            EnableMotorY = 1;
            EnableMotorZ = 1; 
            printf("JOGManual finalizado \n \r");
            break;
        }


        EnableMotorX = 1;
        EnableMotorY = 1;
        EnableMotorZ = 1;

        }
    }
// }


// JOG AUTOMATICO DOS EIXOS X E Y
void JogAutomaticoXY(int AlvoX, int AlvoY) {
    CounterAutoX = UltimoX; // Atribui a última posição de X ao contador X do Jog Automatico
    CounterAutoY = UltimoY; // Atribui a última posição de Y ao contador Y do Jog Automatico

    // printf("CounterX %d \n \r", CounterAutoX); // Imprime o valor atual do contador X do Jog Automatico
    // printf("CounterY %d \n \r", CounterAutoY); // Imprime o valor atual do contador Y do Jog Automatico
    // printf("Alvo atual X %d \n \r", AlvoX); // Imprime o alvo atual X
    // printf("Alvo atual Y %d \n \r", AlvoY); // Imprime o alvo atual Y

    SinalFinalizadoX = 0; // Sinal de conclusão para o eixo X é definido como 0 (incompleto)
    SinalFinalizadoY = 0; // Sinal de conclusão para o eixo Y é definido como 0 (incompleto)

    while(!(SinalFinalizadoX && SinalFinalizadoY)) { // Enquanto X ou Y não atingirem o destino...
        StepDriverXY = !StepDriverXY; // Alterna o estado do driver de passo
        wait_ms(tempo); // Espera por um tempo definido anteriormente

        // Ajusta a direção e o contador do eixo X com base na posição atual e no alvo
        if(CounterAutoX > AlvoX){            
            EnableMotorX = 0; // Habilita o motor X
            MotorX.definirDirecao(HORARIO); // Define a direção do motor X para horário
            CounterAutoX-=1; // Diminui o contador X do Jog Automatico
        } else if(CounterAutoX < AlvoX){
            EnableMotorX = 0; // Habilita o motor X
            MotorX.definirDirecao(ANTIHORARIO); // Define a direção do motor X para anti-horário
            CounterAutoX+=1; // Aumenta o contador X do Jog Automatico
        } else {
            EnableMotorX = 1; // Desabilita o motor X
            UltimoX = CounterAutoX; // Atribui a posição atual do contador X à última posição de X
            SinalFinalizadoX = 1; // Sinal de conclusão para o eixo X é definido como 1 (completo)
        }

        // Ajusta a direção e o contador do eixo Y com base na posição atual e no alvo
        if(CounterAutoY > AlvoY){
            EnableMotorY = 0; // Habilita o motor Y
            MotorY.definirDirecao(ANTIHORARIO); // Define a direção do motor Y para anti-horário
            CounterAutoY-=1; // Diminui o contador Y do Jog Automatico
        } else if(CounterAutoY < AlvoY){
            EnableMotorY = 0; // Habilita o motor Y
            MotorY.definirDirecao(HORARIO); // Define a direção do motor Y para horário
            CounterAutoY+=1; // Aumenta o contador Y do Jog Automatico
        } else {
            EnableMotorY = 1; // Desabilita o motor Y
            UltimoY = CounterAutoY; // Atribui a posição atual do contador Y à última posição de Y
            SinalFinalizadoY = 1; // Sinal de conclusão para o eixo Y é definido como 1 (completo)
        }
    }
}



// void JogAutomaticoY(int ValorAlvo) {
//     CounterAutoY = UltimoY;
//     printf("CounterY %d \n \r", CounterAutoY);

//     while(CounterAutoY != ValorAlvo) {
//         StepDriverXY = !StepDriverXY;
//         wait_ms(tempo);
        
//         if(CounterAutoY > ValorAlvo){
//             EnableMotorY = 0;
//             MotorY.definirDirecao(ANTIHORARIO);
//             CounterAutoY-=1;
//         } else if(CounterAutoY < ValorAlvo){
//             EnableMotorY = 0;
//             MotorY.definirDirecao(HORARIO);
//             CounterAutoY+=1;
//         }
//     }
    
//     printf("Posicionado no Alvo Y \n \r");
//     EnableMotorY = 1;
//     UltimoY = CounterAutoY;
// }

void JogAutomaticoZ(int ValorAlvo) {
    CounterAutoZ = UltimoZ; // Atribui a última posição de Z ao contador Z do Jog Automatico
    EnableMotorX = 1; // Desabilita o motor X
    EnableMotorY = 1; // Desabilita o motor Y

    // printf("CounterZ %d \n \r", CounterAutoZ); // Imprime o valor atual do contador Z do Jog Automatico

    while(CounterAutoZ != ValorAlvo) { // Enquanto Z não atingir o destino...
        StepDriverZ = !StepDriverZ; // Alterna o estado do driver de passo
        wait_ms(tempo); // Espera por um tempo definido anteriormente
        
        // Ajusta a direção e o contador do eixo Z com base na posição atual e no alvo
        if(CounterAutoZ > ValorAlvo){
            EnableMotorZ = 0; // Habilita o motor Z
            MotorZ.definirDirecao(HORARIO); // Define a direção do motor Z para anti-horário
            CounterAutoZ-=1; // Diminui o contador Z
        } else if(CounterAutoZ < ValorAlvo){
            EnableMotorZ = 0; // Habilita o motor Z
            MotorZ.definirDirecao(ANTIHORARIO); // Define a direção do motor Z para horário
            CounterAutoZ+=1; // Aumenta o contador Z
        }
    }
    EnableMotorZ = 1; // Desabilita o motor Z
    UltimoZ = CounterAutoZ; // Atribui a posição atual do contador Z à última posição de Z
}

// FUNCAO DO BOTAO DE CONFIRMACAO
void LidaConfirma() {
    if(debounce.read_ms() > 300) { // Se a leitura do tempo de debounce for superior a 100ms
        exec += 1; // Incrementa o contador exec  
    }
    prosseguir = true; // Seta a variável de prosseguir como verdadeira
    wait(1); // Espera por um segundo
    debounce.reset(); // Reseta o tempo de debounce
}

// FUNCAO DO BOTAO DE CONFIRMACAO
void LidaPosicao() {
    if(debounce.read_ms() > 500) { // Se a leitura do tempo de debounce for superior a 100ms
        if(SinalJOGManual == 3) {
            SinalJOGManual = 1;
        } else {
            SinalJOGManual += 1;

        }
           
    }
    debounce.reset(); // Reseta o tempo de debounce
    // lcd.locate(10, 2);
    // lcd.printf("SINAL: %d\n", SinalJOGManual);
}


//FUNCAO DO BOTAO MENU
void LidarVoltar() {
    if(debounce.read_ms() > 100){ // Se a leitura do tempo de debounce for superior a 100ms
        SinalReferenciamentoX = 0; // Seta o sinal de referenciamento de X para 0
        SinalReferenciamentoY = 0; // Seta o sinal de referenciamento de Y para 0
        SinalReferenciamentoZ = 0; // Seta o sinal de referenciamento de Z para 0
        exec = 0; // Reseta o estágio de execução para 0
    }
    debounce.reset(); // Reseta o tempo de debounce
}

void LidaOk(){
    led = 1;
    if(debounce.read_ms() > 200){
        SinalJOGManual = 4;
        }
        debounce.reset();
    }
    


// Função de tratamento para botão de emergência
void LidaEmergencia(){
    // Desativa todos os motores
    EnableMotorX = 1;
    EnableMotorY = 1;
    EnableMotorZ = 1;

    // Reinicia o sistema
    NVIC_SystemReset();
}

int SelecaoPontosSoltar() {
    bool isRunning = true;

    lcd.cls();
    lcd_show(5);
    
    while(isRunning) {
        
        if(botaoMAISML.read() == 0 ) { // Add a check here
            wait(0.3);
            NSoltar += 1;
        }
        else if(botaoMENOSML.read() == 0 && NSoltar > 1) {
            wait(0.3);
            NSoltar -= 1;
        }
        
        lcd.locate(0, 1);
        printf("Numero de soltas: %d \n \r", NSoltar);
        lcd.printf("De despejo: %3d\n", NSoltar);
        
        if(exec == 7){
            isRunning = false;
        }
    }
    
    //returning current number of releases
    return NSoltar;
}






int main() {
    wait(1);
    // Inicia o temporizador debounce
    debounce.start();
    // Liga a luz de fundo do LCD e configura interrupções
    lcd.setBacklight(TextLCD::LightOn);
    FimDeCursoY1.fall(&CheckInicioY);
    FimDeCursoZ1.fall(&CheckInicioZ);
    FimDeCursoX1.fall(&CheckInicioX);
    botaoMENOSML.fall(&LidaPosicao);


    botaoCONFIRMACAO.fall(&LidaConfirma);
    // botaoMAISML.fall(&LidaOk);
    botaoEMERGENCIA.fall(&LidaEmergencia);


    // Desativa todos os motores e define direções iniciais
    EnableMotorX = 1;
    EnableMotorY = 1;
    EnableMotorZ = 1;
    StepDriverXY = 1;
    StepDriverZ = 1;

    MotorX.definirDirecao(HORARIO);
    MotorY.definirDirecao(ANTIHORARIO);
    MotorZ.definirDirecao(HORARIO);

    AcionamentoPipeta = 1;

    // Verifica se o botão de emergência está pressionado no início da execução
    if(botaoEMERGENCIA == 0){
        lcd_show(3);
        EnableMotorX = 1;
        EnableMotorY = 1;
        EnableMotorZ = 1;
        StepDriverXY = 0;
        StepDriverZ = 0;

        // Aguarda até o botão de emergência ser solto
        while(botaoEMERGENCIA == 0){}
    }

    // Loop principal
    while(1){
        EnableMotorZ = 1;
        EnableMotorX = 1;
        EnableMotorY = 1;

        // A variável "exec" determina o estado atual da execução
        // exec = 0: Inicialização
        if(exec == 0){
            lcd_show(0);
            exec = 1;
            // lcd.locate(19, 3);
            // lcd.printf("%i", exec);
        } 
        // exec = 2: Referenciamento dos eixos
        else if(exec == 2){
            lcd_show(1);
            ReferenciamentoZ(1);
            ReferenciamentoX(1);
            ReferenciamentoY(1);
            exec = 3;
            // lcd.locate(19, 3);
            // lcd.printf("%i", exec);

        }
        // exec = 3: Preparação para o movimento manual
        else if(exec == 3){
            lcd_show(2);
            EnableMotorX = 1;
            EnableMotorY = 1;
            EnableMotorZ = 1;
            StepDriverXY = 0;
            StepDriverZ = 0;

            exec = 4;
            // lcd.locate(19, 3);
            // lcd.printf("%i", exec);

        }
        // exec = 5: Movimento manual
        else if(exec == 5){
            SinalJOGManual = 1;
            JOGManual(1);
            exec = 6;
            // lcd.locate(19, 3);
            // lcd.printf("%i", exec);
        } 
        // exec = 6: Seleção dos pontos de soltura
        else if(exec == 6){
            SelecaoPontosSoltar();

            // lcd.locate(19, 3);
            // lcd.printf("%i", exec);

        }
        // exec = 7: Preparação para a soltura
        else if(exec == 7){
            lcd_show(6);
            exec = 8;
            // lcd.locate(19, 3);
            // lcd.printf("%i", exec);
        }
        // exec = 9: Movimento manual até o ponto de soltura
        else if(exec == 9){
            SinalJOGManual = 1;
            printf("Numero de posicoes de solta %d \n \r", NSoltar + 1);
            JOGManual(NSoltar);
            etapa_real = 9 + NSoltar;
            exec = etapa_real;
            // lcd.locate(19, 3);
            // lcd.printf("%i", exec);
        }
        //// exec = etapa_real: Preparação para o ciclo de soltura
        else if(exec == etapa_real){
            EnableMotorX = 1;
            EnableMotorY = 1;
            EnableMotorZ = 1;
            StepDriverXY = 1;
            StepDriverZ = 1;

            lcd_show(7);
            prosseguir = false;
            exec = etapa_real + 1;
            // lcd.locate(19, 3);
            // lcd.printf("%i", exec);
        }
        // exec = etapa + 1: Movimento automático para o ponto de soltura
        else if(exec == etapa_real + 1 && prosseguir == true){
            lcd_show(8);
            JogAutomaticoZ(ValorInicialZ);
            exec = etapa_real + 2;
            // lcd.locate(19, 3);
            // lcd.printf("%i", exec);
        }
        // exec = etapa_real + 2: Ciclo de soltura
        else if(exec == etapa_real + 2){
            exec = 14;
            lcd_show(9);

            for(int i = 1; i <= NSoltar; i+=1){
                lcd.cls();
                lcd.locate(0,0);
                lcd.printf("Ciclo para posicao %d \n", i);
                wait(1);
                Counter = 0;

                if(i >= 0){
                    mililitros = lista_ml[i];
                    while(Counter < mililitros){
                        lcd.locate(0,1);
                        lcd.printf("Iteracao numero %d \n", Counter);
                        JogAutomaticoZ(ValorInicialZ);
                        JogAutomaticoXY(listaPosX[0], listaPosY[0]);
                        JogAutomaticoZ(listaPosZ[0]);
                        AcionamentoPipeta = 0;
                        wait(2);
                        AcionamentoPipeta = 1;
                        wait(4);
                        JogAutomaticoZ(ValorInicialZ);
                        
                        JogAutomaticoXY(listaPosX[i], listaPosY[i]);
                        JogAutomaticoZ(listaPosZ[i]);

                        AcionamentoPipeta = 0;
                        wait(2);
                        AcionamentoPipeta = 1;
                        wait(4);
                        Counter +=1;
                    }
                }             
            }

            JogAutomaticoZ(ValorInicialZ);
            JogAutomaticoXY(ValorInicialX, ValorInicialY);
            lcd.cls();
            lcd.printf("Ciclos finalizados");
            EnableMotorX = 1;
            EnableMotorY = 1;
            EnableMotorZ = 1;
        }
        //exec = 14: Retorna para a etapa real
        else if(exec == 14){
            exec = etapa_real;
        }
    }
}

