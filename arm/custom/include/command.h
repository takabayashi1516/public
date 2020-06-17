/**
 *
 */

#ifndef __COMMAND_H
#define __COMMAND_H

#include <stdint.h>

#define BIT(n)				((1u) << (n))

/**
	direction bit (host->device: 0, otherwise: 1)
  */
#define CMD_TYPE_BIT_DIR	(15u)
/**
	response bit (response: 1, otherwise: 0)
  */
#define CMD_TYPE_BIT_RES	(14u)

/**
	command type number, get
  */
#define CMD_TYPE_GET		BIT(0u)
/**
	command type number, set
  */
#define CMD_TYPE_SET		BIT(1u)

#define CMD_VAL_IDX_MAX			(14)
#define CMD_VAL_IDX_ADC_START	(4)
#define CMD_VAL_IDX_ADC_BITS	(6)
#define CMD_VAL_IDX_PWM_START	(CMD_VAL_IDX_ADC_START + CMD_VAL_IDX_ADC_BITS)
#define CMD_VAL_IDX_PWM_BITS	(CMD_VAL_IDX_MAX - CMD_VAL_IDX_PWM_START)

#define CMD_VAL_IDX_RESULT	(0u)
#define CMD_VAL_IDX_POWER	(1u)
#define CMD_VAL_IDX_PERIOD	(2u)
#define CMD_VAL_IDX_SWITCH	(3u)
#define CMD_VAL_IDX_ADC(n)	((n) + CMD_VAL_IDX_ADC_START)
#define CMD_VAL_IDX_PWM(n)	((n) + CMD_VAL_IDX_PWM_START)

#pragma pack(1)
typedef struct command_t {
	uint16_t	pattern;
	uint16_t	type;
	uint32_t	value[CMD_VAL_IDX_MAX];
} command_t;
#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __COMMAND_H */
