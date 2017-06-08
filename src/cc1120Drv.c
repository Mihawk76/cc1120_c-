#define _GNU_SOURCE 1
#include <wiringPiSPI.h>
#include <wiringPi.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cc112x_spi.h"
#include "cc1120Drv.h"
#include "cc112x_easy_link_reg_config.h"


/*******************************************************************************
 * @fn          trxReadWriteBurstSingle
 *
 * @brief       When the address byte is sent to the SPI slave, the next byte
 *              communicated is the data to be written or read. The address
 *              byte that holds information about read/write -and single/
 *              burst-access is provided to this function.
 *
 *              Depending on these two bits this function will write len bytes to
 *              the radio in burst mode or read len bytes from the radio in burst
 *              mode if the burst bit is set. If the burst bit is not set, only
 *              one data byte is communicated.
 *
 *              NOTE: This function is used in the following way:
 *
 *              TRXEM_SPI_BEGIN();
 *              while(TRXEM_PORT_IN & TRXEM_SPI_MISO_PIN);
 *              ...[Depending on type of register access]
 *              trxReadWriteBurstSingle(uint8 addr,uint8 *pData,uint16 len);
 *              TRXEM_SPI_END();
 *
 * input parameters
 *
 * @param       none
 *
 * output parameters
 *
 * @return      int
 */

int trxReadWriteBurstSingle(uint8_t addr, uint8_t *pData, uint16_t len)
{
  int ret;
  uint8_t temp[len];

  /* Communicate len number of bytes: if RX - the procedure sends 0x00 to push bytes from slave*/
  if(addr&RADIO_READ_ACCESS)
  {
    memset(temp,0x00,len);
	wiringPiSPIDataRW(0, pData, len);
	ret = pData[len-1];
  }
  else
  {
	memcpy(temp,pData,len);
	wiringPiSPIDataRW(0, temp, len);
	ret = temp[len-1];
  }


  return ret;
}


uint8_t trxSpiCmdStrobe(uint8_t cmd)
{

  /* Pull CS_N low */
  digitalWrite(CC1120_SSEL,  LOW) ;

  uint8_t tx[] = {
	cmd, 0,
  };

  wiringPiSPIDataRW(0, tx, ARRAY_SIZE(tx));


  /* Pull CS_N high */
  digitalWrite(CC1120_SSEL,  HIGH) ;

  /* return the status byte value */
  return(tx[1]);
}


/*******************************************************************************
 * @fn          trx8BitRegAccess
 *
 * @brief       This function performs a read or write from/to a 8bit register
 *              address space. The function handles burst and single read/write
 *              as specfied in addrByte. Function assumes that chip is ready.
 *
 * input parameters
 *
 * @param       accessType - Specifies if this is a read or write and if it's
 *                           a single or burst access. Bitmask made up of
 *                           RADIO_BURST_ACCESS/RADIO_SINGLE_ACCESS/
 *                           RADIO_WRITE_ACCESS/RADIO_READ_ACCESS.
 * @param       addrByte - address byte of register.
 * @param       pData    - data array
 * @param       len      - Length of array to be read(TX)/written(RX)
 *
 * output parameters
 *
 * @return      chip status
 */
uint8_t trx8BitRegAccess(uint8_t accessType, uint8_t addrByte, uint8_t *pData, uint16_t len)
{
  uint8_t d = accessType|addrByte;
  uint8_t tx[] = {d};

  /* Pull CS_N low*/
  digitalWrite(CC1120_SSEL,  LOW);

  wiringPiSPIDataRW( 0, tx, ARRAY_SIZE(tx));
  trxReadWriteBurstSingle( d, pData, len);

  /* Pull CS_N high */
  digitalWrite( CC1120_SSEL,  HIGH) ;

  /* return the status byte value */
  return pData[0];
}

