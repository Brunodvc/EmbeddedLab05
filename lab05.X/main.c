//--------------------------------------------------------------------
// Name: William Richardson, Bruno Do Val Callao
// Date:10/4
// Purp: Lab05
//
// Assisted:
// Assisted by: Microchips 18F26K22 Tech Docs
// Conversion to MCC Melody: Nolan Pratt (nolanpratt@mines.edu), Fall 2025
//-
//- Academic Integrity Statement: I certify that, while others may have
//- assisted me in brain storming, debugging and validating this program,
//- the program itself is my own work. I understand that submitting code
//- which is the work of other individuals is a violation of the course
//- Academic Integrity Policy and may result in a zero credit for the
//- assignment, or course failure and a report to the Academic Dishonesty
//- Board. I also understand that if I knowingly give my original work to
//- another individual that it could also result in a zero credit for the
//- assignment, or course failure and a report to the Academic Dishonesty
//- Board.
//------------------------------------------------------------------------
#include "mcc_generated_files/system/system.h"
#include <inttypes.h>
#pragma warning disable 520 // warning: (520) function "xyz" is never called
#pragma warning disable 1498 // fputc.c:16:: warning: (1498) pointer (unknown)
#define COLOR_INC 0x10 //maybe 
#define NUM_COLOR 6
#define STEPS_PER_EDGE 256
#define STEP_SIZE 1000 // 1 step = 0.001 seconds = 1 ms, 1000 counts perstep
#define INC_STEP 200 // +/- step = 0.2 ms = 200 counts on a 1:16

//globals
uint16_t deltaRed[NUM_COLOR] = {0x0000, 0x0000, 0x0001, 0x0000, 0x0000, 0xFFFF};
uint16_t deltaGre[NUM_COLOR] = {0x0001, 0x0000, 0x0000, 0xFFFF, 0x0000, 0x0000};
uint16_t deltaBlu[NUM_COLOR] = {0x0000, 0xFFFF, 0x0000, 0x0000, 0x0001, 0x0000};

uint16_t initRed[NUM_COLOR] = {0x00,   0x00,   0x00,   0xFF,   0xFF,   0xFF};
uint16_t initGre[NUM_COLOR] = {0x00,   0xFF,   0xFF,   0xFF,   0x00,   0x00};
uint16_t initBlu[NUM_COLOR] = {0xFF,   0xFF,   0x00,   0x00,   0x00,   0xFF};

uint8_t color_tour = false; //variable we set true to start the tour/false to end
uint8_t redDuty = 0xFF; //initalize duty cycles
uint8_t greDuty = 0xFF;
uint8_t bluDuty = 0xFF;
uint16_t step = 0;
uint8_t tourIndex = 0;
uint16_t red = 0x00;
uint16_t gre = 0x00;
uint16_t blu = 0xFF;

//ISR 
void myTMR0ISR(void) {
    if (color_tour == true) {
        redDuty = (uint8_t)red;
        greDuty = (uint8_t)gre;
        bluDuty = (uint8_t)blu;
        EPWM1_LoadDutyValue((uint16_t)redDuty << 2);
        EPWM1_LoadDutyValue((uint16_t)greDuty << 2);
        EPWM1_LoadDutyValue((uint16_t)bluDuty << 2);
        red += deltaRed[tourIndex];
        gre += deltaGre[tourIndex];
        blu += deltaBlu[tourIndex]; 
        step++;
        if (step == STEPS_PER_EDGE){
            step = 0;
            tourIndex++;
            if (tourIndex == NUM_COLOR) tourIndex = 0;
            red = initRed[tourIndex];
            gre = initGre[tourIndex];
            blu = initBlu[tourIndex];
                 
        }
    }
    else {  //restart
        step = 0;
        tourIndex = 0;
        red = initRed[0];
        gre = initGre[0];
        blu = initBlu[0];
    }
}


