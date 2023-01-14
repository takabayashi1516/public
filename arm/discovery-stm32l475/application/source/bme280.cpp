/**
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>

#include <peripheral_interface.h>
#include <bme280.h>


/**
 *
 */
CBme280::CBme280(CI2cBus *a_pobjI2c)
	:	CHandlerBase(~0u, 256u, 16u, 6u, NULL, true),
		m_pobjI2c(a_pobjI2c), m_objExecGetInfo(this)
{
	signalPrepare();
	waitStartUp();
}

/**
 *
 */
CBme280::~CBme280()
{
}

/**
 *
 */
int CBme280::onInitialize()
{
	::printf("l(%4d): %s\n", __LINE__, __PRETTY_FUNCTION__);

	int result = initDevice();
	(void) registerExec(ERequestGetInfo, &m_objExecGetInfo);
	CThreadInterfaceBase::onInitialize();
	return result;
}

/**
 *
 */
int CBme280::initDevice()
{
	int result;

	/* device initialize */
	uint8_t data = 0xb6;
	result = writeRegister(BME280_REG_RESET, &data, sizeof(data));
	if (result) {
		return result;
	}

	int len;
	result = readRegister(BME280_REG_ID, &data, &len, sizeof(data));
	if (result) {
		return result;
	}

	data = BME280_REG_CTRL_HUM_VAL;
	result = writeRegister(BME280_REG_CTRL_HUM, &data, sizeof(data));
	if (result) {
		return result;
	}

	data = BME280_REG_CTRL_MEAS_VAL;
	result = writeRegister(BME280_REG_CTRL_MEAS, &data, sizeof(data));

	return result;
}

/**
 *
 */
int CBme280::readInfo(CInfo *data)
{
	return requestSync(ERequestGetInfo, &data, sizeof(data));
}

/**
 *
 */
int32_t CBme280::compensateTemperature(int32_t adc_T,
		CRawCalib *calib, int32_t *t_fine)
{
	long var1, var2, T;
	var1 = ((((adc_T >> 3) - ((long)(calib->dig_T1) << 1))) * ((long)(calib->dig_T2))) >> 11;
	var2 = (((((adc_T >> 4) - ((long)(calib->dig_T1))) * ((adc_T >> 4) -
			((long)(calib->dig_T1)))) >> 12) * ((long)(calib->dig_T3))) >> 14;

	*t_fine = var1 + var2;
	T = ((*t_fine) * 5 + 128) >> 8;
	return T;
}

/**
 *
 */
uint32_t CBme280::compensatePressure(int32_t adc_P,
		CRawCalib *calib, int32_t t_fine)
{
	long var1, var2;
	unsigned long P;
	var1 = (((long)t_fine) >> 1) - (long)64000;
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((long)(calib->dig_P6));
	var2 = var2 + ((var1*((long)(calib->dig_P5))) << 1);
	var2 = (var2 >> 2)+(((long)(calib->dig_P4)) << 16);
	var1 = ((((calib->dig_P3) * (((var1>>2)*(var1>>2)) >> 13)) >>3) + ((((long)(calib->dig_P2)) * var1)>>1))>>18;
	var1 = ((((32768+var1))*((long)(calib->dig_P1)))>>15);
	if (var1 == 0) {
		return 0;
	}
	P = (((unsigned long)(((long)1048576)-adc_P)-(var2>>12)))*3125;
	if (P < 0x80000000) {
		P = (P << 1) / ((unsigned long) var1);
	} else {
		P = (P / (unsigned long)var1) * 2;
	}
	var1 = (((long)(calib->dig_P9)) * ((long)(((P>>3) * (P>>3))>>13)))>>12;
	var2 = (((long)(P>>2)) * ((long)(calib->dig_P8)))>>13;
	P = (unsigned long)((long)P + ((var1 + var2 + (calib->dig_P7)) >> 4));
	return P;
}

/**
 *
 */
uint32_t CBme280::compensateHumidity(int32_t adc_H,
		CRawCalib *calib, int32_t t_fine)
{
	long v_x1;
	v_x1 = (t_fine - ((long)76800));
	v_x1 = (((((adc_H << 14) -(((long)(calib->dig_H4)) << 20) - (((long)(calib->dig_H5)) * v_x1)) +
			((long)16384)) >> 15) * (((((((v_x1 * ((long)(calib->dig_H6))) >> 10) *
			(((v_x1 * ((long)(calib->dig_H3))) >> 11) + ((long) 32768))) >> 10) + ((long)2097152)) *
			((long)(calib->dig_H2)) + 8192) >> 14));
	v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * ((long)(calib->dig_H1))) >> 4));
	v_x1 = (v_x1 < 0 ? 0 : v_x1);
	v_x1 = (v_x1 > 419430400 ? 419430400 : v_x1);
	return (unsigned long)(v_x1 >> 12);
}

/**
 *
 */
int CBme280::writeRegister(uint8_t addr, uint8_t *data, int len)
{
	int result;
	uint8_t *p = new uint8_t[len + 1];
	assert_param(p);
	if (!p) {
		return -1;
	}
	p[0] = addr;
	::memcpy(&p[1], data, len);
	len++;
	result = m_pobjI2c->transmit(BME280_SLAVE_ADDRESS, p, len);
	if (result != 0) {
		result = -1;
		goto free_mem;
	}

	result = 0;

free_mem:
	delete [] p;
	return result;
}

/**
 *
 */
int CBme280::readRegister(uint8_t addr, uint8_t *buff, int *len, int req_len)
{
	int result;

	result = m_pobjI2c->receive(BME280_SLAVE_ADDRESS, &addr, 1, buff, req_len);
	if (result != 0) {
		result = -1;
		return result;
	}
	*len = req_len;
	result = 0;

	return result;
}