/******************************************************************************
 * @fn          trx16BitRegAccess
 *
 * @brief       This function performs a read or write in the extended adress
 *              space of CC112X.
 *
 * input parameters
 *
 * @param       accessType - Specifies if this is a read or write and if it's
 *                           a single or burst access. Bitmask made up of
 *                           RADIO_BURST_ACCESS/RADIO_SINGLE_ACCESS/
 *                           RADIO_WRITE_ACCESS/RADIO_READ_ACCESS.
 * @param       extAddr - Extended register space address = 0x2F.
 * @param       regAddr - Register address in the extended address space.
 * @param       *pData  - Pointer to data array for communication
 * @param       len     - Length of bytes to be read/written from/to radio
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
uint8_t trx16BitRegAccess(uint8_t accessType, uint8_t extAddr, uint8_t regAddr, uint8_t *pData, uint8_t len)
{
  /* Pull CS_N low*/
  digitalWrite(CC1120_SSEL,  LOW) ;

  uint8_t d = accessType|extAddr;
  uint8_t tx[] = {d, regAddr};

  wiringPiSPIDataRW(0, tx, ARRAY_SIZE(tx));
  trxReadWriteBurstSingle( d, pData, len);


  /* Pull CS_N high*/
  digitalWrite(CC1120_SSEL,  HIGH) ;

  /* return the status byte value */
  return pData[0];
}



/******************************************************************************
 * @fn          cc112xSpiReadReg
 *
 * @brief       Read value(s) from config/status/extended radio register(s).
 *              If len  = 1: Reads a single register
 *              if len != 1: Reads len register values in burst mode
 *
 * input parameters
 *
 * @param       addr   - address of first register to read
 * @param       *pData - pointer to data array where read bytes are saved
 * @param       len   - number of bytes to read
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
uint8_t cc112xSpiReadReg(uint16_t addr, uint8_t *pData, uint8_t len)
{
  uint8_t tempExt  = (uint8_t)(addr>>8);
  uint8_t tempAddr = (uint8_t)(addr & 0x00FF);
  uint8_t rc=0;

  /* Checking if this is a FIFO access -> returns chip not ready  */
  if((CC112X_SINGLE_TXFIFO<=tempAddr)&&(tempExt==0)) return STATUS_CHIP_RDYn_BM;

  /* Decide what register space is accessed */
  if(!tempExt)
  {
    rc = trx8BitRegAccess((RADIO_BURST_ACCESS|RADIO_READ_ACCESS),tempAddr,pData,len);
  }
  else if (tempExt == 0x2F)
  {
    rc = trx16BitRegAccess((RADIO_BURST_ACCESS|RADIO_READ_ACCESS),tempExt,tempAddr,pData,len);
  }
  return (rc);
}

/******************************************************************************
 * @fn          cc112xSpiWriteReg
 *
 * @brief       Write value(s) to config/status/extended radio register(s).
 *              If len  = 1: Writes a single register
 *              if len  > 1: Writes len register values in burst mode
 *
 * input parameters
 *
 * @param       addr   - address of first register to write
 * @param       *pData - pointer to data array that holds bytes to be written
 * @param       len    - number of bytes to write
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
uint8_t cc112xSpiWriteReg(uint16_t addr, uint8_t *pData, uint8_t len)
{
  uint8_t tempExt  = (uint8_t)(addr>>8);
  uint8_t tempAddr = (uint8_t)(addr & 0x00FF);
  uint8_t rc=0;

  /* Checking if this is a FIFO access - returns chip not ready */
  if((CC112X_SINGLE_TXFIFO<=tempAddr)&&(tempExt==0)) return STATUS_CHIP_RDYn_BM;

  /* Decide what register space is accessed */
  if(!tempExt)
  {
    rc = trx8BitRegAccess((RADIO_BURST_ACCESS|RADIO_WRITE_ACCESS),tempAddr,pData,len);
  }
  else if (tempExt == 0x2F)
  {
    rc = trx16BitRegAccess((RADIO_BURST_ACCESS|RADIO_WRITE_ACCESS),tempExt,tempAddr,pData,len);
  }
  return (rc);
}



/*******************************************************************************
*   @fn         cc112xregConfig
*
*   @brief      Write register settings as given by SmartRF Studio found in
*               cc112x_easy_link_reg_config.h
*
*   @param      freq - frequency channel (0-80)
*
*   @return     none
*/
#define CC112X_FREQ_BASE 0x00668000l   // frequency base is 410Mhz
#define CC112X_FREQ_STEP 0x4000        // frequency step is 1Mhz

