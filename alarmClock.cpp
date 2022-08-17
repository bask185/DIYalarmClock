/*      alarm clock things todo

- alter number editing to work with constrained values, for instance, 
  you cannot enter 32 hours or 26 hours,  42 days, 9th day of week etc



*/
// HEADER FILES
#include <Arduino.h>
#include "alarmClock.h"
#include "src/macros.h"
#include "src/io.h"
#include "src/stateMachineClass.h"
#include <Wire.h>
#include <LiquidCrystal.h>

// #include i2c liquid crytal library
#include "src/debounceClass.h"
#include "src/keypad.h"
#include "src/makeNumber.h"


#define entryState  if( sm.entryState() )
#define onState     if( sm.onState() )
#define exitState   if( sm.exitState() == 0 ) return 0 ; \
                    else
static StateMachine sm ;

#define beginState boot
#ifndef beginState
#error beginState not defined
#endif

// VARIABLES
bool    alarmEnabled ;      // true when alarm is set
uint8   index ;             // used to set numbers of time and date
uint8   *pointer ;          // used to alter digits
uint8   digit[10] ;         // holds low and high bytes of up to 5 different variables
uint8   maxDigit ;


// LCD
LiquidCrystal lcd(12, 11, 10, 5, 4, 3, 2); // replace me by I2C variant

// RTC
const int DS3231 = 0x68 ;
enum ds3231Registers
{
    SECONDS = 0x00 ,        //  0 - 59
    MINUTES ,               //  0 - 59
    HOUR ,                  //  0 - 23

    DAY_OF_WEEK ,           //  1 -  7
    DAY ,                   //  1 - 32
    MONTH ,                 //  1 - 12
    YEAR ,                  //  0 - 99

    ALARM_SECONDS ,         //  0 - 59
    ALARM_MINUTES ,         //  0 - 59
    ALARM_HOUR ,            //  0 - 23

    ALARM_DAY ,             //  1 -  7
    ALARM_DATE ,            //  1 - 32
} ;


/************ KEYPAD *****************/

char key ;

void notifyKeyPressed( uint8 X, uint8 newKey )      // called from updateKeypad()
{
    key = newKey ;
}




/************ TIME *****************/
typedef struct timeObj
{
    uint8   second ;
    uint8   minute ;
    uint8   hour ;
} Time ;

Time    current ;
Time    alarm ;

void getTime( Time *time, uint8 type ) // 0 is normal time, other is alarm time
{
    Wire.beginTransmission( DS3231 ) ;
    if( type == 0 ) Wire.write( SECONDS ) ;
    else            Wire.write( ALARM_SECONDS ) ;
    Wire.endTransmission() ;

    Wire.requestFrom( DS3231, 3 ) ;
    time -> second = Wire.read() & 0x7F ; // add bcd to dec? (or not...)
    time -> minute = Wire.read() & 0x7F ;
    time -> hour   = Wire.read() & 0x7F ;
}

void storeTime( Time *time, uint8 type ) // 0 is normal time, other is alarm time
{
    Wire.beginTransmission( DS3231 ) ;
    if( type == 0 ) Wire.write( SECONDS ) ;
    else            Wire.write( ALARM_SECONDS ) ;

    Wire.write(time -> second ) ; 
    Wire.write(time -> minute ) ; 
    Wire.write(time -> hour ) ; 
    Wire.endTransmission() ;
}

void displayTime( Time *time )
{
    lcd.setCursor( 0, 0 ) ;
    lcd.print( time -> hour    >> 0x04 ) ; // high digit
    lcd.print( time -> hour     & 0x0F ) ; //  low digit
    lcd.print('/') ;
    lcd.print( time -> minute  >> 0x04 ) ;
    lcd.print( time -> minute   & 0x0F ) ;
    lcd.print('/') ;
    lcd.print( time -> second  >> 0x04 ) ;
    lcd.print( time -> second   & 0x0F ) ;
}


/************ DATE *****************/
typedef struct dateObj
{
    uint8   dayOfWeek ;
    uint8   day ;
    uint8   month ;
    uint8   year ;
} Date ;

Date    currentDate ;

void getDate( Date *date )
{
    Wire.beginTransmission( DS3231 ) ;
    Wire.write( DAY_OF_WEEK ) ;
    Wire.endTransmission() ;

    Wire.requestFrom( DS3231, 4 ) ;
    date -> dayOfWeek   = Wire.read() ; // add bcd to dec?
    date -> day         = Wire.read() ;
    date -> month       = Wire.read() & 0x1F ;
    date -> year        = Wire.read() ;
}

void storeDate( Date *date ) // 0 is normal time, other is alarm time
{
    Wire.beginTransmission( DS3231 ) ;
    Wire.write( DAY_OF_WEEK ) ;
    Wire.write( date -> dayOfWeek ) ; 
    Wire.write( date -> day ) ; 
    Wire.write( date -> month ) ; 
    Wire.write( date -> year ) ; 
    Wire.endTransmission() ;
}

