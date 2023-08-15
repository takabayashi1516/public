
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "gattserver.h"

#define GATTS_TAG "GATTS_SAMPLE"
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40

class CGattServerListenerSample : public CGattServerListener {
public:
	CGattServerListenerSample();

	virtual int onInitialize();
	virtual int onConnect(esp_gatt_if_t gatt_if, uint16_t conn_id, uint8_t link_role,
			esp_bd_addr_t remote_bda, esp_gatt_conn_params_t conn_params);
	virtual int onDisconnect(esp_gatt_if_t gatt_if, uint16_t conn_id, esp_bd_addr_t remote_bda,
			esp_gatt_conn_reason_t reason);
	virtual int onRead(esp_gatt_if_t gatts_if, uint16_t conn_id,
			uint32_t trans_id, esp_bd_addr_t bda, uint16_t handle,
			uint16_t offset, bool is_long, bool need_rsp);
	virtual int onWrite(esp_gatt_if_t gatts_if, uint16_t conn_id,
			uint32_t trans_id, esp_bd_addr_t bda, uint16_t handle,
			uint16_t offset, bool need_rsp, bool is_prep,
			uint16_t len, uint8_t *value);
	virtual int onExecWrite(esp_gatt_if_t gatts_if, uint16_t conn_id,
			uint32_t trans_id, esp_bd_addr_t bda, uint8_t exec_write_flag);

};

CGattServerListenerSample::CGattServerListenerSample()
{
}

int CGattServerListenerSample::onInitialize()
{
	CGattServer::getSingleton()->setLocalMtu(500);
	return 0;
}

int CGattServerListenerSample::onConnect(esp_gatt_if_t gatt_if, uint16_t conn_id,
		uint8_t link_role, esp_bd_addr_t remote_bda, esp_gatt_conn_params_t conn_params)
{
	return 0;
}

int CGattServerListenerSample::onDisconnect(esp_gatt_if_t gatt_if, uint16_t conn_id,
		esp_bd_addr_t remote_bda, esp_gatt_conn_reason_t reason)
{
	return 0;
}

int CGattServerListenerSample::onRead(esp_gatt_if_t gatts_if, uint16_t conn_id,
		uint32_t trans_id, esp_bd_addr_t bda, uint16_t handle, uint16_t offset,
		bool is_long, bool need_rsp)
{
	esp_gatt_rsp_t rsp;
	::memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = handle;
	rsp.attr_value.len = 4;
	rsp.attr_value.value[0] = 0xde;
	rsp.attr_value.value[1] = 0xed;
	rsp.attr_value.value[2] = 0xbe;
	rsp.attr_value.value[3] = 0xef;
	CGattServer::getSingleton()->sendResponse(gatts_if, conn_id, trans_id,
			ESP_GATT_OK, &rsp);
	return 0;
}

int CGattServerListenerSample::onWrite(esp_gatt_if_t gatts_if, uint16_t conn_id,
		uint32_t trans_id, esp_bd_addr_t bda, uint16_t handle, uint16_t offset,
		bool need_rsp, bool is_prep, uint16_t len, uint8_t *value)
{
	ESP_LOGI(GATTS_TAG, "l(%4d): %s: need_rsp=%d, is_prep=%d, len=%d",
			__LINE__, __PRETTY_FUNCTION__, need_rsp, is_prep, len);
	if (need_rsp) {
		esp_gatt_rsp_t rsp;
		::memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
		rsp.attr_value.handle = handle;
		rsp.attr_value.len = 4;
		rsp.attr_value.value[0] = 0x12;
		rsp.attr_value.value[1] = 0x34;
		rsp.attr_value.value[2] = 0x56;
		rsp.attr_value.value[3] = 0x78;
		CGattServer::getSingleton()->sendResponse(gatts_if, conn_id,
				trans_id, ESP_GATT_OK, &rsp);
	}

	if ((CGattServer::getSingleton()
			->getGattProfileTable(gatts_if)->char_prop) & ESP_GATT_CHAR_PROP_BIT_NOTIFY) {
		uint8_t value[32];
		::memset(value, 0xaa, sizeof(value));
		CGattServer::getSingleton()->sendIndicate(gatts_if, conn_id,
				CGattServer::getSingleton()
						->getGattProfileTable(gatts_if)->char_handle,
				sizeof(value), value, false);
	}

	return 0;
}

