/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/****************************************************************************
*
* This demo showcases BLE GATT server. It can send adv data, be connected by client.
* Run the gatt_client demo, the client demo will automatically connect to the gatt_server demo.
* Client demo will enable gatt_server's notify after connection. The two devices will then exchange
* data.
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdkconfig.h"
#include "gattserver.h"

// #define TEST_MANUFACTURER_DATA_LEN	17
// #define PREPARE_BUF_MAX_SIZE 1024

#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {
		0x02, 0x01, 0x06,
		0x02, 0x0a, 0xeb, 0x03, 0x03, 0xab, 0xcd
};
#else

static uint8_t adv_service_uuid128[32] = {
	/* LSB <--------------------------------------------------------------------------------> MSB */
	//first uuid, 16bit, [12],[13] is the value
	0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
	//second uuid, 32bit, [12], [13], [14], [15] is the value
	0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

// The length of adv data must be less than 31 bytes
//static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
//adv data
static esp_ble_adv_data_t adv_data = {
	.set_scan_rsp = false,
	.include_name = true,
	.include_txpower = false,
	.min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
	.max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
	.appearance = 0x00,
	.manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
	.p_manufacturer_data =	NULL, //&test_manufacturer[0],
	.service_data_len = 0,
	.p_service_data = NULL,
	.service_uuid_len = sizeof(adv_service_uuid128),
	.p_service_uuid = adv_service_uuid128,
	.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
	.set_scan_rsp = true,
	.include_name = true,
	.include_txpower = true,
	.min_interval = 0x0006,
	.max_interval = 0x0010,
	.appearance = 0x00,
	.manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
	.p_manufacturer_data =	NULL, //&test_manufacturer[0],
	.service_data_len = 0,
	.p_service_data = NULL,
	.service_uuid_len = sizeof(adv_service_uuid128),
	.p_service_uuid = adv_service_uuid128,
	.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

#endif /* CONFIG_SET_RAW_ADV_DATA */

const char *CGattServer::TAG = "GATTS";

CGattServer *CGattServer::m_pobjSingleton = NULL;

CGattServer *CGattServer::getSingleton() {
	return m_pobjSingleton;
}

int CGattServer::getItemsOfGattProfileTable() {
	if (!gl_profile_tab) {
		return -1;
	}
	for (int i = 0; ; i++) {
		if (!gl_profile_tab[i]->gatts_cb) {
			return i;
		}
	}
	assert(false);
	return -2;
}

int CGattServer::addGattProfileTable(gatt_profile_inst *data /* = NULL */)
{
	ESP_LOGI(TAG, "l(%4d): %s: data=%p", __LINE__, __PRETTY_FUNCTION__, data);
	if (!data) {
		data = new gatt_profile_inst();
		data->gatt_if = ESP_GATT_IF_NONE;
	}
	if (gl_profile_tab == NULL) {
		gl_profile_tab = new gatt_profile_inst_p[1];
		gl_profile_tab[0] = data;
		return 0;
	}
	if (!data) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return -1;
	}
	int items = getItemsOfGattProfileTable();
	gatt_profile_inst **tmp = new gatt_profile_inst_p[items + 2];
	for (int i = 0; i <= items; i++) {
		tmp[i] = gl_profile_tab[i];
		if (i >= items) {
			gatt_profile_inst *d = new gatt_profile_inst();
			::memcpy(d, data, sizeof(gatt_profile_inst));
			d->gatt_if = ESP_GATT_IF_NONE;
			tmp[i] = d;
		}
	}
	delete [] gl_profile_tab;
	gl_profile_tab = tmp;
	gatt_profile_inst *d0 = new gatt_profile_inst();
	d0->gatt_if = ESP_GATT_IF_NONE;
	items++;
	tmp[items] = d0;
	return items;
}

int CGattServer::updateGattIfInProfileTable(uint16_t app_id, esp_gatt_if_t gatt_if)
{
	ESP_LOGI(TAG, "l(%4d): %s: app_id=%d, gatt_if=%d", __LINE__, __PRETTY_FUNCTION__, app_id, gatt_if);
	int items = getItemsOfGattProfileTable();
	if (app_id >= items) {
		ESP_LOGE(TAG, "l(%4d): %s: %d >= %d", __LINE__, __PRETTY_FUNCTION__, app_id, items);
		return ESP_FAIL;
	}
	gatt_profile_inst *inst = gl_profile_tab[app_id];
	inst->gatt_if = gatt_if;
	return ESP_OK;
}