void cc112xregConfig(uint8_t freq) {

    uint8_t writeByte;
    uint16_t i;
	uint32_t freq_code;
	freq_code = CC112X_FREQ_BASE + ((uint16_t)freq * CC112X_FREQ_STEP);// original code

    // Reset radio
    trxSpiCmdStrobe(CC112X_SRES);

	delay(100);

	trxSpiCmdStrobe(CC112X_SRES);

    // Write registers to radio
	for(i = 0; i < (sizeof(preferredSettings)/sizeof(registerSetting_t)); i++) {
		writeByte = preferredSettings[i].data;
		if (preferredSettings[i].addr == CC112X_FREQ2) writeByte = (freq_code >> 16) & 0xff;
		else if (preferredSettings[i].addr == CC112X_FREQ1) writeByte = (freq_code >> 8) & 0xff;
		else if (preferredSettings[i].addr == CC112X_FREQ0) writeByte = freq_code & 0xff;
		cc112xSpiWriteReg(preferredSettings[i].addr, &writeByte, 1);
    }
}

/*******************************************************************************
*   @fn         registerConfigChek
*
*   @brief      Write register settings as given by SmartRF Studio found in
*               cc112x_easy_link_reg_config.h
*
*   @param      none
*
*   @return     int (0: all registers config match, 1: one or more registers config didn't match)
*/
int cc112xConfigChek(uint8_t freq) {

    uint8_t readByte;
	  uint8_t cmpByte;
	  //buf[20];
    uint16_t i;
   	uint32_t freq_code;

    freq_code = CC112X_FREQ_BASE + ((uint16_t)freq * CC112X_FREQ_STEP);


    // Read registers to radio
    for(i = 0; i < (sizeof(preferredSettings)/sizeof(registerSetting_t)); i++) {
        readByte = 0x00;
		cmpByte = preferredSettings[i].data;
        cc112xSpiReadReg(preferredSettings[i].addr, &readByte, 1);
		if (preferredSettings[i].addr == CC112X_FREQ2) cmpByte = (freq_code >> 16) & 0xff;
		else if (preferredSettings[i].addr == CC112X_FREQ1) cmpByte = (freq_code >> 8) & 0xff;
		else if (preferredSettings[i].addr == CC112X_FREQ0) cmpByte = freq_code & 0xff;
		if (readByte != cmpByte) return 1;
    }
	return 0;

}




/*******************************************************************************
*   @fn         manualCalibration
*
*   @brief      Calibrates radio according to CC112x errata
*
*   @param      none
*
*   @return     none
*/
#define VCDAC_START_OFFSET 2
#define FS_VCO2_INDEX 0
#define FS_VCO4_INDEX 1
#define FS_CHP_INDEX 2

