
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "gattclient.h"

#define GATTC_TAG "GATTC_SAMPLE"

const bool
#ifdef CONFIG_MULTIPUL_CONNECT
	multipul_connection = true;
#else
	multipul_connection = false;
#endif

class CGattClientListenerSample : public CGattClientListener {
public:
	CGattClientListenerSample();

	virtual int onInitialize();

	virtual int onConnect(esp_gatt_if_t gatt_if, uint16_t conn_id, uint8_t link_role,
			esp_bd_addr_t *remote_bda, esp_gatt_conn_params_t *conn_params);
	virtual int onOpen(esp_gatt_if_t gatt_if, esp_gatt_status_t status,
			uint16_t conn_id, esp_bd_addr_t remote_bda, uint16_t mtu);
	virtual int onSearchCmplete(esp_gatt_if_t gatt_if,
			esp_gatt_status_t status, uint16_t conn_id,
			esp_service_source_t searched_service_source);
	virtual int onDisconnect(esp_gatt_if_t gatt_if, uint16_t conn_id, esp_bd_addr_t *remote_bda,
			esp_gatt_conn_reason_t reason);
	virtual int onNotify(esp_gatt_if_t gatt_if, uint16_t conn_id,
			esp_bd_addr_t *remote_bda, uint16_t handle,
			uint16_t value_len, uint8_t *value, bool is_notify);
	virtual int onRead(esp_gatt_if_t gatt_if, esp_gatt_status_t status,
			uint16_t conn_id, uint16_t handle, uint8_t *value, uint16_t value_len);
	virtual int onWrite(esp_gatt_if_t gatt_if, esp_gatt_status_t status,
			uint16_t conn_id, uint16_t handle, uint16_t offset);
	virtual int onExecWrite(esp_gatt_if_t gatt_if,
			esp_gatt_status_t status, uint16_t conn_id);
	virtual bool onUndiscoverScan();

};

CGattClientListenerSample::CGattClientListenerSample() {
}