CGattServer::gatt_profile_inst *CGattServer::getGattProfileTable(esp_gatt_if_t gatt_if)
{
	ESP_LOGI(TAG, "l(%4d): %s: gatt_if=%d", __LINE__, __PRETTY_FUNCTION__, gatt_if);
	int items = getItemsOfGattProfileTable();
	for (int i = 0; i < items; i++) {
		gatt_profile_inst *inst = gl_profile_tab[i];
//		ESP_LOGI(TAG, "l(%4d): %s: inst->gatt_if=%d", __LINE__, __PRETTY_FUNCTION__, inst->gatt_if);
		if (inst->gatt_if != gatt_if) {
			continue;
		}
		return inst;
	}
	ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	return NULL;
}

void CGattServer::gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gatts_cb_param_t *param)
{
	getSingleton()->gattsEventHandler(event, gatt_if, param);
}

void CGattServer::gattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gatts_cb_param_t *param)
{
	esp_ble_gatts_cb_param_t prm;
	::memcpy(&prm, param, sizeof(esp_ble_gatts_cb_param_t));
	param = &prm;
	if (event == ESP_GATTS_WRITE_EVT) {
		if (param->write.len > 0u) {
			uint8_t *buf = new uint8_t[param->write.len];
			::memcpy(buf, param->write.value, param->write.len);
			param->write.value = buf;
		} else {
			param->write.value = NULL;
		}
	}
	CExecGattEvents::CRequest *req =
			new CExecGattEvents::CRequest(event, gatt_if, param);
	(void) requestSync(ERequestGattEvents, req, sizeof(CExecGattEvents::CRequest));
	delete req;
}

void CGattServer::createInstance(const char *deviceName /* = NULL */, CGattServerListener *listener /* = NULL */)
{
	new CGattServer(deviceName, listener);
}

CGattServer::CGattServer(const char *deviceName, CGattServerListener *listener)
	: CGap(deviceName, static_cast<uint32_t>(~0u), static_cast<uint32_t>(4096),
			static_cast<uint32_t>(512), static_cast<uint32_t>(5)),
		gl_profile_tab(NULL),
		m_pobjListener(listener),
		m_rawScanResp(NULL) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	m_pobjExecSetLocalMtu = new CExecSetLocalMtu(this);
	m_pobjExecRegisterApp = new CExecRegisterApp(this);
	m_pobjExecGattEvents = new CExecGattEvents(this);
	m_pobjExecGattReaponse = new CExecGattReaponse(this);
	m_pobjExecGattIndicate = new CExecGattIndicate(this);

	assert(!getSingleton());
	m_pobjSingleton = this;

	signalPrepare();
	waitStartUp();
}

CGattServer::~CGattServer()
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	delete m_pobjExecSetLocalMtu;
	delete m_pobjExecRegisterApp;
	delete m_pobjExecGattEvents;

	int items = getItemsOfGattProfileTable();
	if (items <= 0) {
		return;
	}
	for (int i = 0; i < items; i++) {
		delete gl_profile_tab[i];
	}
	delete [] gl_profile_tab;
	gl_profile_tab = NULL;
}

int CGattServer::onInitialize()
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);

	CGap::onInitialize();

	(void) registerExec(ERequestSetLocalMtu, m_pobjExecSetLocalMtu);
	(void) registerExec(ERequestRegisterApp, m_pobjExecRegisterApp);
	(void) registerExec(ERequestGattEvents, m_pobjExecGattEvents);
	(void) registerExec(ERequestGattReaponse, m_pobjExecGattReaponse);
	(void) registerExec(ERequestGattIndicate, m_pobjExecGattIndicate);

	esp_err_t rc = initBle();
	if (rc) {
		return rc;
	}

	addGattProfileTable();

	if (m_pobjListener) {
		rc = m_pobjListener->onInitialize();
		if (rc) {
			return rc;
		}
	}

	rc = CHandlerBase::onInitialize();
	return rc;
}

int CGattServer::onTerminate()
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	return 0;
}

int CGattServer::onTimeout()
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	return 0;
}

