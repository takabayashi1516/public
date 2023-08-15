#ifndef __SENSORS_H
#define __SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void initLPS22hh();
float getPressure();
void initHTS221();
float getHumidity();
float getTemperature();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SENSORS_H */
