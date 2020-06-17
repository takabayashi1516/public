/**
 *
 */
#ifndef __BME280_H
#define __BME280_H

#include <stddef.h>
#include <stdint.h> /* READ COMMENT ABOVE. */
#include <framework.h>

#define	HSWAP(x)	(x)

#define	BME280_I2C_CH					(1)
#define	BME280_SLAVE_ADDRESS			(0x76)

#define	BME280_REG_HUM_MSB				(0xfd)
#define	BME280_REG_HUM_LSB				(0xfe)
#define	BME280_REG_TEMP_MSB				(0xfa)
#define	BME280_REG_TEMP_LSB				(0xfb)
#define	BME280_REG_PRESS_MSB			(0xf7)
#define	BME280_REG_PRESS_LSB			(0xf8)

#define	BME280_REG_CONFIG				(0xf5)
#define	BME280_REG_CTRL_MEAS			(0xf4)
#define	BME280_REG_STATUS				(0xf3)
#define	BME280_REG_CTRL_HUM				(0xf2)

#define	BME280_REG_RESET				(0xe0)
#define	BME280_REG_ID					(0xd0)

#define	BME280_REG_CTRL_MEAS_VAL		((1 << 5) | (1 << 2) | (1))
#define	BME280_REG_CTRL_HUM_VAL			(0x01)

#define	BME280_REG_CTRL_MEAS_MODE_MSK	(0x03)
#define	BME280_REG_STATUS_MERSURING_MSK	(0x08)

#ifdef __cplusplus

class CI2cBus;

/**
 */
class CBme280 : public CHandlerBase {
public:
	///
	enum ERequest {
		ERequestGetInfo = 100
	};

public:
	/**
	 */
	class CInfo {
	public:
		uint32_t	temp;
		uint32_t	press;
		uint32_t	hum;
	};

#pragma	pack(1)
	/**
	 */
	class CRawCalib_1st {
	public:
		uint16_t	dig_T1;			// 0x88 / 0x89
		uint16_t	dig_T2;			// 0x8A / 0x8B
		uint16_t	dig_T3;			// 0x8C / 0x8D
		uint16_t	dig_P1;			// 0x8E / 0x8F
		uint16_t	dig_P2;			// 0x90 / 0x91
		uint16_t	dig_P3;			// 0x92 / 0x93
		uint16_t	dig_P4;			// 0x94 / 0x95
		uint16_t	dig_P5;			// 0x96 / 0x97
		uint16_t	dig_P6;			// 0x98 / 0x99
		uint16_t	dig_P7;			// 0x9A / 0x9B
		uint16_t	dig_P8;			// 0x9C / 0x9D
		uint16_t	dig_P9;			// 0x9E / 0x9F
	};

	/**
	 */
	class CRawCalib_2nd {
	public:
		uint8_t		dig_H1;			// 0xA1
	};

	/**
	 */
	class CRawCalib_3rd {
	public:
		uint16_t	dig_H2;			// 0xE1 / 0xE2
		uint8_t		dig_H3;			// 0xE3
		uint8_t		dig_H4_E4;		// 0xE4			dig_H4[11:4]
		uint8_t		dig_H4_E5 : 4;	// 0xE5[3:0]	dig_H4[3:0]
		uint8_t		dig_H5_E5 : 4;	// 0xE5[7:4]	dig_H5[3:0]
		uint8_t		dig_H5_E6;		// 0xE6			dig_H5[11:4]
		uint8_t		dig_H6;			// 0xE7
	};
#pragma	pack()

	/**
	 */
	class CRawCalib {
	public:
		uint16_t	dig_T1;			// 0x88 / 0x89
		int16_t		dig_T2;			// 0x8A / 0x8B
		int16_t		dig_T3;			// 0x8C / 0x8D
		uint16_t	dig_P1;			// 0x8E / 0x8F
		int16_t		dig_P2;			// 0x90 / 0x91
		int16_t		dig_P3;			// 0x92 / 0x93
		int16_t		dig_P4;			// 0x94 / 0x95
		int16_t		dig_P5;			// 0x96 / 0x97
		int16_t		dig_P6;			// 0x98 / 0x99
		int16_t		dig_P7;			// 0x9A / 0x9B
		int16_t		dig_P8;			// 0x9C / 0x9D
		int16_t		dig_P9;			// 0x9E / 0x9F
		uint8_t		dig_H1;			// 0xA1
		int16_t		dig_H2;			// 0xE1 / 0xE2
		uint8_t		dig_H3;			// 0xE3
		int16_t		dig_H4;			// 0xE4 / 0xE5[3:0]
		int16_t		dig_H5;			// 0xE5[7:4] / 0xE6
		int8_t		dig_H6;			// 0xE7
	};

	///
	class CExecGetInfo : public CHandlerBase::CExecBase {
	public:
		///
		CExecGetInfo(CBme280 *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		CBme280	*m_pobjOwner;
	};

public:
	///
	CBme280(CI2cBus *a_pobjI2c);
	///
	virtual ~CBme280();
	///
	virtual int onInitialize();

	///
	int readInfo(CInfo *data);

private:
	///
	int initDevice();
	///
	int32_t compensateTemperature(int32_t adc_T,
			CRawCalib *calib, int32_t *t_fine);
	///
	uint32_t compensatePressure(int32_t adc_P,
			CRawCalib *calib, int32_t t_fine);
	///
	uint32_t compensateHumidity(int32_t adc_H,
			CRawCalib *calib, int32_t t_fine);

	///
	int writeRegister(uint8_t addr, unsigned char *data, int len);
	///
	int readRegister(uint8_t addr, unsigned char *buff, int *len, int req_len);
	///
	int readCalib(CRawCalib *calib);

private:
	///
	CI2cBus			*m_pobjI2c;
	///
	CExecGetInfo	m_objExecGetInfo;
};

#endif /* __cplusplus */

#endif /* __BME280_H */
