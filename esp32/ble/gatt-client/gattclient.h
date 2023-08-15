
#ifndef __GATTCLIENT_H
#define __GATTCLIENT_H


#include <string.h>

#include "esp_system.h"
#include "esp_log.h"

#include "esp_gattc_api.h"
#include "esp_gatt_common_api.h"

#include "gap.h"

#define INVALID_HANDLE (0)

#ifdef __cplusplus

class CGattClient;
class CGattClientListener;

class CGattClient : public CGap {
public:
	enum ERequest {
		ERequestMinimum = CGap::ERequestGapEvents,
		ERequestSetLocalMtu,
		ERequestRegisterApp,
		ERequestGattEvents,
		ERequestWrite,
		ERequestPrepareWrite,
		ERequestReadChar,
		ERequestReadByType,
		ERequestReadMultiple,
		ERequestReadCharDescr,
		ERequestSupremum
	};

	enum ESessionStatus {
		ESessionStatusClosed = 0,
		ESessionStatusOpened
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
		CExecSetLocalMtu(CGattClient *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattClient *m_pobjOwner;
	};

	class CExecRegisterApp : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(esp_bt_uuid_t *remote_filter_service_uuid,
					esp_bt_uuid_t *remote_filter_char_uuid,
					esp_bt_uuid_t *notify_descr_uuid) :
				m_remote_filter_service_uuid(remote_filter_service_uuid),
				m_remote_filter_char_uuid(remote_filter_char_uuid),
				m_notify_descr_uuid(notify_descr_uuid) {
			}
		public:
			esp_bt_uuid_t *m_remote_filter_service_uuid;
			esp_bt_uuid_t *m_remote_filter_char_uuid;
			esp_bt_uuid_t *m_notify_descr_uuid;
		};

	public:
		///
		CExecRegisterApp(CGattClient *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattClient *m_pobjOwner;
	};