int CGattServer::initBle() {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	esp_err_t rc = CGap::initBle();
	ESP_ERROR_CHECK(rc);
	if (rc) {
		ESP_LOGE(TAG, "l(%4d): %s: %s", __LINE__, __PRETTY_FUNCTION__, ::esp_err_to_name(rc));
		return rc;
	}

	rc = ::esp_ble_gatts_register_callback(CGattServer::gatts_event_handler);
	if (rc) {
		ESP_LOGE(TAG, "gatts register error, error code = %x", rc);
		return rc;
	}

	return rc;
}

int CGattServer::CExecSetLocalMtu::doExec(const void *a_lpParam, const int a_cnLength)
{
	ESP_LOGI(TAG, "l(%4d): %s: len=%u", __LINE__, __PRETTY_FUNCTION__, a_cnLength);
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s: %d != %d", __LINE__, __PRETTY_FUNCTION__, a_cnLength, sizeof(CRequest));
		return ESP_FAIL;
	}

	esp_err_t rc = ::esp_ble_gatt_set_local_mtu(req->m_hwMtu);
	ESP_LOGI(TAG, "l(%4d): %s: mtu=%d", __LINE__, __PRETTY_FUNCTION__, req->m_hwMtu);
	if (rc) {
		ESP_LOGE(TAG, "l(%4d): %s = %x", __LINE__, __PRETTY_FUNCTION__, rc);
		return rc;
	}
	return 0;
}

int CGattServer::CExecRegisterApp::doExec(const void *a_lpParam, const int a_cnLength)
{
	ESP_LOGI(TAG, "l(%4d): %s: len=%u", __LINE__, __PRETTY_FUNCTION__, a_cnLength);
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}

	gatt_profile_inst data;
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	::memcpy(&(data.service_id), req->m_service_id, sizeof(esp_gatt_srvc_id_t));
	::memcpy(&(data.char_uuid), req->m_char_uuid, sizeof(esp_bt_uuid_t));
	::memcpy(&(data.attr_value), req->m_attr_value, sizeof(esp_attr_value_t));
	::memcpy(&(data.char_prop), req->m_char_prop, sizeof(esp_gatt_char_prop_t));
	data.num_handle = req->m_num_handle;
	data.gatts_cb = gatts_event_handler;
	int id = m_pobjOwner->addGattProfileTable(&data);

	esp_err_t rc = ::esp_ble_gatts_app_register(id - 1);
	if (rc) {
		ESP_LOGE(TAG, "l(%4d): %s = %x", __LINE__, __PRETTY_FUNCTION__, rc);
		return rc;
	}

	return 0;
}

int CGattServer::CExecGattEvents::doExec(const void *a_lpParam, const int a_cnLength)
{
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	ESP_LOGI(TAG, "l(%4d): %s: len=%u", __LINE__, __PRETTY_FUNCTION__, a_cnLength);
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}

	esp_gatts_cb_event_t event = req->mEvent;
	esp_gatt_if_t gatt_if = req->mGattIf;
	esp_ble_gatts_cb_param_t *param = &(req->mParam);

	if (event == ESP_GATTS_REG_EVT) {
		if (param->reg.status != ESP_GATT_OK) {
			ESP_LOGI(TAG, "Reg app failed, app_id %04x, status %d",
					param->reg.app_id,
					param->reg.status);
			return ESP_OK;
		}
		m_pobjOwner->updateGattIfInProfileTable(param->reg.app_id, gatt_if);
	}

	m_pobjOwner->gattsProfileEventHandler(event, gatt_if, param);
	return ESP_OK;
}

int CGattServer::CExecGattReaponse::doExec(const void *a_lpParam, const int a_cnLength)
{
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	ESP_LOGI(TAG, "l(%4d): %s: len=%u", __LINE__, __PRETTY_FUNCTION__, a_cnLength);
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}
	ESP_LOGI(TAG, "l(%4d): %s: gatt-if=%d, conn-id=%d, trans-id=%ld, status=%d",
			__LINE__, __PRETTY_FUNCTION__, req->mGattIf, req->mConnId,
			req->mTransId, req->mStatus);
	CUtil::dump(&req->mRsp, sizeof(req->mRsp));
	esp_err_t rc = ::esp_ble_gatts_send_response(req->mGattIf, req->mConnId,
			req->mTransId, req->mStatus, &(req->mRsp));
	return rc;
}

