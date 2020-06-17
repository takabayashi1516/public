#ifndef __LIBZIGBEE_UTIL_H
#define __LIBZIGBEE_UTIL_H

#include <stdint.h>
#include <termios.h>

#include <lib-event.h>

#ifdef __cplusplus

/**
 */
class CZigBee : public CHandle {
public:
	/**
	 */
	class CRfEventListener {
	public:
		///
		virtual void onReceive(uint8_t a_byDstAddr64[8], uint16_t a_hwDstAddr16,
				uint8_t *a_pbyData, uint16_t a_hwLength) = 0;
		///
		virtual void onTransmitResult(uint8_t a_byFrameId,
				uint8_t a_byDeliveryStatus, uint8_t a_byDiscoveryStatus) = 0;
		///
		virtual void onAtResponse(uint8_t a_byFrameId, uint8_t a_byStatus,
				uint8_t *a_pbyValue, uint16_t a_hwLength) = 0;
		///
		virtual void onCommand(uint8_t *a_pbyCommand, uint16_t a_hwLength) = 0;
	};

private:
	struct termios		m_objTerminos;
	int					m_nBaudRate;
	bool				m_bRxEscape;
	uint8_t				*m_pbyRxBuffer;
	uint8_t				*m_pbyPoolPointer;
	CRfEventListener	*m_pobjEventListener;

public:
	CZigBee(CIOEventDispatcher& a_objDispatcher,
			CRfEventListener *a_pobjListener,
			const char a_szTty[]);
	virtual ~CZigBee();

	virtual RESULT onError(CIOEventDispatcher& a_objDispatcher, int a_nError);
	virtual RESULT onInput(CIOEventDispatcher& a_objDispatcher);

	///
	int transmit(uint8_t a_byFrameId, uint8_t a_byDstAddr64[8],
			uint16_t a_hwDstAddr16, uint8_t *a_pbyData, uint16_t a_hwLength);
	///
	int atCommand(uint8_t a_byFrameId, uint8_t a_byCommand[], uint16_t a_hwLength);

private:
	///
	CRfEventListener *getListener() const {
		return m_pobjEventListener;
	}
	///
	void initRxPtr();
	///
	void addRxData(uint8_t a_byData) {
		*m_pbyPoolPointer = a_byData;
		m_pbyPoolPointer++;
	}
	///
	bool isRxComplete();
	///
	uint8_t calcSumCheck(void *a_pData, uint16_t a_hwLength);
	///
	uint16_t convateToApi2(uint8_t*& a_pbyApi2, uint8_t *a_pbyApi1, uint16_t a_hwLength);
	///
	bool isEscapeCharacter(uint8_t a_byApi1);
	///
	bool isEscapeCharacter(const uint8_t *a_pbyApi2);
	///
	uint16_t toEscape(uint8_t *a_pbyApi2, uint8_t a_byApi1);
};

#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif /* __LIBZIGBEE_UTIL_H */
