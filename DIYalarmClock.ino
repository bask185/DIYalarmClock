#include "src/io.h"
#include "src/date.h"
#include "src/version.h"
#include "src/macros.h"
#include "alarmClock.h"

void setup()
{
    initIO() ;
    Serial.begin( 115200 ) ;
    Serial.println( version ) ;
    Serial.println( date ) ;
    alarmClockInit() ;
}

void loop()
{
	alarmClock();
}