int CGattServer::CExecGattIndicate::doExec(const void *a_lpParam, const int a_cnLength)
{
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	ESP_LOGI(TAG, "l(%4d): %s: len=%u", __LINE__, __PRETTY_FUNCTION__, a_cnLength);
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}
	ESP_LOGI(TAG, "l(%4d): %s: gatt_if=%d, conn_id=%d, attr_handle=%d, need_confirm=%d",
			__LINE__, __PRETTY_FUNCTION__, req->m_gatt_if, req->m_conn_id,
			req->m_attr_handle, req->m_need_confirm);
	CUtil::dump(req->m_value, req->m_value_len);
	esp_err_t rc = ::esp_ble_gatts_send_indicate(req->m_gatt_if, req->m_conn_id,
			req->m_attr_handle, req->m_value_len, req->m_value,
			req->m_need_confirm);
	if (req->m_value) {
		delete [] req->m_value;
	}
	return rc;
}

void CGattServer::setLocalMtu(uint16_t a_hwMtu)
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecSetLocalMtu::CRequest *req =
			new CExecSetLocalMtu::CRequest(a_hwMtu);
	requestAsync(ERequestSetLocalMtu, req, sizeof(CExecSetLocalMtu::CRequest));
}

int CGattServer::registerApp(esp_gatt_srvc_id_t *service_id,
		esp_bt_uuid_t *char_uuid, esp_attr_value_t *attr_value,
		esp_gatt_char_prop_t *char_prop, uint16_t num_handle)
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecRegisterApp::CRequest req(service_id, char_uuid,
			attr_value, char_prop, num_handle);
	int id = requestSync(ERequestRegisterApp, &req, sizeof(req));
	ESP_LOGI(TAG, "l(%4d): %s: id=%u", __LINE__, __PRETTY_FUNCTION__, id);
	return id;
}

void CGattServer::sendResponse(esp_gatt_if_t gatt_if, uint16_t conn_id, uint32_t trans_id,
		esp_gatt_status_t status, esp_gatt_rsp_t *rsp)
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecGattReaponse::CRequest *req =
			new CExecGattReaponse::CRequest(gatt_if, conn_id, trans_id,
					status, rsp);
	requestAsync(ERequestGattReaponse, req, sizeof(CExecGattReaponse::CRequest));
}

void CGattServer::sendIndicate(esp_gatt_if_t gatt_if, uint16_t conn_id, uint16_t attr_handle,
		uint16_t value_len, uint8_t *value, bool need_confirm)
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecGattIndicate::CRequest *req =
			new CExecGattIndicate::CRequest(gatt_if, conn_id, attr_handle,
					value_len, value, need_confirm);
	requestAsync(ERequestGattIndicate, req, sizeof(CExecGattIndicate::CRequest));
}

