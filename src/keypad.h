#include <Arduino.h>
#include "macros.h"

#define RED 'Y'
#define GREEN 'N'
#define MENU 'E'


#define POINTS 'p'
#define PROGRAM 'P'
#define DIRECTION 'D'

#define LOCO 'l'


extern void updateKeypad() ;

extern void notifyKeyPressed( uint8 pressTime, char key ) __attribute__((weak)) ;
