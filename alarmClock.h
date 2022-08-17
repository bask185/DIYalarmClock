enum alarmClockStates
{
    alarmClockIDLE,
    setTime,
    setAlarm,
    idle,
    boot,
    setDate
} ;

extern uint8_t alarmClock(void) ; 
extern void alarmClockInit(void) ; 
