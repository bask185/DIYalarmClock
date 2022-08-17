#include "makeNumber.h"

uint32_t	prevTime ;
const int 	keypadInterval = 666 ;
bool		makingNumber ;


/**
 * @brief Function to alter and assemble numbers with
 *
 * @param address of uint8_t, char in range ('0', '9'), maximum value
 *
 * @return N/A
 */
void makeNumber( uint8_t *number, char _c, uint8_t min, uint8_t max )
{
	if( _c < '0' || _c > '9') return ; // discard strange characters
	
	uint8_t c = _c - '0' ;
	uint32_t currTime = millis() ;

	makingNumber = true ;
	
	*number *= 10 ;
	*number += c ;
	
	if( *number >= max + 1 )
	{
		if( c >= max )
		{
			*number = max ;
		}
		else
		{
			*number = c ;
		}
	}
	if( *number <= min ) *number = min ;

	prevTime = currTime ;

	//if(printNumber) printNumber( *number ) ; // wanted?
}

void abortMakeNumber()
{
	makingNumber = false ;	// user may abort entering a new nnumber
}

void updateNumber()
{
	if( makingNumber == true  && ( millis() - prevTime >= keypadInterval ) )
	{	makingNumber = false ;

		//if( notifyNumberEntered ) notifyNumberEntered() ;
	}
}