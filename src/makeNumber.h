#include <Arduino.h>

extern void makeNumber( uint8_t *number, char _c, uint8_t min, uint8_t max ) ;
//extern bool makingNumber ;
extern bool makeNumberTimeout() ;
extern void abortMakeNumber() ;
extern void updateNumber() ;
// extern void printNumber(uint8_t) __attribute__ ((weak)) ;
// extern void notifyNumberEntered() __attribute__ ((weak)) ;