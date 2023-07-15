/**
 *
 */

#ifndef __COMMAND_H
#define __COMMAND_H

#include <stdint.h>

#define COMMAND_TYPE_NOTIF	(1u)
#define COMMAND_TYPE_GET	(2u)
#define COMMAND_TYPE_SET	(3u)

#define NOTIF_IDX_POWEROFF	(0u)

#define GET_IDX_RESULT		(0u)
#define GET_IDX_TEMPERETURE	(1u)
#define GET_IDX_PRESSURE	(2u)
#define GET_IDX_HUMIDITY	(3u)
#define GET_IDX_LUMINANCE	(4u)

#define SET_IDX_REQUEST		(0u)
#define SET_IDX_PERIOD		(1u)
#define SET_IDX_PWM1		(2u)
#define SET_IDX_PWM2		(3u)

#define BIT(n)				((1u) << (n))

#pragma pack(1)

typedef struct command_t {
	uint16_t	type;
	uint16_t	pattern;
	uint32_t	value[7];
} command_t;

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __COMMAND_H */