//----------------------------------------------
// Main "function"
//----------------------------------------------
void main(void) {
    uint16_t duty = 127;
    uint8_t i;
    char cmd;
    SYSTEM_Initialize();
    redDuty = 0xFF;
    greDuty = 0xFF;
    bluDuty = 0xFF;
    // Not necessary, but this delay allows the baud rate generator to
    // stablize before printing the splash screen on reset. If you are going to
    // do this, then make sure to put delay BEFORE enabling TMR interrupt.
    TMR0_CounterSet(0x0000);
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 0;
    TMR0_OverflowCallbackRegister(myTMR0ISR);
    while(TMR0_OverflowStatusGet() == false);
    //TMR0_PeriodSet((uint16_t)(65536 - ((uint32_t)tickUs * TICKS_PER_US)));
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    printf("inLab 05\r\n");
    printf("Color Cube\r\n");
    printf("Dev'21 board wiring\r\n");
    printf("RC2 <-> Red LED");
    printf("RC1 <-> Green LED");
    printf("RB5 <-> Blue LED");
    printf("\r\n> "); // print a nice command prompt
    for (;;) {
        if (EUSART1_IsRxReady()) { // wait for incoming data on USART
            cmd = EUSART1_Read();
            switch (cmd) { // and do what it tells you to do
                case '?':
                    printf("------------------------------\r\n");
                    printf("Red: 0x%02x \r\n", (unsigned int)CCPR1L);
                    printf("Green: 0x%02x \r\n", (unsigned int)CCPR2L);
                    printf("Blue: 0x%02x \r\n", (unsigned int)CCPR3L);
                    printf("------------------------------\r\n");
                    printf("?: Help menu\r\n");
                    printf("Z: Reset processor\r\n");
                    printf("z: Clear the terminal\r\n");
                    printf("R/r: increase/decrease Red intensity.\r\n");
                    printf("G/g: increase/decrease Green intensity.\r\n");
                    printf("B/b: increase/decrease Blue intensity. \r\n");
                    printf("C/c: start/stop color cycle. \r\n");
                    printf("a: All LEDs off. \r\n");
                    printf("+/-: increase/decrease the color tour speed. \r\n");
                    printf("------------------------------\r\n");
                break;
                //--------------------------------------------
                //--------------------------------------------
                case 'R':  //increase duty cycle of PWM with red LED
                    if (color_tour) { printf("Stop the color tour (c) first.\r\n"); break; }
                    if (redDuty <= 0xFF - COLOR_INC) redDuty += COLOR_INC;
                    else                             redDuty = 0xFF;
                    EPWM1_LoadDutyValue((uint16_t)redDuty << 2);
                    printf("Red: 0x%02x\r\n", (unsigned int)CCPR1L);
                        break;
                break;
                //--------------------------------------------
                //--------------------------------------------
                case 'r':  //decrease duty cycle of PWM associated with red LED
                    if (color_tour) { printf("Stop the color tour (c) first.\r\n"); break; }
                    if (redDuty >= COLOR_INC) redDuty -= COLOR_INC;
                    else                      redDuty = 0x00;
                    EPWM1_LoadDutyValue((uint16_t)redDuty << 2);
                    printf("Red: 0x%02x\r\n", (unsigned int)CCPR1L);
                break;
                //--------------------------------------------
                //--------------------------------------------
                case 'G': //increase duty cycle of PWM associated with green LED
                    
                break;
                //--------------------------------------------
                //--------------------------------------------
                case 'g':  //decrease duty cycle of PWM associated with green LED
                    
                break;
                //--------------------------------------------
                //--------------------------------------------
                case 'B': //increase duty cycle of PWM associated with blue LED
                    if (duty <= 0xFF - COLOR_INC) duty += COLOR_INC;  // increase the PWM duty cycle of CCP1 on RC2
                    EPWM1_LoadDutyValue(duty);
                    printf("The duty cycle is %d TMR2 counts high in a period of %dTMR2 counts.\r\n", duty, 0xFF);
                break;
                //--------------------------------------------
                // decrease the PWM duty cycle of CCP1 on RC2
                //--------------------------------------------
                case 'b': //decrease duty cycle of PWM associated with blue LED
                    if (duty >= COLOR_INC) duty -= COLOR_INC;
                    EPWM1_LoadDutyValue(duty);
                    printf("The duty cycle is %d TMR2 counts high in a period of %dTMR2 counts.\r\n", duty, 0xFF);
                break;
                //--------------------------------------------
                //--------------------------------------------
                case'C':  //start color cycle
                    printf ("Color tour - BEGIN \r\n");
                    color_tour = true;
                break;
                //--------------------------------------------
                //--------------------------------------------
                case 'c':  //stop color cycle
                    printf ("Color tour - END \r\n");
                    color_tour = false;
                break;
                //--------------------------------------------
                //--------------------------------------------
                case 'a':  //all LEDs off
                    color_tour = false;
                    printf("All LEDs off \r\n");
                break;
                //--------------------------------------------
                //--------------------------------------------
                case '+':  //increase color tour speed
                    printf("increase color tour speed \r\n");
                    if (TMR0_PeriodGet() <= 65535 - INC_STEP) TMR0_PeriodSet(TMR0_PeriodGet() + INC_STEP);
                break;
                //--------------------------------------------
                //--------------------------------------------
                case '-':  //decrease color tour speed
                    printf("decrease color tour speed \r\n");
                    
                break;
                //--------------------------------------------
                // Reset the processor after clearing the terminal
                //--------------------------------------------
                case 'Z':
                    for (i = 0; i < 40; i++) printf("\n");
                    RESET();
                break;
                //--------------------------------------------
                // Clear the terminal
                //--------------------------------------------
                case 'z':
                    for (i = 0; i < 40; i++) printf("\n");
                break;
                //--------------------------------------------
                // If something unknown is hit, tell user
                //--------------------------------------------
                default:
                    printf("Unknown key %c\r\n", cmd);
                break;
            } // end switch
        } // end if
    } // end while
} // end main
    