void displayDate()
{
    lcd.setCursor( 0, 1 ) ;
    lcd.print( currentDate.day   >> 0x04 ) ; // high digit
    lcd.print( currentDate.day    & 0x0F ) ; //  low digit
    lcd.print('/') ;
    lcd.print( currentDate.month >> 0x04 ) ;
    lcd.print( currentDate.month  & 0x0F ) ;
    lcd.print('/') ;
    lcd.print( currentDate.year  >> 0x04 ) ;
    lcd.print( currentDate.year   & 0x0F ) ;
}

/************ UTILITY STUFF ****************/

bool checkAlarm()
{
    if( alarmEnabled   == false         ) return false ;
    if( current.hour   != alarm.hour    ) return false ;
    if( current.minute != alarm.minute  ) return false ;
    if( current.second != 0             ) return false ;

    return true ;
}

extern void alarmClockInit(void)
{
    Wire.begin() ;
    sm.nextState( beginState, 0 ) ;

    getTime( &alarm, 1 ) ;          // upon booting load alarm time from RTC to variables
    getDate( &currentDate ) ;
}

/************STATE FUNCTIONS  ****************/

#ifndef DEBUG
void testKeypad()
{
    if( key )
    {
        lcd.setCursor(0,0);
        lcd.print( key ) ;
    } 

    REPEAT_MS( 500 )
    {
        int sample = analogRead( A0 ) ;
        lcd.setCursor(0,1);
        lcd.clear() ;
        lcd.print( sample ) ;
    }
    END_REPEAT   
}
#endif

StateFunction( boot )
{
    // keyLight on
    // main light off
    // backlight 100%

    lcd.clear() ;
    lcd.print(F("Alarm clock")) ;
    lcd.setCursor( 0, 1 ) ;
    lcd.print(F("Powering up")) ;
    return 1 ;
}

StateFunction( idle )
{
    entryState
    {
        
    }
    onState
    {
        getTime( &current, 0 ) ;
        displayTime( &current ) ;

        if( checkAlarm() )
        {
            REPEAT_MS( 3529 )       // 255 pwm steps devided over 15 minutes
            {
                // incLight() ;
            } END_REPEAT
            // time to get up bruh
        }

        if( key ) sm.exit() ;
    }
    exitState
    {
        
        return 1 ;
    }
}


//  HH:MM:SS
StateFunction( setTime )
{
    entryState
    {
        lcd.clear() ;
        key  = '#' ;
        index  = 1 ;
        
        digit[0]  = current.hour   >> 0x04 ;                                    // load current time in digit array
        digit[1]  = current.hour    & 0x0F ;
        digit[2]  = current.minute >> 0x04 ;
        digit[3]  = current.minute  & 0x0F ;
        current.second = 0 ;
    }
    onState
    {
        if( key ) 
        {
            switch (index)
            {
            case 0: if( digit[1] > 3 )  makeNumber( &digit[0], key, 0, 1 ) ;                // if 2nd digit is 4 or greater, this digit cannot become 2
                    else                makeNumber( &digit[0], key, 0, 2 ) ; 
                break ;

            case 1: if( digit[0] > 1 )  makeNumber( &digit[1], key, 0, 3 ) ;                // if first digit is 2, this digit cannot be higher than 3
                    else                makeNumber( &digit[1], key, 0, 9 ) ;
                break ;

            case 2:                     makeNumber( &digit[2], key, 0, 5 ) ;
                break ;

            case 3:                     makeNumber( &digit[3], key, 0, 9 ) ;
                break ;
            }

            current.hour   = (digit[0] << 4) | digit[1] ;                       // stuff digits back in the appropiate variables
            current.minute = (digit[2] << 4) | digit[3] ;
            displayTime( &current ) ; 

            if( key == '#' )
            {
                if( index > 0 )         index -- ;                              // decrement index for # to correct a wrong press
                else                    sm.exit() ;                             // if # is pressed when index is already 0 -> exit
            }
            else if( ++ index == 4 )    sm.exit() ;                             // if the last digit is entered -> exit                               // if the last digit is entered -> exit

            lcd.setCursor(0,1) ;                                                // update arrow '^'
            lcd.print(F("    ")) ;
            lcd.setCursor(0,index) ;
            lcd.write('^') ;
        }
    }
    exitState
    {
        storeTime( &current, 0 ) ;                                              // store new time
        lcd.clear() ;
        lcd.print(F("Time stored!")) ;
        return 1 ;
    }
}

