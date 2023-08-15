
#ifndef __GAP_H
#define __GAP_H


#include <string.h>

#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"

#include "framework.h"

#define adv_config_flag 	 (1 << 0)
#define scan_rsp_config_flag (1 << 1)

#ifdef __cplusplus

class CGap;
typedef CGap *pCGap_t;

class CGap : public CHandlerBase {

protected:
	enum EScanStatus {
		EScanStatusInit = 0,
		EScanStatusStop,
		EScanStatusSetting,
		EScanStatusScanning,
	};

public:
	enum ERequest {
		ERequestMinimum = 100,
		ERequestStartScanning = ERequestMinimum,
		ERequestGapEvents,
	};

public:
	class CExecGapEvents : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
				: mEvent(event) {
				::memcpy(&mParam, param, sizeof(esp_ble_gap_cb_param_t));
			}
		public:
			esp_gap_ble_cb_event_t mEvent;
			esp_ble_gap_cb_param_t mParam;
		};

	public:
		///
		CExecGapEvents(CGap *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGap *m_pobjOwner;
	};

	class CExecStartScanning : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(uint32_t duration)
				: duration(duration) {
			}
		public:
			uint32_t duration;
		};

	public:
		///
		CExecStartScanning(CGap *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGap *m_pobjOwner;
	};

public:
	///
	virtual ~CGap();
	///
	virtual int onInitialize();

	void restartScanningAsync(uint32_t duration);

protected:
	///
	CGap(const char *deviceName, uint32_t a_unRecvTimeout, uint32_t a_unStagesOfStack,
			uint32_t a_unQueues, uint32_t a_unPriority);

	EScanStatus getScanStatus() const;
	void setScanStatus(const EScanStatus status);
	int setScanParam(esp_ble_scan_params_t *params);
	int startScanning(uint32_t duration);
	int stopScanning();

public:
	const char *getDeviceName() const;

protected:

	virtual int initBle();
	virtual void gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

	virtual bool onDiscoverScan(esp_ble_gap_cb_param_t *param);
	virtual void onScanStopComplete(esp_ble_gap_cb_param_t *param);
	virtual bool onUndiscoverScan(esp_ble_gap_cb_param_t *param);

	static CLock *getLock();

private:
	int gapEventHandlerInst(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

	static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

	static void addChannel(CGap *pInst);
	static void delChannel(CGap *pInst);
	static CGap *getChannel(int no);
	static int getChannelNo(CGap *pInst);

protected:
	uint8_t adv_config_done;
	const char *m_cszDeviceName;

	static esp_ble_adv_params_t adv_params;

private:
	CExecStartScanning *m_pobjExecStartScanning;
	CExecGapEvents *m_pobjExecGapEvents;
	EScanStatus m_nScanStatus;

	static const char *TAG;
	static CGap **m_pobjChannels;
	static int m_nChannels;
	static CLock m_objLock;

};

#endif /* __cplusplus */

#endif /* __GAP_H */
