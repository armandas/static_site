#include <p18f4450.h>
#include <delays.h>

#pragma config WDT = OFF
#pragma config PLLDIV = 5  
#pragma config CPUDIV = OSC3_PLL4
#pragma config USBDIV = 2
#pragma config FOSC = HSPLL_HS
#pragma config BOR = OFF 
#pragma config PBADEN = OFF
#pragma config PWRT = OFF
#pragma config MCLRE = OFF 
#pragma config XINST = OFF

// ---------------------- function defs -------------------------- //
void HighPriorityISRCode(void);
void LowPriorityISRCode(void);
extern void _startup (void);        // See c018i.c in C18 compiler dir

// --------------------- Vector Remapping -------------------------//
#define REMAPPED_RESET_VECTOR_ADDRESS			0x1000
#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1008
#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1018

#pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
void _reset (void)
{
    _asm goto _startup _endasm
}
#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
void Remapped_High_ISR (void)
{
     _asm goto HighPriorityISRCode _endasm
}
#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
void Remapped_Low_ISR (void)
{
     _asm goto LowPriorityISRCode _endasm
}
#pragma code

//---------------------- Interrupt Handlers ----------------------//

#pragma interrupt HighPriorityISRCode
void HighPriorityISRCode(void){
// This space deliberately left blank :) 
}
	
#pragma interruptlow LowPriorityISRCode
void LowPriorityISRCode(void){
// This space deliberately left blank :) 
}
#pragma code

//-------------------------Main Code -------------------------------//

// movement definitions
#define FORWARD    0b0101
#define REVERSE    0b1010
#define STOP       0b0000
#define SLOW_LEFT  0b0001
#define FAST_LEFT  0b1001
#define SLOW_RIGHT 0b0100
#define FAST_RIGHT 0b0110

// definition of modes
#define IDLE      1
#define TEST      2
#define NORMAL    4
#define RAMP      8
#define MEMORISE 16
#define MODE_SEL 32

#define MEM_SIZE 100
unsigned char mem[MEM_SIZE];
int mem_counter = 0;

// enters mode selection mode if button RD6 is pressed
void check_mode_sel(unsigned char *mode)
{
    if (PORTDbits.RD6 == 0)
        *mode = MODE_SEL;
}

// mode selection
void choose_mode(unsigned char *mode,
                 unsigned char *delta_mode,
                 int *mode_timeout)
{
    // check if button has not been
    // pressed for some time
    if (*mode_timeout > 10) {
        // indicate timeout by lighting up LEDs
        LATA = 0xff;
        Delay10KTCYx(255);
        Delay10KTCYx(255);

        // confirm chosen mode
        *mode = *delta_mode;
        // indicate selected mode
        LATA = *mode;
        Delay10KTCYx(255);

        // restore temporary variables
        *delta_mode = 1;
        *mode_timeout = 0;

        return;
    }
    // if timeout has not yet occured
    // wait for input from the user
    else {
        // if button is pressed
        if (PORTDbits.RD6 == 0) {
            // reset timeout counter
            *mode_timeout = 0;

            // we got 5 modes, and need to start
            // from the beggining after 5th
            if (*delta_mode == 32)
                *delta_mode = 1;
            // if not yet at 5, choose next mode
            else
                *delta_mode = *delta_mode << 1;

            // show mode number
            LATA = *delta_mode;
            Delay10KTCYx(255);
        }
        // if button is not pressed
        else {
            // increase timeout counter
            (*mode_timeout)++;
            Delay10KTCYx(255);
        }
    }
}

// LED effects for IDLE mode
void kitt(unsigned char *th, unsigned char *th_dir)
{
    if (*th > 31)
        *th_dir = 0;
    else if (*th < 2)
        *th_dir = 1;

    if (*th_dir == 1) {
        *th = *th << 1;
        LATA = *th;
        Delay10KTCYx(230);
    }
    else {
        *th = *th >> 1;
        LATA = *th;
        Delay10KTCYx(230);
    }
}

