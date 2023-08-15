/*
 * enable.h
 *
 *  Created on: 18 gen 2021
 *      Author: UTPM9
 */

#ifndef INC_ENABLE_H_
#define INC_ENABLE_H_
#include "main.h"


#define BLE_OK 0

#define EVENT_STARTUP_SIZE 6
#define ACI_GATT_INIT_COMPLETE_SIZE 7
#define SET_ATTRIBUTES(n) (n)
#define SET_CONTENT_LENGTH(n) (n)

extern uint8_t UUID_SERVICE_1[];
extern uint8_t CUSTOM_SERVICE_HANDLE[2];
extern uint8_t UUID_CHAR_1[];
extern uint8_t CUSTOM_CHAR_HANDLE[2];
extern uint8_t VALUE1[];
extern uint8_t UUID_CHAR_2[];
extern uint8_t MIC_CHAR_HANDLE[2];
extern uint8_t VALUE2[];
extern uint8_t UUID_CHAR_TEMP[];
extern uint8_t TEMP_CHAR_HANDLE[2];
extern uint8_t VALUE_TEMP[];
extern uint8_t UUID_CHAR_HUM[];
extern uint8_t HUM_CHAR_HANDLE[2];
extern uint8_t VALUE_HUM[];
extern uint8_t UUID_CHAR_PRESS[];
extern uint8_t PRESS_CHAR_HANDLE[2];
extern uint8_t VALUE_PRESS[];
extern uint8_t UUID_INERTIAL_SERVICE[];
extern uint8_t INERTIAL_SERVICE_HANDLE[2];
extern uint8_t UUID_CHAR_INERTIAL_NAME[];
extern uint8_t NAME_INERTIAL_HANDLE[2];
extern uint8_t NAME_INERTIAL_VALUE[];
extern uint8_t UUID_CHAR_INERTIAL_ACCX[];
extern uint8_t ACCX_CHAR_HANDLE[2];
extern uint8_t ACCX_INERTIAL_VALUE[];
extern uint8_t UUID_CHAR_INERTIAL_ACCY[];
extern uint8_t ACCY_CHAR_HANDLE[2];
extern uint8_t ACCY_INERTIAL_VALUE[];
extern uint8_t UUID_CHAR_INERTIAL_ACCZ[];
extern uint8_t ACCZ_CHAR_HANDLE[2];
extern uint8_t ACCZ_INERTIAL_VALUE[];
extern uint8_t X_VALUE[];
extern uint8_t Y_VALUE[];
extern uint8_t Z_VALUE[];
extern uint8_t MIC_VALUE[];
extern uint8_t UUID_MAGNETIC_SERVICE[];
extern uint8_t MAGNETIC_SERVICE_HANDLE[2];
extern uint8_t UUID_CHAR_MAGNETIC_NAME[];
extern uint8_t NAME_MAGNETIC_HANDLE[2];
extern uint8_t NAME_MAGNETIC_VALUE[];
extern uint8_t UUID_CHAR_MAGNETIC_MAGX[];
extern uint8_t MAGX_CHAR_HANDLE[2];
extern uint8_t UUID_CHAR_MAGNETIC_MAGY[];
extern uint8_t MAGY_CHAR_HANDLE[2];
extern uint8_t UUID_CHAR_MAGNETIC_MAGZ[];
extern uint8_t MAGZ_CHAR_HANDLE[2];
extern uint8_t UUID_GYROSCOPE_SERVICE[];
extern uint8_t GYROSCOPE_SERVICE_HANDLE[2];
extern uint8_t UUID_CHAR_GYROSCOPE_NAME[];
extern uint8_t NAME_GYROSCOPE_HANDLE[2];
extern uint8_t NAME_GYROSCOPE_VALUE[];
extern uint8_t UUID_CHAR_GYROSCOPE_GYROX[];
extern uint8_t GYROX_CHAR_HANDLE[2];
extern uint8_t UUID_CHAR_GYROSCOPE_GYROY[];
extern uint8_t GYROY_CHAR_HANDLE[2];
extern uint8_t UUID_CHAR_GYROSCOPE_GYROZ[];
extern uint8_t GYROZ_CHAR_HANDLE[2];
extern uint8_t UUID_TOF_SERVICE[];
extern uint8_t TOF_SERVICE_HANDLE[2];
extern uint8_t UUID_CHAR_TOF_NAME[];
extern uint8_t NAME_TOF_HANDLE[2];
extern uint8_t NAME_TOF_VALUE[];
extern uint8_t UUID_CHAR_TOF_VALUE[];
extern uint8_t TOF_CHAR_HANDLE[2];
extern uint8_t TOF_VALUE[];

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//function for starting the BLE protocol
void ble_init(void);

//function that gets a pending event and save the data in the pointer *container
int fetchBleEvent(uint8_t *container, int size);

//check if the event that was fetched is what I expected
int checkEventResp(uint8_t *event, uint8_t *reference, int size);

void sendCommand(uint8_t *command,int size);

void catchBLE();

void setConnectable();

//set data +- 9999 as maximum
void updateSignedMillesimal(uint8_t *service, uint8_t*characteristic,uint8_t *defaultValue,uint8_t offset, int16_t data);

int BLE_command(uint8_t* command, int size, uint8_t* result, int sizeRes, int returnHandles);

void addService(uint8_t* UUID, uint8_t* handle, int attributes);

void addCharacteristic(uint8_t* UUID,uint8_t* handleChar, uint8_t* handleService, uint8_t maxsize, uint8_t proprieties);

void updateCharValue(uint8_t* handleService,uint8_t* handleChar, int offset, int size,uint8_t* data);

//-+999.9 as value
void updateSignedFloat(uint8_t *service, uint8_t*characteristic,uint8_t *defaultValue,uint8_t offset, float data);

void EXTI9_5_IRQHandler(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* INC_ENABLE_H_ */
