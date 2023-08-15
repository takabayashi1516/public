
#ifndef __GATTSERVER_H
#define __GATTSERVER_H


#include <string.h>

#include "esp_system.h"
#include "esp_log.h"

#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"

#include "gap.h"

#ifdef __cplusplus

class CGattServer;
class CGattServerListener;

class CGattServer : public CGap {
public:
	enum ERequest {
		ERequestMinimum = CGap::ERequestGapEvents,
		ERequestSetLocalMtu,
		ERequestRegisterApp,
		ERequestGattEvents,
		ERequestGattReaponse,
		ERequestGattIndicate,
		ERequestSupremum
	};

public:

	class CExecSetLocalMtu : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(uint16_t a_hwMtu)
				: m_hwMtu(a_hwMtu) {
			}
		public:
			uint16_t m_hwMtu;
		};

	public:
		///
		CExecSetLocalMtu(CGattServer *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattServer *m_pobjOwner;
	};

	class CExecRegisterApp : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(esp_gatt_srvc_id_t *service_id, esp_bt_uuid_t *char_uuid,
					esp_attr_value_t *attr_value, esp_gatt_char_prop_t *char_prop,
					uint16_t num_handle) :
							m_service_id(service_id), m_char_uuid(char_uuid),
							m_attr_value(attr_value), m_char_prop(char_prop),
							m_num_handle(num_handle) {
			}
		public:
			esp_gatt_srvc_id_t *m_service_id;
			esp_bt_uuid_t *m_char_uuid;
			esp_attr_value_t *m_attr_value;
			esp_gatt_char_prop_t *m_char_prop;
			uint16_t m_num_handle;
		};

	public:
		///
		CExecRegisterApp(CGattServer *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattServer *m_pobjOwner;
	};

	class CExecGattEvents : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(esp_gatts_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gatts_cb_param_t *param)
				: mEvent(event), mGattIf(gatt_if) {
				::memcpy(&mParam, param, sizeof(esp_ble_gatts_cb_param_t));
			}
		public:
			esp_gatts_cb_event_t mEvent;
			esp_gatt_if_t mGattIf;
			esp_ble_gatts_cb_param_t mParam;
		};

	public:
		///
		CExecGattEvents(CGattServer *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattServer *m_pobjOwner;
	};

	class CExecGattReaponse : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(esp_gatt_if_t gatt_if, uint16_t conn_id, uint32_t trans_id,
					esp_gatt_status_t status, esp_gatt_rsp_t *rsp)
				: mGattIf(gatt_if), mConnId(conn_id), mTransId(trans_id), mStatus(status) {
				::memcpy(&mRsp, rsp, sizeof(esp_gatt_rsp_t));
			}
		public:
			esp_gatt_if_t mGattIf;
			uint16_t mConnId;
			uint32_t mTransId;
			esp_gatt_status_t mStatus;
			esp_gatt_rsp_t mRsp;
		};

	public:
		///
		CExecGattReaponse(CGattServer *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattServer *m_pobjOwner;
	};

	class CExecGattIndicate : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(esp_gatt_if_t gatt_if, uint16_t conn_id, uint16_t attr_handle,
					uint16_t value_len, uint8_t *value, bool need_confirm)
				: m_gatt_if(gatt_if), m_conn_id(conn_id), m_attr_handle(attr_handle),
						m_value_len(value_len), m_value(NULL), m_need_confirm(need_confirm) {
				if ((value_len > 0u) && (value)) {
					m_value = new uint8_t[value_len];
					::memcpy(m_value, value, value_len);
				} else {
					m_value_len = 0u;
				}
			}

		public:
			esp_gatt_if_t m_gatt_if;
			uint16_t m_conn_id;
			uint16_t m_attr_handle;
			uint16_t m_value_len;
			uint8_t *m_value;
			bool m_need_confirm;
		};

	public:
		///
		CExecGattIndicate(CGattServer *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattServer *m_pobjOwner;
	};

	class gatt_profile_inst {
	public:
		gatt_profile_inst() {
			::memset(this, 0, sizeof(gatt_profile_inst));
		}
	public:
		esp_gatts_cb_t gatts_cb;
		uint16_t gatt_if;
		uint16_t service_handle;
		esp_gatt_srvc_id_t service_id;
		uint16_t char_handle;
		esp_bt_uuid_t char_uuid;
		esp_gatt_perm_t perm;
		esp_gatt_char_prop_t property;
		uint16_t descr_handle;
		esp_bt_uuid_t descr_uuid;
		uint16_t num_handle;
		esp_attr_value_t attr_value;
		esp_gatt_char_prop_t char_prop;
	};
	typedef gatt_profile_inst *gatt_profile_inst_p;