// main function for following the line
unsigned char go(int *back_counter, unsigned char *out)
{

    // stops in the pit-stop
    if ((PORTD & 0b00011111) == 0b00011111) {
        *out = STOP;
        return STOP;
    }
    else if (PORTD & 0b00000001) {
        *out = FAST_RIGHT;
        return FAST_RIGHT;
    }
    else if (PORTD & 0b00010000) {
        *out = FAST_LEFT;
        return FAST_LEFT;
    }
    else if (PORTD & 0b00000010) {
        *out = SLOW_RIGHT;
        return SLOW_RIGHT;
    } 
    else if (PORTD & 0b00001000) {
        *out = SLOW_LEFT;
        return SLOW_LEFT;
    }
    else if (PORTD & 0b00000100) {
        *out = FORWARD;
        // when line is found, reset counter
        // see below for details
        *back_counter = 0;
        return FORWARD;
    }
    // if no black line is detected
    else {
        // do not move backwards immediately
        if ((*back_counter)++ < 2000)
            // this could probably benefit from
            // moving forwards
            return 0xff;

        // go backwards if no line is found
        // after some time
        *out = REVERSE;
        Delay1KTCYx(100);
        *out = STOP;
        Delay1KTCYx(5);
    }
}

// function for going over the ramp
// utilizes only the side sensors
// also uses delays to move slower
void go_ramp(void)
{
    if (PORTD & 0b00000001) {
        LATB = FAST_LEFT;
        Delay1KTCYx(5);
    }
    else if (PORTD & 0b00010000) {
        LATB = FAST_RIGHT;
        Delay1KTCYx(5);
    }
    else {
        LATB = FORWARD;
        Delay1KTCYx(5);
    }
    LATB = STOP;
    Delay1KTCYx(6);
}

// function for memorising the track
// uses small array stored in ram
// cannot memorise whole track
// proof of concept only
void memorise(int *back_counter)
{
    int i;
    int mem_count = 0;
    unsigned char dir = 0xff;

    // indication of start
    for (i = 0; i < 5; i++) {
        LATA = 0xff;
        Delay10KTCYx(255);
        LATA = 0x00;
        Delay10KTCYx(255);
    }

    // learn the track
    for (i = 0; i < MEM_SIZE; i++) {
        dir = go(back_counter, &LATB);
        // skip data if track is lost
        if (dir == 0xff)
           continue;

        //put direction into array
        mem[mem_count++] = dir;
        Delay10KTCYx(15);
    }

    LATB = STOP;
    // indication of finish
    for (i = 0; i < 5; i++) {
        LATA = 0xff;
        Delay10KTCYx(255);
        LATA = 0x00;
        Delay10KTCYx(255);
    }

    // replay the track
    for (i = 0; i < MEM_SIZE; i++) {
        LATB = mem[i];
        Delay10KTCYx(15);
    }
    LATB = STOP;
}

void main (void)
{
    unsigned char mode = IDLE;

    // variables for the idle mode
    unsigned char throbber = 1;
    unsigned char th_direction = 1;

    // variables for mode selection
    int mode_timeout = 0;
    unsigned char delta_mode = 1;

    int back_counter = 0;

    TRISA = 0x00;       // set PORTA as output port (LEDs)
    TRISB = 0xf0;       // set PORTB as output port (Motors)
    TRISD = 0b00111111; // set PORTD as input port  (Sensors)

    LATB = STOP;

    while (1) {
        check_mode_sel(&mode);

        switch (mode) {
            case IDLE:
                kitt(&throbber, &th_direction);
                break;

            case TEST:
                LATA = PORTD;
                break;
            
            case NORMAL:
                go(&back_counter, &LATB);
                break;
            
            case RAMP:
                go_ramp();
                break;
            
            case MEMORISE:
                memorise(&back_counter);
                break;
            
            case MODE_SEL:
                choose_mode(&mode, &delta_mode, &mode_timeout);
                break;
        }
    }
}
