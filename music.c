// music.c
// Runs on LM4F120/TM4C123
// Use Timer0A in periodic mode to request interrupts at a particular
// period.
// Daniel Valvano
// September 11, 2013

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 9.8

  "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Volume 2, Program 7.5, example 7.6

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
#include <stdint.h>
#include "..//inc//tm4c123gh6pm.h"
#include "Sound.h"

#define C1   1194   // 2093 Hz
#define B1   1265   // 1975.5 Hz
#define BF1  1341   // 1864.7 Hz
#define A1   1420   // 1760 Hz
#define AF1  1505   // 1661.2 Hz
#define G1   1594   // 1568 Hz
#define GF1  1689   // 1480 Hz
#define F1   1790   // 1396.9 Hz
#define E1   1896   // 1318.5 Hz
#define EF1  2009   // 1244.5 Hz
#define D1   2128   // 1174.7 Hz
#define DF1  2255   // 1108.7 Hz
#define C    2389   // 1046.5 Hz
#define B    2531   // 987.8 Hz
#define BF   2681   // 932.3 Hz
#define A    2841   // 880 Hz
#define AF   3010   // 830.6 Hz
#define G    3189   // 784 Hz
#define GF  3378   // 740 Hz
#define F   3579   // 698.5 Hz
#define E   3792   // 659.3 Hz
#define EF  4018   // 622.3 Hz
#define D   4257   // 587.3 Hz
#define DF  4510   // 554.4 Hz
#define C0  4778   // 523.3 Hz
#define B0  5062   // 493.9 Hz
#define BF0 5363   // 466.2 Hz
#define A0  5682   // 440 Hz
#define AF0 6020   // 415.3 Hz
#define G0  6378   // 392 Hz
#define GF0 6757   // 370 Hz
#define F0  7159   // 349.2 Hz
#define E0  7584   // 329.6 Hz
#define EF0 8035   // 311.1 Hz
#define D0  8513   // 293.7 Hz
#define DF0 9019   // 277.2 Hz
#define C7  9556   // 261.6 Hz
#define B7  10124   // 246.9 Hz
#define BF7 10726   // 233.1 Hz
#define A7  11364   // 220 Hz
#define AF7 12039   // 207.7 Hz
#define G7  12755   // 196 Hz
#define GF7 13514   // 185 Hz
#define F7  14317   // 174.6 Hz
#define E7  15169   // 164.8 Hz
#define EF7 16071   // 155.6 Hz
#define D7  17026   // 146.8 Hz
#define DF7 18039   // 138.6 Hz
#define C6  19111   // 130.8 Hz

//void DisableInterrupts(void); // Disable interrupts
//void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void (*PeriodicTask)(void);   // user function

unsigned short wave[32] = 
    {8,9,11,12,13,14,14,15,15,15,14,14,13,12,11,9,8,7,5,4,3,2,2,1,1,1,2,2,3,4,5,7};
unsigned short Trumpet[32] = {
  10,11,11,12,10,8,3,1,8,15,15,	
  11,10,10,11,10,10,10,10,10,10,10,	
  10,11,11,11,11,11,11,10,10,10	
};
unsigned short Bassoon[32] = {  				
  8,8,8,8,7,7,9,15,10,1,1,				
  4,8,11,10,6,3,2,6,10,8,5,				
  5,5,6,7,7,9,8,8,8,7				
};  				
unsigned short Flute[64] = {  	
  6,7,9,9,10,11,12,13,13,14,15,	
  15,15,15,15,14,13,13,12,11,10,9,	
  8,7,7,6,6,5,5,5,5,4,	
  4,4,4,4,4,3,3,3,3,3,	
  2,2,1,1,1,1,0,0,0,0,	
  0,0,1,1,1,2,2,3,3,4,4,5	
};  	
unsigned short Guitar[32] = {  				
  5,5,4,1,1,3,8,11,11,9,4,				
  2,5,11,15,13,9,7,5,5,6,8,				
  8,7,4,3,3,3,3,4,5,5				
};  				
unsigned short Horn[64] = {  	
  7,8,8,8,8,9,10,12,15,15,15,	
  13,10,7,4,3,3,3,3,3,3,3,	
  4,4,4,4,5,6,7,8,8,9,	
  9,10,11,11,12,13,13,12,12,13,	
  14,12,11,9,8,6,3,2,1,1,	
  0,1,1,1,2,2,3,4,4,6,7,7	
};  	
unsigned short Silence[32] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0};


typedef struct{
    const uint32_t pitch;
    const uint32_t duration;
    unsigned short *voice;
}note;

uint32_t tempo = 100; //bpm
#define	sixteenth	9557523
#define	eighth	20176992
#define	quarter	41415930
#define	half	83893806
#define	whole	168849558
#define	quarterTrip	13097346
#define	halfTrip	27256638
#define	dotEighth	30796461
#define	dotQuarter	62654868
#define gap 1061946