public:
	void setLocalMtu(uint16_t a_hwMtu);
	int registerApp(esp_gatt_srvc_id_t *service_id, esp_bt_uuid_t *char_uuid,
			esp_attr_value_t *attr_value, esp_gatt_char_prop_t *char_prop,
			uint16_t num_handle);
	void sendResponse(esp_gatt_if_t gatt_if, uint16_t conn_id, uint32_t trans_id,
			esp_gatt_status_t status, esp_gatt_rsp_t *rsp);
	void sendIndicate(esp_gatt_if_t gatt_if, uint16_t conn_id, uint16_t attr_handle,
			uint16_t value_len, uint8_t *value, bool need_confirm);

	///
	virtual ~CGattServer();
	///
	virtual int onInitialize();
	///
	virtual int onTerminate();
	///
	virtual int onTimeout();

	gatt_profile_inst *getGattProfileTable(esp_gatt_if_t gatt_if);

	///
	static void createInstance(const char *deviceName = NULL, CGattServerListener *listener = NULL);

	static CGattServer *getSingleton();

protected:
	///
	CGattServer(const char *deviceName, CGattServerListener *listener);

	virtual int initBle();

private:
	int getItemsOfGattProfileTable();
	int addGattProfileTable(gatt_profile_inst *data = NULL);
	int updateGattIfInProfileTable(uint16_t app_id, esp_gatt_if_t gatt_if);

	uint8_t *getRawScanData();
	uint32_t getRawScanDataLength();

	void gattsProfileEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gatts_cb_param_t *param);
	void gattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gatts_cb_param_t *param);

	static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gatts_cb_param_t *param);

private:
	gatt_profile_inst **gl_profile_tab;

	CExecSetLocalMtu *m_pobjExecSetLocalMtu;
	CExecRegisterApp *m_pobjExecRegisterApp;
	CExecGattEvents *m_pobjExecGattEvents;
	CExecGattReaponse *m_pobjExecGattReaponse;
	CExecGattIndicate *m_pobjExecGattIndicate;

	CGattServerListener *m_pobjListener;

	uint8_t *m_rawScanResp;

	static CGattServer *m_pobjSingleton;
	static const char *TAG;
};

class CGattServerListener {
public:
	virtual int onInitialize() = 0;
	virtual int onConnect(esp_gatt_if_t gatt_if, uint16_t conn_id, uint8_t link_role,
			esp_bd_addr_t remote_bda, esp_gatt_conn_params_t conn_params) = 0;
	virtual int onDisconnect(esp_gatt_if_t gatt_if, uint16_t conn_id, esp_bd_addr_t remote_bda,
			esp_gatt_conn_reason_t reason) = 0;
	virtual int onRead(esp_gatt_if_t gatt_if, uint16_t conn_id,
			uint32_t trans_id, esp_bd_addr_t bda, uint16_t handle,
			uint16_t offset, bool is_long, bool need_rsp) = 0;
	virtual int onWrite(esp_gatt_if_t gatt_if, uint16_t conn_id,
			uint32_t trans_id, esp_bd_addr_t bda, uint16_t handle,
			uint16_t offset, bool need_rsp, bool is_prep, uint16_t len,
			uint8_t *value) = 0;
	virtual int onExecWrite(esp_gatt_if_t gatt_if, uint16_t conn_id,
			uint32_t trans_id, esp_bd_addr_t bda, uint8_t exec_write_flag) = 0;
};

#endif /* __cplusplus */

#endif /* __GATTSERVER_H */