void manualCalibration(void) {

    uint8_t original_fs_cal2;
    uint8_t calResults_for_vcdac_start_high[3];
    uint8_t calResults_for_vcdac_start_mid[3];
    uint8_t marcstate;
    uint8_t writeByte;
		//FILE *writeFile = fopen("/home/data.log", "a");
		//if (writeFile == NULL)
		//{
    	//printf("Error opening file!\n");
    	//exit(1);
		//}

    // 1) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
    writeByte = 0x00;
    cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);

    // 2) Start with high VCDAC (original VCDAC_START + 2):
    cc112xSpiReadReg(CC112X_FS_CAL2, &original_fs_cal2, 1);
    writeByte = original_fs_cal2 + VCDAC_START_OFFSET;
    cc112xSpiWriteReg(CC112X_FS_CAL2, &writeByte, 1);

    // 3) Calibrate and wait for calibration to be done
    //   (radio back in IDLE state)
    trxSpiCmdStrobe(CC112X_SCAL);

    do {
        cc112xSpiReadReg(CC112X_MARCSTATE, &marcstate, 1);
		printf("marcstate: %02x\r\n", marcstate);
		//fprintf(writeFile, "marcstate: %02x\r\n", marcstate);

    } while (marcstate != 0x41);


    // 4) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with
    //    high VCDAC_START value
    cc112xSpiReadReg(CC112X_FS_VCO2,
                     &calResults_for_vcdac_start_high[FS_VCO2_INDEX], 1);
    cc112xSpiReadReg(CC112X_FS_VCO4,
                     &calResults_for_vcdac_start_high[FS_VCO4_INDEX], 1);
    cc112xSpiReadReg(CC112X_FS_CHP,
                     &calResults_for_vcdac_start_high[FS_CHP_INDEX], 1);

    // 5) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
    writeByte = 0x00;
    cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);

    // 6) Continue with mid VCDAC (original VCDAC_START):
    writeByte = original_fs_cal2;
    cc112xSpiWriteReg(CC112X_FS_CAL2, &writeByte, 1);

    // 7) Calibrate and wait for calibration to be done
    //   (radio back in IDLE state)
    trxSpiCmdStrobe(CC112X_SCAL);

    do {
        cc112xSpiReadReg(CC112X_MARCSTATE, &marcstate, 1);
		printf("marcstate: %02x\r\n", marcstate);
		//fprintf(writeFile, "marcstate: %02x\r\n", marcstate);
    } while (marcstate != 0x41);

    // 8) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained
    //    with mid VCDAC_START value
    cc112xSpiReadReg(CC112X_FS_VCO2,
                     &calResults_for_vcdac_start_mid[FS_VCO2_INDEX], 1);
    cc112xSpiReadReg(CC112X_FS_VCO4,
                     &calResults_for_vcdac_start_mid[FS_VCO4_INDEX], 1);
    cc112xSpiReadReg(CC112X_FS_CHP,
                     &calResults_for_vcdac_start_mid[FS_CHP_INDEX], 1);

    // 9) Write back highest FS_VCO2 and corresponding FS_VCO
    //    and FS_CHP result
    if (calResults_for_vcdac_start_high[FS_VCO2_INDEX] >
        calResults_for_vcdac_start_mid[FS_VCO2_INDEX]) {
        writeByte = calResults_for_vcdac_start_high[FS_VCO2_INDEX];
        cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);
        writeByte = calResults_for_vcdac_start_high[FS_VCO4_INDEX];
        cc112xSpiWriteReg(CC112X_FS_VCO4, &writeByte, 1);
        writeByte = calResults_for_vcdac_start_high[FS_CHP_INDEX];
        cc112xSpiWriteReg(CC112X_FS_CHP, &writeByte, 1);
    } else {
        writeByte = calResults_for_vcdac_start_mid[FS_VCO2_INDEX];
        cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);
        writeByte = calResults_for_vcdac_start_mid[FS_VCO4_INDEX];
        cc112xSpiWriteReg(CC112X_FS_VCO4, &writeByte, 1);
        writeByte = calResults_for_vcdac_start_mid[FS_CHP_INDEX];
        cc112xSpiWriteReg(CC112X_FS_CHP, &writeByte, 1);
    }
		//fclose(writeFile);
}

/*******************************************************************************
 * @fn          cc112xSpiWriteTxFifo
 *
 * @brief       Write pData to radio transmit FIFO.
 *
 * input parameters
 *
 * @param       *pData - pointer to data array that is written to TX FIFO
 * @param       len    - Length of data array to be written
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
uint8_t cc112xSpiWriteTxFifo(uint8_t *pData, uint8_t len)
{
  uint8_t rc;
  rc = trx8BitRegAccess(0x00,CC112X_BURST_TXFIFO, pData, len);
  return (rc);
}

/*******************************************************************************
 * @fn          cc112xSpiReadRxFifo
 *
 * @brief       Reads RX FIFO values to pData array
 *
 * input parameters
 *
 * @param       *pData - pointer to data array where RX FIFO bytes are saved
 * @param       len    - number of bytes to read from the RX FIFO
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
uint8_t cc112xSpiReadRxFifo(uint8_t * pData, uint8_t len)
{
  uint8_t rc;
  rc = trx8BitRegAccess(0x00,CC112X_BURST_RXFIFO, pData, len);
  return (rc);
}

/******************************************************************************
 * @fn      cc112xGetTxStatus(void)
 *
 * @brief   This function transmits a No Operation Strobe (SNOP) to get the
 *          status of the radio and the number of free bytes in the TX FIFO.
 *
 *          Status byte:
 *
 *          ---------------------------------------------------------------------------
 *          |          |            |                                                 |
 *          | CHIP_RDY | STATE[2:0] | FIFO_BYTES_AVAILABLE (free bytes in the TX FIFO |
 *          |          |            |                                                 |
 *          ---------------------------------------------------------------------------
 *
 *
 * input parameters
 *
 * @param   none
 *
 * output parameters
 *
 * @return  rfStatus_t
 *
 */
uint8_t cc112xGetTxStatus(void)
{
    return(trxSpiCmdStrobe(CC112X_SNOP));
}