void CGattServer::gattsProfileEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gatts_cb_param_t *param)
{
	switch (event) {
	case ESP_GATTS_REG_EVT: {
		ESP_LOGI(TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);

		esp_err_t set_dev_name_ret = ::esp_ble_gap_set_device_name(getDeviceName());
		if (set_dev_name_ret) {
			ESP_LOGE(TAG, "set device name failed, error code = %x", set_dev_name_ret);
		}
#ifdef CONFIG_SET_RAW_ADV_DATA
		esp_err_t raw_adv_ret = ::esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
		if (raw_adv_ret) {
			ESP_LOGE(TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
		}
		adv_config_done |= adv_config_flag;
		esp_err_t raw_scan_ret = ::esp_ble_gap_config_scan_rsp_data_raw(getRawScanData(), getRawScanDataLength());
		if (raw_scan_ret) {
			ESP_LOGE(TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
		}
		adv_config_done |= scan_rsp_config_flag;
#else
		//config adv data
		esp_err_t rc = ::esp_ble_gap_config_adv_data(&adv_data);
		if (rc) {
			ESP_LOGE(TAG, "config adv data failed, error code = %x", rc);
		}
		adv_config_done |= adv_config_flag;
		//config scan response data
		rc = ::esp_ble_gap_config_adv_data(&scan_rsp_data);
		if (rc) {
			ESP_LOGE(TAG, "config scan response data failed, error code = %x", rc);
		}
		adv_config_done |= scan_rsp_config_flag;

#endif
		gatt_profile_inst *inst = getGattProfileTable(gatt_if);
		::esp_ble_gatts_create_service(gatt_if, &inst->service_id, inst->num_handle);
		break;
	}
	case ESP_GATTS_READ_EVT: {
		ESP_LOGI(TAG, "GATT_READ_EVT, conn_id %d, trans_id %ld, handle %d", param->read.conn_id, param->read.trans_id, param->read.handle);
		if (m_pobjListener) {
			m_pobjListener->onRead(gatt_if, param->read.conn_id, param->read.trans_id,
					param->read.bda, param->read.handle, param->read.offset,
					param->read.is_long, param->read.need_rsp);
			break;
		}
		esp_gatt_rsp_t rsp;
		::memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
		rsp.attr_value.handle = param->read.handle;
		rsp.attr_value.len = 1;
		rsp.attr_value.value[0] = 0xcc;
		::esp_ble_gatts_send_response(gatt_if, param->read.conn_id, param->read.trans_id,
				ESP_GATT_READ_NOT_PERMIT, &rsp);
		break;
	}
	case ESP_GATTS_WRITE_EVT: {
		ESP_LOGI(TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %ld, handle %d",
				param->write.conn_id, param->write.trans_id, param->write.handle);
		if (m_pobjListener) {
			m_pobjListener->onWrite(gatt_if, param->write.conn_id, param->write.trans_id, param->write.bda,
					param->write.handle, param->write.offset, param->write.need_rsp, param->write.is_prep,
					param->write.len, param->write.value);
			if (param->write.value) {
				delete [] param->write.value;
			}
			break;
		}

		if (!param->write.need_rsp) {
			if (param->write.value) {
				delete [] param->write.value;
			}
			break;
		}

		esp_gatt_rsp_t rsp;
		::memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
		rsp.attr_value.handle = param->read.handle;
		rsp.attr_value.len = param->write.len;
		if (param->write.value) {
			int len = param->write.len;
			if (len > ESP_GATT_MAX_ATTR_LEN) {
				len = ESP_GATT_MAX_ATTR_LEN;
			}
			::memcpy(rsp.attr_value.value, param->write.value, len);
			delete [] param->write.value;
		}
		::esp_ble_gatts_send_response(gatt_if, param->write.conn_id, param->write.trans_id,
				ESP_GATT_WRITE_NOT_PERMIT, &rsp);
		break;
	}
	case ESP_GATTS_EXEC_WRITE_EVT: {
		ESP_LOGI(TAG,"ESP_GATTS_EXEC_WRITE_EVT");
		int cbrc = ESP_GATT_OK;
		if (m_pobjListener) {
			cbrc = m_pobjListener->onExecWrite(gatt_if, param->exec_write.conn_id,
					param->exec_write.trans_id, param->exec_write.bda,
					param->exec_write.exec_write_flag);
//			break;
		}
		::esp_ble_gatts_send_response(gatt_if, param->exec_write.conn_id,
				param->exec_write.trans_id, static_cast<esp_gatt_status_t>(cbrc), NULL);
		if (param->exec_write.exec_write_flag != ESP_GATT_PREP_WRITE_EXEC) {
			ESP_LOGI(TAG,"ESP_GATT_PREP_WRITE_CANCEL");
		}
		break;
	}
	case ESP_GATTS_MTU_EVT:
		ESP_LOGI(TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
		break;
	case ESP_GATTS_UNREG_EVT:
		break;
	case ESP_GATTS_CREATE_EVT: {
		ESP_LOGI(TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d", param->create.status, param->create.service_handle);
		gatt_profile_inst *inst = getGattProfileTable(gatt_if);
		inst->service_handle = param->create.service_handle;

		esp_ble_gatts_start_service(inst->service_handle);
		esp_err_t add_char_ret = ::esp_ble_gatts_add_char(inst->service_handle, &inst->char_uuid,
				ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, inst->char_prop, &inst->attr_value, NULL);
		if (add_char_ret) {
			ESP_LOGE(TAG, "add char failed, error code =%x", add_char_ret);
		}
		break;
	}
	case ESP_GATTS_ADD_INCL_SRVC_EVT:
		break;
	case ESP_GATTS_ADD_CHAR_EVT: {
		uint16_t length = 0;
		const uint8_t *prf_char;

		ESP_LOGI(TAG, "ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d",
				param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
		gatt_profile_inst *inst = getGattProfileTable(gatt_if);
		inst->char_handle = param->add_char.attr_handle;
		inst->descr_uuid.len = ESP_UUID_LEN_16;
		inst->descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
		esp_err_t get_attr_ret = ::esp_ble_gatts_get_attr_value(param->add_char.attr_handle, &length, &prf_char);
		if (get_attr_ret == ESP_FAIL) {
			ESP_LOGE(TAG, "ILLEGAL HANDLE");
		}

		ESP_LOGI(TAG, "the gatts demo char length = %x", length);
		for (int i = 0; i < length; i++) {
			ESP_LOGI(TAG, "prf_char[%x] =%x", i, prf_char[i]);
		}
		esp_err_t add_descr_ret = ::esp_ble_gatts_add_char_descr(inst->service_handle, &inst->descr_uuid,
				ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, NULL, NULL);
		if (add_descr_ret) {
			ESP_LOGE(TAG, "add char descr failed, error code =%x", add_descr_ret);
		}
		break;
	}
	case ESP_GATTS_ADD_CHAR_DESCR_EVT: {
		gatt_profile_inst *inst = getGattProfileTable(gatt_if);
		inst->descr_handle = param->add_char_descr.attr_handle;
		ESP_LOGI(TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d",
				 param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
		break;
	}
	case ESP_GATTS_DELETE_EVT:
		break;
	case ESP_GATTS_START_EVT:
		ESP_LOGI(TAG, "SERVICE_START_EVT, status %d, service_handle %d",
				 param->start.status, param->start.service_handle);
		break;
	case ESP_GATTS_STOP_EVT:
		break;
	case ESP_GATTS_CONNECT_EVT: {
		esp_ble_conn_update_params_t conn_params;
		::memset(&conn_params, 0, sizeof(conn_params));
		::memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
		/* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
		conn_params.latency = 0;
		conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
		conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
		conn_params.timeout = 400;	  // timeout = 400*10ms = 4000ms
		ESP_LOGI(TAG, "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
				 param->connect.conn_id,
				 param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
				 param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
		//start sent the update connection parameters to the peer device.
		::esp_ble_gap_update_conn_params(&conn_params);
		int advRetry = 1;
		if (m_pobjListener) {
			int rc = m_pobjListener->onConnect(gatt_if, param->connect.conn_id, param->connect.link_role,
					param->connect.remote_bda, param->connect.conn_params);
			if (rc != ESP_OK) {
				advRetry = 0;
			}
		}
		if (advRetry) {
			::esp_ble_gap_start_advertising(&adv_params);
		}
		break;
	}
	case ESP_GATTS_DISCONNECT_EVT: {
		ESP_LOGI(TAG, "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
		int advRetry = 1;
		if (m_pobjListener) {
			int rc = m_pobjListener->onDisconnect(gatt_if, param->disconnect.conn_id,
					param->disconnect.remote_bda, param->disconnect.reason);
			if (rc != ESP_OK) {
				advRetry = 0;
			}
		}
		if (advRetry) {
			::esp_ble_gap_start_advertising(&adv_params);
		}
		break;
	}
	case ESP_GATTS_CONF_EVT:
		ESP_LOGI(TAG, "ESP_GATTS_CONF_EVT, status %d attr_handle %d", param->conf.status, param->conf.handle);
		if (param->conf.status != ESP_GATT_OK) {
			esp_log_buffer_hex(TAG, param->conf.value, param->conf.len);
		}
		break;
	case ESP_GATTS_OPEN_EVT:
	case ESP_GATTS_CANCEL_OPEN_EVT:
	case ESP_GATTS_CLOSE_EVT:
	case ESP_GATTS_LISTEN_EVT:
	case ESP_GATTS_CONGEST_EVT:
	default:
		break;
	}
}

uint8_t *CGattServer::getRawScanData()
{
	if (m_rawScanResp) {
		delete [] m_rawScanResp;
	}
	int len = ::strlen(getDeviceName());
	m_rawScanResp = new uint8_t[len + 2];
	m_rawScanResp[0] = len + 1;
	m_rawScanResp[1] = 9;
	::memcpy(&m_rawScanResp[2], getDeviceName(), len);
	CUtil::dump(m_rawScanResp, getRawScanDataLength());
	return m_rawScanResp;
}

uint32_t CGattServer::getRawScanDataLength()
{
	return ::strlen(getDeviceName()) + 2;
}