	class CExecGattEvents : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(esp_gattc_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gattc_cb_param_t *param)
				: mEvent(event), mGattIf(gatt_if) {
				::memcpy(&mParam, param, sizeof(esp_ble_gattc_cb_param_t));
			}
		public:
			esp_gattc_cb_event_t mEvent;
			esp_gatt_if_t mGattIf;
			esp_ble_gattc_cb_param_t mParam;
		};

	public:
		///
		CExecGattEvents(CGattClient *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattClient *m_pobjOwner;
	};

	/*
	esp_err_t esp_ble_gattc_write_char(esp_gatt_if_t gatt_if,
			uint16_t conn_id, uint16_t handle, uint16_t value_len,
			uint8_t *value, esp_gatt_write_type_t write_type,
			esp_gatt_auth_req_t auth_req);
	esp_err_t esp_ble_gattc_write_char_descr(esp_gatt_if_t gatt_if,
			uint16_t conn_id, uint16_t handle, uint16_t value_len,
			uint8_t *value, esp_gatt_write_type_t write_type,
			esp_gatt_auth_req_t auth_req);
	*/
	class CExecWrite : public CHandlerBase::CExecBase {
	public:
		enum EApi {
			EApiChar,
			EApiDescriptor
		};
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(EApi api, esp_gatt_if_t gatt_if, uint16_t conn_id,
					uint16_t handle, uint16_t value_len, uint8_t *value,
					esp_gatt_write_type_t write_type, esp_gatt_auth_req_t auth_req)
				: m_api(api), m_gatt_if(gatt_if), m_conn_id(conn_id), m_handle(handle),
						m_value_len(value_len), m_write_type(write_type), m_auth_req(auth_req) {
				if ((value) && (value_len > 0u)) {
					m_value = new uint8_t[value_len];
					::memcpy(m_value, value, value_len);
				} else {
					m_value_len = 0u;
					m_value = NULL;
				}
			}
		public:
			EApi m_api;
			esp_gatt_if_t m_gatt_if;
			uint16_t m_conn_id;
			uint16_t m_handle;
			uint16_t m_value_len;
			uint8_t *m_value;
			esp_gatt_write_type_t m_write_type;
			esp_gatt_auth_req_t m_auth_req;
		};

	public:
		///
		CExecWrite(CGattClient *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattClient *m_pobjOwner;
	};

	/*
	esp_err_t esp_ble_gattc_prepare_write(esp_gatt_if_t gatt_if,
			uint16_t conn_id, uint16_t handle, uint16_t offset,
			uint16_t value_len, uint8_t *value, esp_gatt_auth_req_t auth_req);
	esp_err_t esp_ble_gattc_prepare_write_char_descr(esp_gatt_if_t gatt_if,
			uint16_t conn_id, uint16_t handle, uint16_t offset,
			uint16_t value_len, uint8_t *value, esp_gatt_auth_req_t auth_req);
	*/
	class CExecPrepareWrite : public CHandlerBase::CExecBase {
	public:
		enum EApi {
			EApiChar,
			EApiDescriptor
		};
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(EApi api, esp_gatt_if_t gatt_if, uint16_t conn_id,
					uint16_t handle, uint16_t offset, uint16_t value_len, uint8_t *value,
					esp_gatt_auth_req_t auth_req)
				: m_api(api), m_gatt_if(gatt_if), m_conn_id(conn_id), m_handle(handle),
						m_offset(offset), m_value_len(value_len), m_auth_req(auth_req) {
				if ((value) && (value_len > 0u)) {
					m_value = new uint8_t[value_len];
					::memcpy(m_value, value, value_len);
				} else {
					m_value_len = 0u;
					m_value = NULL;
				}
			}
		public:
			EApi m_api;
			esp_gatt_if_t m_gatt_if;
			uint16_t m_conn_id;
			uint16_t m_handle;
			uint16_t m_offset;
			uint16_t m_value_len;
			uint8_t *m_value;
			esp_gatt_auth_req_t m_auth_req;
		};

	public:
		///
		CExecPrepareWrite(CGattClient *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattClient *m_pobjOwner;
	};

	/*
	esp_err_t esp_ble_gattc_read_char(esp_gatt_if_t gatt_if,
			uint16_t conn_id,
			uint16_t handle,
			esp_gatt_auth_req_t auth_req);
	*/
	class CExecReadChar : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(esp_gatt_if_t gatt_if, uint16_t conn_id,
					uint16_t handle, esp_gatt_auth_req_t auth_req)
				: m_gatt_if(gatt_if), m_conn_id(conn_id), m_handle(handle), m_auth_req(auth_req) {
			}
		public:
			esp_gatt_if_t m_gatt_if;
			uint16_t m_conn_id;
			uint16_t m_handle;
			esp_gatt_auth_req_t m_auth_req;
		};

	public:
		///
		CExecReadChar(CGattClient *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattClient *m_pobjOwner;
	};

	/*
	esp_err_t esp_ble_gattc_read_by_type(esp_gatt_if_t gatt_if,
			uint16_t conn_id,
			uint16_t start_handle,
			uint16_t end_handle,
			esp_bt_uuid_t *uuid,
			esp_gatt_auth_req_t auth_req);
	*/
	class CExecReadByType : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(esp_gatt_if_t gatt_if,
					uint16_t conn_id, uint16_t start_handle,
					uint16_t end_handle, esp_bt_uuid_t *uuid,
					esp_gatt_auth_req_t auth_req)
				: m_gatt_if(gatt_if), m_conn_id(conn_id),
						m_start_handle(start_handle), m_end_handle(end_handle), m_auth_req(auth_req) {
				if (uuid) {
					::memcpy(&m_uuid, uuid, sizeof(esp_bt_uuid_t));
				} else {
					::memset(&m_uuid, 0, sizeof(esp_bt_uuid_t));
				}
			}
		public:
			esp_gatt_if_t m_gatt_if;
			uint16_t m_conn_id;
			uint16_t m_start_handle;
			uint16_t m_end_handle;
			esp_bt_uuid_t m_uuid;
			esp_gatt_auth_req_t m_auth_req;
		};

	public:
		///
		CExecReadByType(CGattClient *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattClient *m_pobjOwner;
	};

	/*
	esp_err_t esp_ble_gattc_read_multiple(esp_gatt_if_t gatt_if,
			uint16_t conn_id, esp_gattc_multi_t *read_multi,
			esp_gatt_auth_req_t auth_req);
	*/
	class CExecReadMultiple : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(esp_gatt_if_t gatt_if, uint16_t conn_id,
					esp_gattc_multi_t *read_multi,
					esp_gatt_auth_req_t auth_req)
				: m_gatt_if(gatt_if), m_conn_id(conn_id), m_auth_req(auth_req) {
				if (read_multi) {
					::memcpy(&m_read_multi, read_multi, sizeof(esp_gattc_multi_t));
				} else {
					::memset(&m_read_multi, 0, sizeof(esp_gattc_multi_t));
				}
			}
		public:
			esp_gatt_if_t m_gatt_if;
			uint16_t m_conn_id;
			esp_gattc_multi_t m_read_multi;
			esp_gatt_auth_req_t m_auth_req;
		};

	public:
		///
		CExecReadMultiple(CGattClient *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattClient *m_pobjOwner;
	};

	/*
	esp_err_t esp_ble_gattc_read_char_descr(esp_gatt_if_t gatt_if,
			uint16_t conn_id,
			uint16_t handle,
			esp_gatt_auth_req_t auth_req);
	*/
	class CExecReadCharDescr : public CHandlerBase::CExecBase {
	public:
		/**
		 */
		class CRequest {
		public:
			///
			CRequest(esp_gatt_if_t gatt_if, uint16_t conn_id,
					uint16_t handle, esp_gatt_auth_req_t auth_req)
				: m_gatt_if(gatt_if), m_conn_id(conn_id), m_handle(handle), m_auth_req(auth_req) {
			}
		public:
			esp_gatt_if_t m_gatt_if;
			uint16_t m_conn_id;
			uint16_t m_handle;
			esp_gatt_auth_req_t m_auth_req;
		};

	public:
		///
		CExecReadCharDescr(CGattClient *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
		///
		int doExec(const void *a_lpParam, const int a_cnLength);
	public:
		///
		CGattClient *m_pobjOwner;
	};

	class gatt_profile_inst {
	public:
		gatt_profile_inst() {
			::memset(this, 0, sizeof(gatt_profile_inst));
		}
		gatt_profile_inst(gatt_profile_inst& inst) {
			gattc_cb = inst.gattc_cb;
			gatt_if = inst.gatt_if;
			service_start_handle = inst.service_start_handle;
			service_end_handle = inst.service_end_handle;
			::memcpy(&remote_filter_service_uuid, &inst.remote_filter_service_uuid, sizeof(esp_bt_uuid_t));
			::memcpy(&remote_filter_char_uuid, &inst.remote_filter_char_uuid, sizeof(esp_bt_uuid_t));
			::memcpy(&notify_descr_uuid, &inst.notify_descr_uuid, sizeof(esp_bt_uuid_t));
		}
		~gatt_profile_inst() {
		}
	public:
		esp_gattc_cb_t gattc_cb;
		esp_gatt_if_t gatt_if;
		uint16_t service_start_handle;
		uint16_t service_end_handle;
		esp_bt_uuid_t remote_filter_service_uuid;
		esp_bt_uuid_t remote_filter_char_uuid;
		esp_bt_uuid_t notify_descr_uuid;
	};
	typedef gatt_profile_inst *gatt_profile_inst_p;

	class gatt_if_session_inst {
	public:
		gatt_if_session_inst(esp_gatt_if_t gatt_if) : gatt_if(gatt_if) {}
	public:
		esp_gatt_if_t gatt_if;
		uint16_t char_handle;
		esp_gatt_char_prop_t properties;
	};
	typedef gatt_if_session_inst *gatt_if_session_inst_p;

	class gatt_session_inst {
	public:
		gatt_session_inst(uint16_t conn_id, esp_bd_addr_t *remote_bda);
		~gatt_session_inst();
		bool equals(uint16_t conn_id);
		void addGattIf(esp_gatt_if_t gatt_if);
		gatt_if_session_inst_p getGattIf(esp_gatt_if_t gatt_if);
		bool setGattIfProperties(esp_gatt_if_t gatt_if, uint16_t char_handle,
				esp_gatt_char_prop_t properties);
	public:
		ESessionStatus session_status;
		uint16_t conn_id;
		esp_bd_addr_t remote_bda;
		gatt_if_session_inst_p *gatt_ifs;
		uint32_t num_of_gatt_ifs;
	};
	typedef gatt_session_inst *gatt_session_inst_p;

public:
	void setLocalMtu(uint16_t a_hwMtu);
	int registerApp(esp_bt_uuid_t *remote_filter_service_uuid,
			esp_bt_uuid_t *remote_filter_char_uuid,
			esp_bt_uuid_t *notify_descr_uuid);

	int writeChar(esp_gatt_if_t gatt_if,
			uint16_t conn_id, uint16_t handle, uint16_t value_len,
			uint8_t *value, esp_gatt_write_type_t write_type,
			esp_gatt_auth_req_t auth_req);
	int writeCharDescr(esp_gatt_if_t gatt_if, uint16_t conn_id,
			uint16_t handle, uint16_t value_len, uint8_t *value,
			esp_gatt_write_type_t write_type,
			esp_gatt_auth_req_t auth_req);
	int prepareWrite(esp_gatt_if_t gatt_if, uint16_t conn_id,
			uint16_t handle, uint16_t offset, uint16_t value_len,
			uint8_t *value, esp_gatt_auth_req_t auth_req);
	int prepareWriteCharDescr(esp_gatt_if_t gatt_if,
			uint16_t conn_id, uint16_t handle, uint16_t offset,
			uint16_t value_len, uint8_t *value, esp_gatt_auth_req_t auth_req);
	int readChar(esp_gatt_if_t gatt_if, uint16_t conn_id,
			uint16_t handle, esp_gatt_auth_req_t auth_req);
	int readByType(esp_gatt_if_t gatt_if, uint16_t conn_id,
			uint16_t start_handle, uint16_t end_handle,
			esp_bt_uuid_t *uuid, esp_gatt_auth_req_t auth_req);
	int readMultiple(esp_gatt_if_t gatt_if, uint16_t conn_id,
			esp_gattc_multi_t *read_multi, esp_gatt_auth_req_t auth_req);
	int readCharDescr(esp_gatt_if_t gatt_if, uint16_t conn_id,
			uint16_t handle, esp_gatt_auth_req_t auth_req);

	///
	virtual ~CGattClient();
	///
	virtual int onInitialize();
	///
	virtual int onTerminate();
	///
	virtual int onTimeout();

	gatt_profile_inst *getGattProfileTable(esp_gatt_if_t gatt_if);
	gatt_profile_inst *getGattProfileTableFromAppId(int app_id);
	int getItemsOfGattProfileTable();

	gatt_session_inst *getSession(uint16_t conn_id);
	gatt_session_inst *getSession(esp_bd_addr_t *remote_bda);

	///
	static void createInstance(const char *deviceName = NULL,
			CGattClientListener *listener = NULL, const bool multi_session = false);

	static CGattClient *getSingleton();

protected:
	///
	CGattClient(const char *deviceName, CGattClientListener *listener, const bool multi_session);

	virtual int initBle();

	virtual bool onDiscoverScan(esp_ble_gap_cb_param_t *param);
	virtual void onScanStopComplete(esp_ble_gap_cb_param_t *param);
	virtual bool onUndiscoverScan(esp_ble_gap_cb_param_t *param);

private:
	int addGattProfileTable(gatt_profile_inst *data = NULL);
	int updateGattIfInProfileTable(uint16_t app_id, esp_gatt_if_t gatt_if);
	int registerNotify(uint16_t conn_id);
	int registerNotify(gatt_session_inst *sess, gatt_profile_inst *inst);

	int addGattSessionTable(uint16_t conn_id, esp_bd_addr_t *remote_bda);
	bool delGattSessionTable(uint16_t conn_id);

	void gattcProfileEventHandler(esp_gattc_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gattc_cb_param_t *param);
	void gattcEventHandler(esp_gattc_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gattc_cb_param_t *param);

	static void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gattc_cb_param_t *param);

