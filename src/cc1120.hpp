#ifndef __CC1120_CLASS__
#define __CC1120_CLASS__

#define RX_BUFFER_SIZE 256
#define TX_BUFFER_SIZE 256

class cc1120_T {
	
public:
	int init(unsigned char freq, unsigned char txPower, unsigned char cs);
	int rxRssiGet(unsigned char cs);
	int dataWrite(unsigned char *buffer, int length, unsigned char cs);
	
	int dataRead(unsigned char *buffer, unsigned char cs);
	void rxFlush(unsigned char cs);
	void txFlush(unsigned char cs);
	
private:
	unsigned char freqSet;
	unsigned char txPowerSet;
	unsigned char rxBuffer[RX_BUFFER_SIZE];
	unsigned char txBuffer[TX_BUFFER_SIZE];
};

#endif