note song[] ={      {BF7, sixteenth, Guitar}, // Ne
                    {A, gap, Silence},
                    {C7, sixteenth, Guitar},  // ver
                    {A, gap, Silence},
                    {DF0, sixteenth, Guitar}, // gon
                    {A, gap, Silence},
                    {BF7, sixteenth, Guitar}, // na
                    {A, gap, Silence},
                    {F0, dotEighth, Guitar},  // give
                    {A, gap, Silence},
                    {F0, dotEighth, Guitar},  // you
                    {A, gap, Silence},
                    {EF0, dotQuarter, Guitar},// up
                    {A, gap, Silence},
                    {AF7, sixteenth, Guitar}, // Ne
                    {A, gap, Silence},
                    {BF7, sixteenth, Guitar}, // ver
                    {A, gap, Silence},
                    {C7, sixteenth, Guitar},  // gon
                    {A, gap, Silence},
                    {AF7, sixteenth, Guitar}, // na
                    {A, gap, Silence},
                    {EF0, dotEighth, Guitar}, // let
                    {A, gap, Silence},
                    {EF0, dotEighth, Guitar}, // you
                    {A, gap, Silence},
                    {DF0, dotEighth, Guitar}, // do
                    {A, gap, Silence},
                    {C7, sixteenth, Guitar},  // w
                    {A, gap, Silence},
                    {BF7, eighth, Guitar},   // n
                    {A, gap, Silence},
                    {BF7, sixteenth, Guitar}, // Ne
                    {A, gap, Silence},
                    {C7, sixteenth, Guitar},  // ver
                    {A, gap, Silence},
                    {DF0, sixteenth, Guitar}, // gon
                    {A, gap, Silence},
                    {BF7, sixteenth, Guitar}, // na
                    {A, gap, Silence},
                    {DF0, quarter, Guitar},   // run
                    {A, gap, Silence},
                    {EF0, eighth, Guitar},    // a
                    {A, gap, Silence},
                    {C7, eighth, Guitar},     // rou
                    {A, gap, Silence},
                    {BF7, eighth, Guitar},    // nd
                    {A, gap, Silence},
                    {AF7, quarter, Guitar},   // and
                    {A, gap, Silence},
                    {F7, eighth, Guitar},     // de
                    {A, gap, Silence},
                    {EF0, quarter, Guitar},   // sert
                    {A, gap, Silence},
                    {DF0, half, Guitar},      // you
                    
                    {BF7, sixteenth, Guitar}, // Ne
                    {A, gap, Silence},
                    {C7, sixteenth, Guitar},  // ver
                    {A, gap, Silence},
                    {DF0, sixteenth, Guitar}, // gon
                    {A, gap, Silence},
                    {BF7, sixteenth, Guitar}, // na
                    {A, gap, Silence},
                    {F0, dotEighth, Guitar},  // make
                    {A, gap, Silence},
                    {F0, dotEighth, Guitar},  // you
                    {A, gap, Silence},
                    {EF0, dotQuarter, Guitar},// cry
                    {A, gap, Silence},
                    {AF7, sixteenth, Guitar}, // Ne
                    {A, gap, Silence},
                    {BF7, sixteenth, Guitar}, // ver
                    {A, gap, Silence},
                    {C7, sixteenth, Guitar},  // gon
                    {A, gap, Silence},
                    {AF7, sixteenth, Guitar}, // na
                    {A, gap, Silence},
                    {AF0, quarter, Guitar},   // say
                    {A, gap, Silence},
                    {C7, eighth, Guitar},      // good
                    {A, gap, Silence},
                    {DF0, dotEighth, Guitar}, // b
                    {A, gap, Silence},
                    {C7, sixteenth, Guitar},  // y
                    {A, gap, Silence},
                    {BF7, eighth, Guitar},   // e
                    {A, gap, Silence},
                    {AF7, sixteenth, Guitar}, // Ne
                    {A, gap, Silence},
                    {BF7, sixteenth, Guitar}, // ver
                    {A, gap, Silence},
                    {C7, sixteenth, Guitar},  // gon
                    {A, gap, Silence},
                    {AF7, sixteenth, Guitar}, // na
                    {A, gap, Silence},
                    {DF0, quarter, Guitar},   // tell
                    {A, gap, Silence},
                    {EF0, eighth, Guitar},    // a
                    {A, gap, Silence},
                    {C7, eighth, Guitar},     // l
                    {A, gap, Silence},
                    {BF7, eighth, Guitar},    // i
                    {A, gap, Silence},
                    {AF7, quarter, Guitar},   // e
                    {A, gap, Silence},
                    {F7, eighth, Guitar},     // and
                    {A, gap, Silence},
                    {EF0, quarter, Guitar},   // hurt
                    {A, gap, Silence},
                    {DF0, half, Guitar},      // you
                    {0, whole, wave}
                   };

note *currentNote;
uint32_t songIndex;
            
                   
// ***************** Timer0A_Init ****************
// Activate TIMER0 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq), 32 bits
// Outputs: none
void Timer0A_Init(uint32_t period){
  long sr;
  volatile int delay;
  sr = StartCritical(); 
  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
  delay = 0;
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER0_TAILR_R = period-1;    // 4) reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
  EndCritical(sr);
}

void disableTimer(void) {
   TIMER0_IMR_R = 0x00000000; 
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer0A timeout
  if(currentNote->pitch == 0) {
     Sound_Play(0,wave);
      disableTimer();
  }
  else if(!(GPIO_PORTF_DATA_R & 0x010)) { //if button pressed
      Sound_Play(currentNote->pitch, currentNote->voice);
      TIMER0_TAILR_R = currentNote->duration;
      songIndex++;
      currentNote = &song[songIndex];
  }
}

void Song_Play(void) {
    songIndex = 0;
    currentNote = &song[songIndex];
    Timer0A_Init(currentNote->duration);
    Timer0A_Handler();
    while(!(GPIO_PORTF_DATA_R & 0x010)); //let run until button released
    disableTimer();
    Sound_Play(0,wave);
}