/******************************************************************************
 *
 *  @fn       cc112xGetRxStatus(void)
 *
 *  @brief
 *            This function transmits a No Operation Strobe (SNOP) with the
 *            read bit set to get the status of the radio and the number of
 *            available bytes in the RXFIFO.
 *
 *            Status byte:
 *
 *            --------------------------------------------------------------------------------
 *            |          |            |                                                      |
 *            | CHIP_RDY | STATE[2:0] | FIFO_BYTES_AVAILABLE (available bytes in the RX FIFO |
 *            |          |            |                                                      |
 *            --------------------------------------------------------------------------------
 *
 *
 * input parameters
 *
 * @param     none
 *
 * output parameters
 *
 * @return    rfStatus_t
 *
 */
uint8_t cc112xGetRxStatus(void)
{
    return(trxSpiCmdStrobe(CC112X_SNOP | RADIO_READ_ACCESS));
}

/******************************************************************************
 *
 *  @fn       cc112x_hw_rst(void)
 *
 *  @brief
 *            Reset cc112x by reset pin.
 *
 *
 *
 * input parameters
 *
 * @param     none
 *
 * output parameters
 *
 * @return    none
 *
 */
void cc112x_hw_rst(void) {
  digitalWrite(CC1120_RST,  LOW) ;
  delay(50);
  digitalWrite(CC1120_RST,  HIGH) ;
}

/******************************************************************************
 *
 *  @fn       cc112xpwrConfig(uint8_t pwrTx)
 *
 *  @brief
 *            Set Tx Power
 *
 *
 *
 * input parameters
 *
 * @param     pwrTx - Tx power value
 *
 * output parameters
 *
 * @return    none
 *
 */
void cc112xpwrConfig(uint8_t pwrTx) {
    uint8_t writeByte;
	writeByte = pwrTx;
    cc112xSpiWriteReg(CC112X_PA_CFG2, &writeByte, 1);
}

/******************************************************************************
 *
 *  @fn       uint8_t wait_exp_val(uint16_t addr, uint8_t exp_val)
 *
 *  @brief
 *            Waiting until register value and expected value is match
 *
 *
 * input parameters

 * @param     addr    - register address
 * @param     exp_val - expected value
 *
 * output parameters
 *
 *
 * @return   register value;
 *
 */
uint8_t wait_exp_val(uint16_t addr, uint8_t exp_val) {
	uint8_t temp_byte = 0;
	do {
		cc112xSpiReadReg(addr, &temp_byte, 1);
	} while (temp_byte != exp_val);
	return temp_byte;
}


/******************************************************************************
 *
 *  @fn       cc112x_init(uint8_t freq, uint8_t pwr)
 *
 *  @brief
 *            Initialize/configure CC112x with given freq channel & tx power
 *
 *
 * input parameters
 *
 * @param     freq    - frequency channel
 * @param     pwr     - tx power
 *
 * output parameters
 *
 *
 * @return   none
 *
 */
int  cc112x_init(uint8_t freq, uint8_t pwr)
{
	uint16_t ii;
	ii = 0;

	wiringPiSetup();
//  pinMode(CC1120_MOSI, SPI_PIN);
//  pinMode(CC1120_MISO, SPI_PIN);
//  pinMode(CC1120_SCLK, SPI_PIN);
	pinMode(CC1120_SSEL, OUTPUT) ;
	pinMode(CC1120_RST, OUTPUT) ;

	wiringPiSPISetup(0, 5000000); //32*1000*1000);
	digitalWrite(CC1120_SSEL,  HIGH) ;

	cc112x_hw_rst();

	do {
		cc112xregConfig(freq);
		if (( ii++ % 5) == 0) {
			/*** CC112X HW Reset ***/
			cc112x_hw_rst();
			printf("CC1120 HW Reset.\r\n");

		}
		if ( ii > 15 ) {
		  printf("CC1120 can not init.\r\n");
		  return 1;
		}

	} while (cc112xConfigChek(freq));

	if (pwr>0x3e) pwr = 0x3e;
	cc112xpwrConfig(0x7f-pwr);

	manualCalibration();

	//memset(txBuffer,0,sizeof(txBuffer));
	//memset(rxBuffer,0,sizeof(rxBuffer));

	trxSpiCmdStrobe(CC112X_SIDLE);

	// Flush RX FIFO
	trxSpiCmdStrobe(CC112X_SFRX);

	// Flush TX FIFO
	trxSpiCmdStrobe(CC112X_SFTX);

	wait_exp_val(CC112X_MARCSTATE, 0x41);

	// Set radio in RX
	trxSpiCmdStrobe(CC112X_SRX);
	return 0;
}