/**
 *
 */
int CBme280::readCalib(CRawCalib *calib)
{
	int len;
	int result;

	CRawCalib_1st raw1;
	CRawCalib_2nd raw2;
	CRawCalib_3rd raw3;

	result = readRegister(0x88, (uint8_t *)&raw1,
			&len, sizeof(raw1));
	if (result < 0) {
		return result;
	}

	result = readRegister(0xa1, (uint8_t *)&raw2,
			&len, sizeof(raw2));
	if (result < 0) {
		return result;
	}

	result = readRegister(0xe1, (uint8_t *)&raw3,
			&len, sizeof(raw3));
	if (result < 0) {
		return result;
	}

	calib->dig_T1 = HSWAP(raw1.dig_T1);
	calib->dig_T2 = (int16_t)HSWAP(raw1.dig_T2);
	calib->dig_T3 = (int16_t)HSWAP(raw1.dig_T3);
	calib->dig_P1 = HSWAP(raw1.dig_P1);
	calib->dig_P2 = (int16_t)HSWAP(raw1.dig_P2);
	calib->dig_P3 = (int16_t)HSWAP(raw1.dig_P3);
	calib->dig_P4 = (int16_t)HSWAP(raw1.dig_P4);
	calib->dig_P5 = (int16_t)HSWAP(raw1.dig_P5);
	calib->dig_P6 = (int16_t)HSWAP(raw1.dig_P6);
	calib->dig_P7 = (int16_t)HSWAP(raw1.dig_P7);
	calib->dig_P8 = (int16_t)HSWAP(raw1.dig_P8);
	calib->dig_P9 = (int16_t)HSWAP(raw1.dig_P9);

	calib->dig_H1 = raw2.dig_H1;

	calib->dig_H2 = (int16_t)HSWAP(raw3.dig_H2);
	calib->dig_H3 = raw3.dig_H3;
	calib->dig_H4 = (int16_t)(((raw3.dig_H4_E4) << 4) | (raw3.dig_H4_E5));
	calib->dig_H5 = (int16_t)(((raw3.dig_H5_E6) << 4) | (raw3.dig_H5_E5));
	calib->dig_H6 = (int8_t)raw3.dig_H6;

	return result;
}

/**
 *
 */
int CBme280::CExecGetInfo::doExec(const void *a_lpParam, const int a_cnLength)
{
	uint8_t bytes[32];
	int result;
	int len;
	CRawCalib calib;
	int32_t t_fine;
	CBme280::CInfo *data = *reinterpret_cast<CBme280::CInfo **>(const_cast<void *>(a_lpParam));

	bytes[0] = BME280_REG_CTRL_MEAS_VAL;
	result = m_pobjOwner->writeRegister(BME280_REG_CTRL_MEAS, bytes, 1);
	if (result < 0) {
		return result;
	}

	do {
		bytes[0] = 0xff;
		result = m_pobjOwner->readRegister(BME280_REG_STATUS,
				(uint8_t *)&bytes[0], &len, 1);
		if (result < 0) {
			return result;
		}
	} while ((bytes[0] & BME280_REG_STATUS_MERSURING_MSK));

	result = m_pobjOwner->readCalib(&calib);
	if (result < 0) {
		return result;
	}

	/* 0xFA temp_msb[7:0]
	     Contains the MSB part ut[19:12] of the raw temperature
	     measurement output data.
	   0xFB temp_lsb[7:0]
	     Contains the LSB part ut[11:4] of the raw temperature
	     measurement output data.
	   0xFC (bit 7, 6, 5, 4) temp_xlsb[3:0]
	     Contains the XLSB part ut[3:0] of the raw temperature
	     measurement output data. Contents depend on pressure resolution. */
	result = m_pobjOwner->readRegister(BME280_REG_TEMP_MSB, bytes, &len, 3);
	if (result < 0) {
		return result;
	}
	data->temp = ((((uint32_t)(bytes[0])) << 12) |
			(((uint32_t)(bytes[1])) << 4) | (((uint32_t)(bytes[2])) >> 4));

	/* 0xF7 press_msb[7:0]
	     Contains the MSB part up[19:12] of the raw pressure
	     measurement output data.
	   0xF8 press_lsb[7:0]
	     Contains the LSB part up[11:4] of the raw pressure
	     measurement output data.
	   0xF9 (bit 7, 6, 5, 4) press_xlsb[3:0]
	     Contains the XLSB part up[3:0] of the raw pressure
	     measurement output data. Contents depend on temperature resolution. */
	result = m_pobjOwner->readRegister(BME280_REG_PRESS_MSB, bytes, &len, 3);
	if (result < 0) {
		return result;
	}
	data->press = ((((uint32_t)(bytes[0])) << 12) |
			(((uint32_t)(bytes[1])) << 4) | (((uint32_t)(bytes[2])) >> 4));

	/* 0xFD hum_msb[7:0]
	     Contains the MSB part uh[15:8] of the raw humidity
	     measurement output data.
	   0xFE temp_lsb[7:0]
	     Contains the LSB part uh[7:0] of the raw humidity
	     measurement output data. */
	result = m_pobjOwner->readRegister(BME280_REG_HUM_MSB, bytes, &len,
			sizeof(uint16_t));
	if (result < 0) {
		return result;
	}
	data->hum = ((((uint32_t)(bytes[0])) << 8) | ((uint32_t)(bytes[1])));

	data->temp = m_pobjOwner->compensateTemperature(data->temp, &calib, &t_fine);
	data->press = m_pobjOwner->compensatePressure(data->press, &calib, t_fine);
	data->hum = m_pobjOwner->compensateHumidity(data->hum, &calib, t_fine);

	return result;
}