//  HH:MM:SS
StateFunction( setAlarm )
{
    entryState
    {
        lcd.clear() ;
        key  = '#' ;
        index  = 1 ;
        
        digit[0]  = alarm.hour   >> 0x04 ;                                    // load current time in digit array
        digit[1]  = alarm.hour    & 0x0F ;
        digit[2]  = alarm.minute >> 0x04 ;
        digit[3]  = alarm.minute  & 0x0F ;
        alarm.second = 0 ;
    }
    onState
    {
        if( key ) 
        {
            switch (index)
            {
            case 0: if( digit[1] > 3 )  makeNumber( &digit[0], key, 0, 1 ) ;    // if 2nd digit is 4 or greater, this digit cannot become 2
                    else                makeNumber( &digit[0], key, 0, 2 ) ; break ;   

            case 1: if( digit[0] > 1 )  makeNumber( &digit[1], key, 0, 3 ) ;    // if first digit is 2, this digit cannot be higher than 3
                    else                makeNumber( &digit[1], key, 0, 9 ) ; break ;

            case 2:                     makeNumber( &digit[2], key, 0, 5 ) ; break ;

            case 3:                     makeNumber( &digit[3], key, 0, 9 ) ; break ;
            }

            alarm.hour   = (digit[0] << 4) | digit[1] ;                         // stuff digits back in the appropiate variables
            alarm.minute = (digit[2] << 4) | digit[3] ;
            displayTime( &alarm ) ; 

             if( key == '#' )
            {
                if( index > 0 )         index -- ;                              // decrement index for # to correct a wrong press
                else                    sm.exit() ;                             // if # is pressed when index is already 0 -> exit
            }
            else if( ++ index == 4 )    sm.exit() ;                             // if the last digit is entered -> exit                               // if the last digit is entered -> exit

            lcd.setCursor(0,1) ;                                                // update arrow '^'
            lcd.print(F("     ")) ;
            if( index < 2 ) lcd.setCursor(0,index) ;
            else            lcd.setCursor(0,index + 1) ;
            lcd.write('^') ;
        }
    }
    exitState
    {
        storeTime( &alarm, 1 ) ;                                                // store new time
        lcd.clear() ;
        lcd.print(F("Alarm stored!")) ;

        return 1 ;
    }
}


/*
uint8   dayOfWeek ;
uint8   day ;
uint8   month ;
uint8   year ;
*/
StateFunction( setDate )
{
    entryState
    {
        lcd.clear() ;
        key  = '#' ;
        index  = 1 ;
        
        digit[0]  = currentDate.day   >> 0x04 ;                                        // load current time in digit array
        digit[1]  = currentDate.day    & 0x0F ;
        digit[2]  = currentDate.month >> 0x04 ;
        digit[3]  = currentDate.month  & 0x0F ;
        digit[4]  = currentDate.year  >> 0x04 ;
        digit[5]  = currentDate.year   & 0x0F ;
        digit[6]  = currentDate.dayOfWeek ;
    }
    onState
    {
        if( key ) 
        {
            if( key >= '0' && key <= '9' ) digit[index] = key - '0' ;           // if a number is entered, update a digit

            currentDate.day         = (digit[0] << 4) | digit[1] ;              // stuff digits back in the appropiate variables
            currentDate.month       = (digit[2] << 4) | digit[3] ;
            currentDate.year        = (digit[4] << 4) | digit[5] ;
            currentDate.dayOfWeek   = digit[6] ;
            displayDate() ;

            if( key == '#' && index > 0 ) index -- ;                            // decrement index for # to correct a wrong press

            else if( ++ index == 7 ) sm.exit() ;                                // if the last digit is entered -> exit

            lcd.setCursor(0,1) ;                                                // update arrow '^'
            lcd.print(F("    ")) ;
            if(      index == 6 ) lcd.setCursor(0, 12) ;                        // point to day of week
            else if( index >= 4)  lcd.setCursor(0,index + 2) ;
            else if( index >= 2)  lcd.setCursor(0,index + 1) ; 
            else                  lcd.setCursor(0,index ) ;
            // DD/MM/YY    DAY
            // 01/23/45     6 <> index
            // 01X34X67    12 <> X pos
            lcd.write('^') ;
        }
    }
    exitState
    {
        storeTime( &alarm, 1 ) ;                                                // store new time
        lcd.clear() ;
        lcd.print(F("Date stored!")) ;
        
        return 1 ;
    }
}



// STATE MACHINE
extern uint8_t alarmClock()
{
    updateKeypad() ;


#ifdef DEBUG
    testKeypad() ;
#else    

    // key = getKey() ; not yet exist

    STATE_MACHINE_BEGIN( sm )

    State( boot ) {
        sm.nextState( idle, 1500 ) ; }

    State(idle) {
        if(      key == '#') sm.nextState( setTime,  0 ) ;
        else if( key == '*') sm.nextState( setAlarm, 0 ) ;
        else if( key == 'D') sm.nextState( setDate,  0 ) ;
        else                 sm.nextState( idle,     0 ) ; }

    State(setTime) {
        sm.nextState( idle, 1000 ) ; }

    State(setAlarm) {
        sm.nextState( idle, 1000 ) ; }

    State(setDate) {
        sm.nextState( idle, 1000 ) ; }

    /*
    State( setBackLight )           // to be added
        sm.nextState( idle, 0 ) ; }

    State( setKeypadLight )
        sm.nextState( idle, 0 ) ; }
    */
    STATE_MACHINE_END( sm ) ;
#endif

    key = 0 ; // reset key after each cycle
}