int CGattClientListenerSample::onInitialize() {
	ESP_LOGI(GATTC_TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CGattClient::getSingleton()->setLocalMtu(500);
	return 0;
}

int CGattClientListenerSample::onConnect(esp_gatt_if_t gatt_if, uint16_t conn_id,
		uint8_t link_role, esp_bd_addr_t *remote_bda, esp_gatt_conn_params_t *conn_params) {
	ESP_LOGI(GATTC_TAG, "l(%4d): %s: gatt_if=%d, conn_id=%d, link_role=%d",
			__LINE__, __PRETTY_FUNCTION__, gatt_if, conn_id, link_role);
	CUtil::dump(remote_bda, sizeof(esp_bd_addr_t));
	return 0;
}

/*
TODO: esp_ble_gattc_get_include_service, esp_ble_gattc_get_db, ...etc
*/
int CGattClientListenerSample::onOpen(esp_gatt_if_t gatt_if, esp_gatt_status_t status,
		uint16_t conn_id, esp_bd_addr_t remote_bda, uint16_t mtu) {
	ESP_LOGI(GATTC_TAG, "l(%4d): %s: gatt_if=%d, status=%d, conn_id=%d, mtu=%d",
			__LINE__, __PRETTY_FUNCTION__, gatt_if, status, conn_id, mtu);
	CUtil::dump(remote_bda, sizeof(esp_bd_addr_t));

	CGattClient *gattc = CGattClient::getSingleton();
	CGattClient::gatt_session_inst *sess = gattc->getSession(conn_id);
	CGattClient::gatt_if_session_inst_p gatt_if_sess = sess->getGattIf(gatt_if);

	ESP_LOGI(GATTC_TAG, "l(%4d): %s: gatt_if=%d, properties=%x",
			__LINE__, __PRETTY_FUNCTION__, gatt_if, gatt_if_sess->properties);

	return 0;
}

int CGattClientListenerSample::onSearchCmplete(esp_gatt_if_t gatt_if,
		esp_gatt_status_t status, uint16_t conn_id,
		esp_service_source_t searched_service_source) {

	ESP_LOGI(GATTC_TAG, "l(%4d): %s: gatt_if=%d, conn_id=%d, status=%d",
			__LINE__, __PRETTY_FUNCTION__, gatt_if, conn_id, status);

	if (status != ESP_GATT_OK) {
		return 0;
	}
///*
	CGattClient *gattc = CGattClient::getSingleton();
	for (int i = 0; i < gattc->getItemsOfGattProfileTable(); i++) {
		CGattClient::gatt_profile_inst *inst = gattc->getGattProfileTableFromAppId(i);
		CGattClient::gatt_session_inst *sess = gattc->getSession(conn_id);
		CGattClient::gatt_if_session_inst_p gatt_if_sess = sess->getGattIf(inst->gatt_if);
		ESP_LOGI(GATTC_TAG, "l(%4d): %s: properties[%d]=%x",
				__LINE__, __PRETTY_FUNCTION__, i, gatt_if_sess->properties);
		// ESP_GATT_CHAR_PROP_BIT_READ
		// ESP_GATT_CHAR_PROP_BIT_WRITE_NR
		// ESP_GATT_CHAR_PROP_BIT_WRITE
		// ESP_GATT_CHAR_PROP_BIT_NOTIFY
		// ESP_GATT_CHAR_PROP_BIT_INDICATE
		///*
		
		if (((gatt_if_sess->properties & ESP_GATT_CHAR_PROP_BIT_WRITE)) ||
				((gatt_if_sess->properties & ESP_GATT_CHAR_PROP_BIT_WRITE_NR)) ||
				((gatt_if_sess->properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY))) {
			uint8_t *value = (uint8_t *) "write data!!";
			CGattClient::getSingleton()->writeChar(gatt_if,
					conn_id, gatt_if_sess->char_handle,
					::strlen((char *) value), value, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
		}
		//*/

		///*
		if ((gatt_if_sess->properties & ESP_GATT_CHAR_PROP_BIT_READ)) {
			CGattClient::getSingleton()->readChar(gatt_if,
					conn_id, gatt_if_sess->char_handle, ESP_GATT_AUTH_REQ_NONE);
		}
		//*/
	}
//*/
	return 0;
}

int CGattClientListenerSample::onDisconnect(esp_gatt_if_t gatt_if, uint16_t conn_id,
		esp_bd_addr_t *remote_bda, esp_gatt_conn_reason_t reason) {
	ESP_LOGI(GATTC_TAG, "l(%4d): %s: gatt_if=%d, conn_id=%d",
			__LINE__, __PRETTY_FUNCTION__, gatt_if, conn_id);
	CUtil::dump(remote_bda, sizeof(esp_bd_addr_t));
	return 0;
}

int CGattClientListenerSample::onNotify(esp_gatt_if_t gatt_if, uint16_t conn_id,
			esp_bd_addr_t *remote_bda, uint16_t handle,
			uint16_t value_len, uint8_t *value, bool is_notify) {
	ESP_LOGI(GATTC_TAG, "l(%4d): %s: gatt_if=%d, conn_id=%d, handle=%d",
			__LINE__, __PRETTY_FUNCTION__, gatt_if, conn_id, handle);
	CUtil::dump(remote_bda, sizeof(esp_bd_addr_t));
	CUtil::dump(value, value_len);
	///*
	uint8_t *wrv = (uint8_t *) "write data!!";
	CGattClient::getSingleton()->writeChar(gatt_if,
			conn_id, handle,
			::strlen((char *) wrv), wrv, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
	//*/

	///*
	CGattClient::getSingleton()->readChar(gatt_if,
			conn_id, handle, ESP_GATT_AUTH_REQ_NONE);
	//*/

	return 0;
}

int CGattClientListenerSample::onRead(esp_gatt_if_t gatt_if, esp_gatt_status_t status,
		uint16_t conn_id, uint16_t handle, uint8_t *value, uint16_t value_len) {
	ESP_LOGI(GATTC_TAG, "l(%4d): %s: gatt_if=%d, status=%d, conn_id=%d, handle=%d",
			__LINE__, __PRETTY_FUNCTION__, gatt_if, status, conn_id, handle);
	CUtil::dump(value, value_len);
	return 0;
}

int CGattClientListenerSample::onWrite(esp_gatt_if_t gatt_if, esp_gatt_status_t status,
		uint16_t conn_id, uint16_t handle, uint16_t offset) {
	ESP_LOGI(GATTC_TAG, "l(%4d): %s: gatt_if=%d, status=%d, conn_id=%d, handle=%d",
			__LINE__, __PRETTY_FUNCTION__, gatt_if, status, conn_id, handle);
	return 0;
}

int CGattClientListenerSample::onExecWrite(esp_gatt_if_t gatt_if,
		esp_gatt_status_t status, uint16_t conn_id)
{
	ESP_LOGI(GATTC_TAG, "l(%4d): %s: gatt_if=%d, status=%d, conn_id=%d",
			__LINE__, __PRETTY_FUNCTION__, gatt_if, status, conn_id);
	return 0;
}

bool CGattClientListenerSample::onUndiscoverScan() {
	ESP_LOGI(GATTC_TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	return true;
}

extern "C" void gattclient(char *deviceName);

void gattclient(char *deviceName) {
	CGattClient::createInstance(deviceName, new CGattClientListenerSample(), multipul_connection);

	int id;

//	::vTaskDelay(10000 / portTICK_PERIOD_MS);

///*
	esp_bt_uuid_t remote_filter_service_uuid1 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xfff0,},
	};

	esp_bt_uuid_t remote_filter_char_uuid1 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xfff1,},
	};

	esp_bt_uuid_t notify_descr_uuid1 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xfff1,},