private:
	gatt_profile_inst **gl_profile_tab;
	gatt_session_inst **gl_session_tab;
	uint32_t sessions;
	const bool multi_session;

	CExecSetLocalMtu *m_pobjExecSetLocalMtu;
	CExecRegisterApp *m_pobjExecRegisterApp;
	CExecGattEvents *m_pobjExecGattEvents;
	CExecWrite *m_pobjExecWrite;
	CExecPrepareWrite *m_pobjExecPrepareWrite;
	CExecReadChar *m_pobjExecReadChar;
	CExecReadByType *m_pobjExecReadByType;
	CExecReadMultiple *m_pobjExecReadMultiple;
	CExecReadCharDescr *m_pobjExecReadCharDescr;

	CGattClientListener *m_pobjListener;

	static esp_ble_scan_params_t ble_scan_params;

	static CGattClient *m_pobjSingleton;
	static const char *TAG;
};

class CGattClientListener {
public:
	virtual int onInitialize() = 0;
	virtual int onConnect(esp_gatt_if_t gatt_if, uint16_t conn_id, uint8_t link_role,
			esp_bd_addr_t *remote_bda, esp_gatt_conn_params_t *conn_params) = 0;
	virtual int onOpen(esp_gatt_if_t gatt_if, esp_gatt_status_t status,
			uint16_t conn_id, esp_bd_addr_t remote_bda, uint16_t mtu) = 0;
	virtual int onDisconnect(esp_gatt_if_t gatt_if, uint16_t conn_id, esp_bd_addr_t *remote_bda,
			esp_gatt_conn_reason_t reason) = 0;
	virtual int onNotify(esp_gatt_if_t gatt_if, uint16_t conn_id,
			esp_bd_addr_t *remote_bda, uint16_t handle,
			uint16_t value_len, uint8_t *value, bool is_notify) = 0;
	virtual int onRead(esp_gatt_if_t gatt_if, esp_gatt_status_t status,
			uint16_t conn_id, uint16_t handle, uint8_t *value, uint16_t value_len) = 0;
	virtual int onWrite(esp_gatt_if_t gatt_if, esp_gatt_status_t status,
			uint16_t conn_id, uint16_t handle, uint16_t offset) = 0;
	virtual int onExecWrite(esp_gatt_if_t gatt_if,
			esp_gatt_status_t status, uint16_t conn_id) = 0;
	virtual int onSearchCmplete(esp_gatt_if_t gatt_if,
			esp_gatt_status_t status, uint16_t conn_id,
			esp_service_source_t searched_service_source) = 0;
	virtual bool onUndiscoverScan() = 0;
};

#endif /* __cplusplus */

#endif /* __GATTCLIENT_H */
