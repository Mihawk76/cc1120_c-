#ifndef __CC1120DRV_H__
#define __CC1120DRV_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _CC1120_COMM_CMD {
	COMM_JOINT = 1,
	COMM_SCAN,
	COMM_UNJOINT,
	COMM_INFO,
	COMM_SYNC,
	COMM_ADD,
	COMM_QUERY,

	COMM_JOINT_RPL = 0x81,
	COMM_SCAN_RPL,
	COMM_UNJOINT_RPL,
	COMM_INFO_RPL,
	COMM_SYNC_RPL,
	COMM_ADD_RPL,
	COMM_QUERY_RPL,

	COMM_SEND_IR = 0x11,
	COMM_VALUES_GET,
	COMM_ADE_GET,
	COMM_ADE_SET,
	COMM_UTILS_SET,
	
	COMM_GW_TO_TH = 0x41,
	
	COMM_SEND_IR_RPL = 0x91,
	COMM_VALUES_RPL,
	COMM_ADE_GET_RPL,
	COMM_ADE_SET_RPL,
	COMM_UTILS_RPL,
	
	COMM_TH_TO_GW = 0xC1,
	
} CC1120_COMM_CMD;

int  cc112x_init(uint8_t freq, uint8_t pwr);
int trxReadWriteBurstSingle(uint8_t addr, uint8_t *pData, uint16_t len);
uint8_t trxSpiCmdStrobe(uint8_t cmd);
uint8_t trx8BitRegAccess(uint8_t accessType, uint8_t addrByte, uint8_t *pData, uint16_t len);
uint8_t trx16BitRegAccess(uint8_t accessType, uint8_t extAddr, uint8_t regAddr, uint8_t *pData, uint8_t len);
uint8_t cc112xSpiReadReg(uint16_t addr, uint8_t *pData, uint8_t len);
uint8_t cc112xSpiWriteReg(uint16_t addr, uint8_t *pData, uint8_t len);
void cc112xregConfig(uint8_t freq);
int cc112xConfigChek(uint8_t freq);
void manualCalibration(void);
uint8_t cc112xSpiWriteTxFifo(uint8_t *pData, uint8_t len);
uint8_t cc112xSpiReadRxFifo(uint8_t * pData, uint8_t len);
uint8_t cc112xGetTxStatus(void);
uint8_t cc112xGetRxStatus(void);
void cc112x_hw_rst(void);
void cc112xpwrConfig(uint8_t pwrTx);
uint8_t wait_exp_val(uint16_t addr, uint8_t exp_val);

#ifdef __cplusplus
}
#endif

#endif //#ifndef __CC1120_H__