//		.uuid = {.uuid16 = ESP_GATT_UUID_CHAR_DESCRIPTION,},
//		.uuid = {.uuid16 = ESP_GATT_UUID_CHAR_PRESENT_FORMAT,},
	};

	id = CGattClient::getSingleton()->registerApp(
			&remote_filter_service_uuid1, &remote_filter_char_uuid1, &notify_descr_uuid1);

	ESP_LOGI(GATTC_TAG, "l(%4d): %s: id=%d", __LINE__, __PRETTY_FUNCTION__, id);
//*/

///*
	esp_bt_uuid_t remote_filter_service_uuid2 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xfff0,},
	};

	esp_bt_uuid_t remote_filter_char_uuid2 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xfff2,},
	};

	esp_bt_uuid_t notify_descr_uuid2 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xfff2,},
//		.uuid = {.uuid16 = ESP_GATT_UUID_CHAR_DESCRIPTION,},
//		.uuid = {.uuid16 = ESP_GATT_UUID_CHAR_PRESENT_FORMAT,},
	};

	id = CGattClient::getSingleton()->registerApp(
			&remote_filter_service_uuid2, &remote_filter_char_uuid2, &notify_descr_uuid2);

	ESP_LOGI(GATTC_TAG, "l(%4d): %s: id=%d", __LINE__, __PRETTY_FUNCTION__, id);
//*/

///*
	esp_bt_uuid_t remote_filter_service_uuid3 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xfff0,},
	};

	esp_bt_uuid_t remote_filter_char_uuid3 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xfff3,},
	};

	esp_bt_uuid_t notify_descr_uuid3 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xfff3,},
//		.uuid = {.uuid16 = ESP_GATT_UUID_CHAR_DESCRIPTION,},
//		.uuid = {.uuid16 = ESP_GATT_UUID_CHAR_PRESENT_FORMAT,},
	};

	id = CGattClient::getSingleton()->registerApp(
			&remote_filter_service_uuid3, &remote_filter_char_uuid3, &notify_descr_uuid3);

	ESP_LOGI(GATTC_TAG, "l(%4d): %s: id=%d", __LINE__, __PRETTY_FUNCTION__, id);
//*/

///*
	esp_bt_uuid_t remote_filter_service_uuid4 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xfff0,},
	};

	esp_bt_uuid_t remote_filter_char_uuid4 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xfff4,},
	};

	esp_bt_uuid_t notify_descr_uuid4 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xfff4,},
//		.uuid = {.uuid16 = ESP_GATT_UUID_CHAR_DESCRIPTION,},
//		.uuid = {.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,},
//		.uuid = {.uuid16 = ESP_GATT_UUID_CHAR_PRESENT_FORMAT,},
	};

	id = CGattClient::getSingleton()->registerApp(
			&remote_filter_service_uuid4, &remote_filter_char_uuid4, &notify_descr_uuid4);

	ESP_LOGI(GATTC_TAG, "l(%4d): %s: id=%d", __LINE__, __PRETTY_FUNCTION__, id);
//*/

/*
	esp_bt_uuid_t remote_filter_service_uuid9 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xffe0,},
	};

	esp_bt_uuid_t remote_filter_char_uuid9 = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = 0xffe1,},
	};

	esp_bt_uuid_t notify_descr_uuid9 = {
		.len = ESP_UUID_LEN_16,
//		.uuid = {.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,},
		.uuid = {.uuid16 = ESP_GATT_UUID_CHAR_PRESENT_FORMAT,},
	};

	id = CGattClient::getSingleton()->registerApp(
			&remote_filter_service_uuid9, &remote_filter_char_uuid9, &notify_descr_uuid9);

	ESP_LOGI(GATTC_TAG, "l(%4d): %s: id=%d", __LINE__, __PRETTY_FUNCTION__, id);
//*/

}
