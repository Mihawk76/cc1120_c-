#include <inttypes.h>
#include <cstring>
#include <wiringPi.h>

#include "cc1120.hpp"
#include "cc1120Drv.h"


using namespace std;


/*
 *   int init(int freq, int txPower)
 *   return : 0 = init success
 */
int cc1120_T::init(unsigned char freq, unsigned char txPower, unsigned char cs)
{
	freqSet = freq;
	txPowerSet = txPower;
	wiringPiSetup();


	pinMode(28, OUTPUT) ;
	digitalWrite(28,1);
	digitalWrite(29,1);


	return 0;
}


int cc1120_T::rxRssiGet(unsigned char cs)
{
	return 0;
}

int cc1120_T::dataWrite(unsigned char *buffer, int length, unsigned char cs)
{
	return 0;
}

int cc1120_T::dataRead(unsigned char *buffer, unsigned char cs)
{
	return 0;
}

void cc1120_T::rxFlush(unsigned char cs){

	memset(rxBuffer, cs, sizeof rxBuffer);
}

void cc1120_T::txFlush(unsigned char cs){
	memset(txBuffer, cs, sizeof txBuffer);
}
