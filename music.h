#include <stdint.h>

extern unsigned short wave[32];
extern unsigned short Trumpet[32];

void Timer0A_Init(uint32_t period);

void Timer0A_Handler(void);

void Song_Play(void);
