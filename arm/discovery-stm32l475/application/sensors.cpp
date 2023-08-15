#include <stdlib.h>
#include <peripheral_interface.h>
#include <main_task.h>
#include <sensors.h>

static int T_C0 = 0;
static int T_C1 = 0;
static int16_t T_C0_lsb = 0;
static int16_t T_C1_lsb = 0;
static float m = 0;

static uint8_t H_0 = 0;
static uint8_t H_1 = 0;
static int16_t H_0_lsb = 0;
static int16_t H_1_lsb = 0;
static float mh = 0;

/*
$ echo $((0xba >> 1)) | xargs printf '0x%02x\n'
0x5d
*/

void initLPS22hh() {
	uint8_t turnOn[] = {0x10, 0x20};	//The address of the register and the value of the register to turn on the sensor
	CMain::getSingleton()->getI2c(2)->transmit(0x5d, turnOn, sizeof(turnOn));
}

float getPressure() {

	int lsb;
	uint8_t pressXL[] = {0x28, 0x00};
	uint8_t pressL[] = {0x29, 0x00};
	uint8_t pressH[] = {0x2a, 0x00};
	uint8_t data[2];

	CMain::getSingleton()->getI2c(2)->transmit(0x5d, pressXL, sizeof(pressXL));
	CMain::getSingleton()->getI2c(2)->receive(0x5d, NULL, 0, data, sizeof(data));
	lsb = data[0];

	CMain::getSingleton()->getI2c(2)->transmit(0x5d, pressL, sizeof(pressL));
	CMain::getSingleton()->getI2c(2)->receive(0x5d, NULL, 0, data, sizeof(data));
	lsb |= data[0] << 8;

	CMain::getSingleton()->getI2c(2)->transmit(0x5d, pressH, sizeof(pressH));
	CMain::getSingleton()->getI2c(2)->receive(0x5d, NULL, 0, data, sizeof(data));
	lsb |= data[0] << 16;


	if (lsb > 8388607){
		lsb = (lsb - 1);
		lsb = (~lsb);
	}

	return ((float)lsb) / ((float)(4096));
}

/*
$ echo $((0xbe >> 1)) | xargs printf '0x%02x\n'
0x5f
*/

void initHTS221() {
	uint8_t turnOn[] = {0x20, 0x81};//The address of the register and the value of the register to turn on the sensor
    uint8_t data[2];

	CMain::getSingleton()->getI2c(2)->transmit(0x5f, turnOn, sizeof(turnOn));

	uint8_t tempMinAddress[] = {0x32};
	uint8_t tempMaxAddress[] = {0x33};
	//reading low temperature calibration lsb
	CMain::getSingleton()->getI2c(2)->transmit(0x5f, tempMinAddress, sizeof(tempMinAddress));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	T_C0 = data[0];

	//reading high temperature calibration lsb
	CMain::getSingleton()->getI2c(2)->transmit(0x5f, tempMaxAddress, sizeof(tempMaxAddress));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	T_C1 = data[0];

	uint8_t MSB_temp[] = {0x35};
	CMain::getSingleton()->getI2c(2)->transmit(0x5f, MSB_temp, sizeof(MSB_temp));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);

	T_C0 |= ((data[0] & 0x03) << 8);
	T_C1 |= (((data[0] & 0x0c) >> 2) << 8);

	T_C0 = T_C0 >> 3;
	T_C1 = T_C1 >> 3;

	uint8_t ADC0L[] = {0x3c};
	uint8_t ADC0H[] = {0x3d};
	//get the calibration adc min
	CMain::getSingleton()->getI2c(2)->transmit(0x5f, ADC0L, sizeof(ADC0L));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	T_C0_lsb = data[0];

	CMain::getSingleton()->getI2c(2)->transmit(0x5f, ADC0H, sizeof(ADC0H));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	T_C0_lsb |= data[0] << 8;

	uint8_t ADC1L[] = {0x3e};
	uint8_t ADC1H[] = {0x3f};
	//leggo temperatura
	CMain::getSingleton()->getI2c(2)->transmit(0x5f, ADC1L, sizeof(ADC1L));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	T_C1_lsb = data[0];

	CMain::getSingleton()->getI2c(2)->transmit(0x5f, ADC1H, sizeof(ADC1H));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	T_C1_lsb |= data[0] << 8;

	m = ((float)(T_C1-T_C0)) / ((float)(T_C1_lsb-T_C0_lsb));

	uint8_t HumMinAddress[] = {0x30};
	uint8_t HumMaxAddress[] = {0x31};
	//reading low temperature calibration lsb
	CMain::getSingleton()->getI2c(2)->transmit(0x5f, HumMinAddress, sizeof(HumMinAddress));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	H_0 = data[0];

	//reading high temperature calibration lsb
	CMain::getSingleton()->getI2c(2)->transmit(0x5f, HumMaxAddress, sizeof(HumMaxAddress));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	H_1 = data[0];

	H_0 = H_0 >> 1;
	H_1 = H_1 >> 1;

	ADC0L[0] = 0x36;
	ADC0H[0] = 0x37;

	//get the calibration adc min
	CMain::getSingleton()->getI2c(2)->transmit(0x5f, ADC0L, sizeof(ADC0L));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	H_0_lsb = data[0];

	CMain::getSingleton()->getI2c(2)->transmit(0x5f, ADC0H, sizeof(ADC0H));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	H_0_lsb |= data[0] << 8;

	ADC1L[0]=0x3a;
	ADC1H[0]=0x3b;
	//leggo temperatura
	CMain::getSingleton()->getI2c(2)->transmit(0x5f, ADC1L, sizeof(ADC1L));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	H_1_lsb = data[0];

	CMain::getSingleton()->getI2c(2)->transmit(0x5f, ADC1H, sizeof(ADC1H));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	H_1_lsb |= data[0] << 8;

	mh = ((float)(H_1 - H_0)) / ((float)(H_1_lsb - H_0_lsb));
}

float getHumidity() {

	uint8_t humL[] = {0x28};
	uint8_t humH[] = {0x29};
	uint8_t data[2];
	int16_t hum;
	//reading temperature
	CMain::getSingleton()->getI2c(2)->transmit(0x5f, humL, sizeof(humL));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	hum = data[0];

	//high register
	CMain::getSingleton()->getI2c(2)->transmit(0x5f, humH, sizeof(humH));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	hum |= data[0] << 1;

	return H_0 + mh * hum;
}

float getTemperature() {
	uint8_t tempL[]={0x2A};
	uint8_t tempH[]={0x2B};
	uint8_t data[2];
	int16_t temp;
	//reading temperature
	CMain::getSingleton()->getI2c(2)->transmit(0x5f, tempL, sizeof(tempL));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	temp = data[0];

	//high register
	CMain::getSingleton()->getI2c(2)->transmit(0x5f, tempH, sizeof(tempH));
	CMain::getSingleton()->getI2c(2)->receive(0x5f, NULL, 0, data, 1);
	temp |= data[0] << 8;

	return T_C0 + m * temp;
}