int CGattServerListenerSample::onExecWrite(esp_gatt_if_t gatts_if,
		uint16_t conn_id, uint32_t trans_id, esp_bd_addr_t bda,
		uint8_t exec_write_flag)
{
	ESP_LOGI(GATTS_TAG, "l(%4d): %s: exec_write_flag=%d",
			__LINE__, __PRETTY_FUNCTION__, exec_write_flag);
	return 0;
}

extern "C" void gattserver(char *deviceName);

void gattserver(char *deviceName) {
	CGattServer::createInstance(deviceName, new CGattServerListenerSample());

//	::vTaskDelay(1000 / portTICK_PERIOD_MS);
	esp_gatt_srvc_id_t service_id;
	esp_bt_uuid_t char_uuid;
	service_id.is_primary = true;
	service_id.id.inst_id = 0x00;
	service_id.id.uuid.len = ESP_UUID_LEN_16;
	service_id.id.uuid.uuid.uuid16 = 0x00EE;
	char_uuid.len = ESP_UUID_LEN_16;
	char_uuid.uuid.uuid16 = 0xEE01;

	uint8_t char1_str[] = {0x11, 0x22, 0x33};
	esp_attr_value_t char1_val =
	{
		.attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
		.attr_len	  = sizeof(char1_str),
		.attr_value   = char1_str,
	};

	esp_gatt_char_prop_t char_prop = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

	int id = CGattServer::getSingleton()->registerApp(&service_id, &char_uuid, &char1_val, &char_prop, 4);
	ESP_LOGI(GATTS_TAG, "l(%4d): %s: id=%d", __LINE__, __PRETTY_FUNCTION__, id);

//	::vTaskDelay(10000 / portTICK_PERIOD_MS);

	service_id.is_primary = true;
	service_id.id.inst_id = 0x00;
	service_id.id.uuid.len = ESP_UUID_LEN_16;
	service_id.id.uuid.uuid.uuid16 = 0x00CC;
	char_uuid.len = ESP_UUID_LEN_16;
	char_uuid.uuid.uuid16 = 0xCC01;

	uint8_t char2_str[] = {0x44, 0x55, 0x66};
	esp_attr_value_t char2_val =
	{
		.attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
		.attr_len	  = sizeof(char2_str),
		.attr_value   = char2_str,
	};

	char_prop = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE;

	id = CGattServer::getSingleton()->registerApp(&service_id, &char_uuid, &char2_val, &char_prop, 4);
	ESP_LOGI(GATTS_TAG, "l(%4d): %s: id=%d", __LINE__, __PRETTY_FUNCTION__, id);

	service_id.is_primary = true;
	service_id.id.inst_id = 0x00;
	service_id.id.uuid.len = ESP_UUID_LEN_16;
	service_id.id.uuid.uuid.uuid16 = 0x00CC;
	char_uuid.len = ESP_UUID_LEN_16;
	char_uuid.uuid.uuid16 = 0xCC02;

	uint8_t char3_str[] = {0x44, 0x55, 0x66};
	esp_attr_value_t char3_val =
	{
		.attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
		.attr_len	  = sizeof(char3_str),
		.attr_value   = char3_str,
	};

	char_prop = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE;

	id = CGattServer::getSingleton()->registerApp(&service_id, &char_uuid, &char3_val, &char_prop, 4);
	ESP_LOGI(GATTS_TAG, "l(%4d): %s: id=%d", __LINE__, __PRETTY_FUNCTION__, id);
}
