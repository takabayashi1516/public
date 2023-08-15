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
#include "gattclient.h"

// #define TEST_MANUFACTURER_DATA_LEN	17
// #define PREPARE_BUF_MAX_SIZE 1024

esp_ble_scan_params_t CGattClient::ble_scan_params = {
	.scan_type = BLE_SCAN_TYPE_ACTIVE,
	.own_addr_type = BLE_ADDR_TYPE_PUBLIC,
	.scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
	.scan_interval = 0x50,
	.scan_window = 0x30,
	.scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
};

const char *CGattClient::TAG = "GATTC";

CGattClient *CGattClient::m_pobjSingleton = NULL;

CGattClient *CGattClient::getSingleton() {
	return m_pobjSingleton;
}

int CGattClient::getItemsOfGattProfileTable() {
	if (!gl_profile_tab) {
		return ESP_FAIL;
	}
	for (int i = 0; ; i++) {
		if (!gl_profile_tab[i]->gattc_cb) {
			return i;
		}
	}
	assert(false);
	return ESP_FAIL;
}

int CGattClient::addGattProfileTable(gatt_profile_inst *data /* = NULL */)
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
			gatt_profile_inst *d = new gatt_profile_inst(*data);
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

int CGattClient::updateGattIfInProfileTable(uint16_t app_id, esp_gatt_if_t gatt_if)
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

CGattClient::gatt_profile_inst *CGattClient::getGattProfileTable(esp_gatt_if_t gatt_if)
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

CGattClient::gatt_profile_inst *CGattClient::getGattProfileTableFromAppId(int app_id)
{
	ESP_LOGV(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	int items = getItemsOfGattProfileTable();
	if ((app_id >= items) || (app_id < 0)) {
		return NULL;
	}
	return gl_profile_tab[app_id];
}

CGattClient::gatt_session_inst *CGattClient::getSession(uint16_t conn_id) {
	if (!gl_session_tab) {
		return NULL;
	}
	for (uint32_t i = 0; i < this->sessions; i++) {
		if (!gl_session_tab[i]->equals(conn_id)) {
			continue;
		}
		return gl_session_tab[i];
	}
	return NULL;
}

CGattClient::gatt_session_inst *CGattClient::getSession(esp_bd_addr_t *remote_bda) {
	if (!gl_session_tab) {
		return NULL;
	}
	for (uint32_t i = 0; i < this->sessions; i++) {
		if (::memcmp(&gl_session_tab[i]->remote_bda, remote_bda, sizeof(esp_bd_addr_t))) {
			continue;
		}
		return gl_session_tab[i];
	}
	return NULL;
}

int CGattClient::addGattSessionTable(uint16_t conn_id, esp_bd_addr_t *remote_bda) {
	ESP_LOGI(TAG, "l(%4d): %s: conn_id=%u, sessions=%ld", __LINE__, __PRETTY_FUNCTION__,
			conn_id, this->sessions);
	CUtil::dump(remote_bda, sizeof(esp_bd_addr_t));
	gatt_session_inst_p p = getSession(remote_bda);
	if (p) {
		if (conn_id == (uint16_t) ~0u) {
			return ESP_OK;
		}
		if (p->conn_id != (uint16_t) ~0u) {
			return ESP_OK;
		}
		p->conn_id = conn_id;
		return ESP_OK;
	}
	p = new gatt_session_inst(conn_id, remote_bda);
	if (gl_session_tab == NULL) {
		gl_session_tab = new gatt_session_inst_p[1];
		gl_session_tab[0] = p;
		this->sessions = 1;
		ESP_LOGI(TAG, "l(%4d): %s: conn_id=%u, sessions=%ld", __LINE__, __PRETTY_FUNCTION__,
				conn_id, this->sessions);
		return ESP_OK;
	}

	uint32_t items = this->sessions;
	gatt_session_inst_p *tmp = new gatt_session_inst_p[items + 1];
	for (uint32_t i = 0; i < items; i++) {
		tmp[i] = gl_session_tab[i];
	}
	delete [] gl_session_tab;
	gl_session_tab = tmp;
	tmp[items] = p;
	this->sessions++;
	ESP_LOGI(TAG, "l(%4d): %s: conn_id=%u, sessions=%ld", __LINE__, __PRETTY_FUNCTION__,
			conn_id, this->sessions);
	return ESP_OK;
}

bool CGattClient::delGattSessionTable(uint16_t conn_id) {
	ESP_LOGI(TAG, "l(%4d): %s: conn_id=%u, sessions=%lu", __LINE__, __PRETTY_FUNCTION__,
			conn_id, this->sessions);
	bool rc = false;
	uint32_t del_index;
	for (uint32_t i = 0; i < this->sessions; i++) {
		if (!gl_session_tab[i]->equals(conn_id)) {
			continue;
		}
		delete gl_session_tab[i];
		del_index = i;
		this->sessions--;
		rc = true;
		break;
	}
	if (!rc) {
		return rc;
	}
	for (uint32_t i = del_index; i < this->sessions; i++) {
		gl_session_tab[i] = gl_session_tab[i + 1];
	}
	gatt_session_inst_p *tmp = new gatt_session_inst_p[this->sessions];
	::memcpy(tmp, gl_session_tab, sizeof(gatt_session_inst_p) * this->sessions);
	delete [] gl_session_tab;
	gl_session_tab = tmp;
	ESP_LOGI(TAG, "l(%4d): %s: conn_id=%u, sessions=%lu", __LINE__, __PRETTY_FUNCTION__,
			conn_id, this->sessions);
	return rc;
}

void CGattClient::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gattc_cb_param_t *param)
{
	getSingleton()->gattcEventHandler(event, gatt_if, param);
}

void CGattClient::gattcEventHandler(esp_gattc_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gattc_cb_param_t *param)
{
	CExecGattEvents::CRequest *req =
			new CExecGattEvents::CRequest(event, gatt_if, param);
	esp_ble_gattc_cb_param_t *p = &(req->mParam);
	switch (event) {
	case ESP_GATTC_READ_CHAR_EVT:
	case ESP_GATTC_READ_DESCR_EVT: {
		if (param->read.value_len > 0u) {
			uint8_t *buf = new uint8_t[param->read.value_len];
			::memcpy(buf, param->read.value, param->read.value_len);
			p->read.value = buf;
		} else {
			p->read.value = NULL;
		}
		break;
	}
	case ESP_GATTC_NOTIFY_EVT: {
		if (param->notify.value_len > 0u) {
			uint8_t *buf = new uint8_t[param->notify.value_len];
			::memcpy(buf, param->notify.value, param->notify.value_len);
			p->notify.value = buf;
		} else {
			p->notify.value = NULL;
		}
		break;
	}
	default:
		break;
	}
	(void) requestSync(ERequestGattEvents, req, sizeof(*req));
	delete req;
}

void CGattClient::createInstance(const char *deviceName /* = NULL */,
		CGattClientListener *listener /* = NULL */,
		const bool multi_session /* = false */)
{
	new CGattClient(deviceName, listener, multi_session);
}

CGattClient::CGattClient(const char *deviceName, CGattClientListener *listener,
		const bool multi_session)
	: CGap(deviceName, static_cast<uint32_t>(~0u), static_cast<uint32_t>(4096),
			static_cast<uint32_t>(512), static_cast<uint32_t>(5)),
		gl_profile_tab(NULL), gl_session_tab(NULL), sessions(0),
		multi_session(multi_session), m_pobjListener(listener) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	m_pobjExecSetLocalMtu = new CExecSetLocalMtu(this);
	m_pobjExecRegisterApp = new CExecRegisterApp(this);
	m_pobjExecGattEvents = new CExecGattEvents(this);
	m_pobjExecWrite = new CExecWrite(this);
	m_pobjExecPrepareWrite = new CExecPrepareWrite(this);
	m_pobjExecReadChar = new CExecReadChar(this);
	m_pobjExecReadByType = new CExecReadByType(this);
	m_pobjExecReadMultiple = new CExecReadMultiple(this);
	m_pobjExecReadCharDescr = new CExecReadCharDescr(this);

	assert(!getSingleton());
	m_pobjSingleton = this;

	signalPrepare();
	waitStartUp();
}

CGattClient::~CGattClient()
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	delete m_pobjExecSetLocalMtu;
	delete m_pobjExecRegisterApp;
	delete m_pobjExecGattEvents;
	delete m_pobjExecWrite;
	delete m_pobjExecPrepareWrite;
	delete m_pobjExecReadChar;
	delete m_pobjExecReadByType;
	delete m_pobjExecReadMultiple;
	delete m_pobjExecReadCharDescr;

	int items = getItemsOfGattProfileTable();
	if (items <= 0) {
		return;
	}
	for (int i = 0; i < items; i++) {
		delete gl_profile_tab[i];
	}
	delete [] gl_profile_tab;
	gl_profile_tab = NULL;

	for (uint32_t i = 0; i < this->sessions; i++) {
		delete gl_session_tab[i];
	}
	delete [] gl_session_tab;
	gl_session_tab = NULL;
}

int CGattClient::onInitialize()
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);

	CGap::onInitialize();

	(void) registerExec(ERequestSetLocalMtu, m_pobjExecSetLocalMtu);
	(void) registerExec(ERequestRegisterApp, m_pobjExecRegisterApp);
	(void) registerExec(ERequestGattEvents, m_pobjExecGattEvents);
	(void) registerExec(ERequestWrite, m_pobjExecWrite);
	(void) registerExec(ERequestPrepareWrite, m_pobjExecPrepareWrite);
	(void) registerExec(ERequestReadChar, m_pobjExecReadChar);
	(void) registerExec(ERequestReadByType, m_pobjExecReadByType);
	(void) registerExec(ERequestReadMultiple, m_pobjExecReadMultiple);
	(void) registerExec(ERequestReadCharDescr, m_pobjExecReadCharDescr);

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

int CGattClient::onTerminate()
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	return 0;
}

int CGattClient::onTimeout()
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	return 0;
}

int CGattClient::initBle() {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	esp_err_t rc = CGap::initBle();
	ESP_ERROR_CHECK(rc);
	if (rc) {
		ESP_LOGE(TAG, "l(%4d): %s: %s", __LINE__, __PRETTY_FUNCTION__, ::esp_err_to_name(rc));
		return rc;
	}

	rc = ::esp_ble_gattc_register_callback(CGattClient::gattc_event_handler);
	if (rc) {
		ESP_LOGE(TAG, "gattc register error, error code = %x", rc);
		return rc;
	}

	return rc;
}

int CGattClient::CExecSetLocalMtu::doExec(const void *a_lpParam, const int a_cnLength)
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

int CGattClient::CExecRegisterApp::doExec(const void *a_lpParam, const int a_cnLength)
{
	ESP_LOGI(TAG, "l(%4d): %s: len=%u", __LINE__, __PRETTY_FUNCTION__, a_cnLength);
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}

	gatt_profile_inst data;
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	::memcpy(&(data.remote_filter_service_uuid), req->m_remote_filter_service_uuid,
			sizeof(esp_bt_uuid_t));
	::memcpy(&(data.remote_filter_char_uuid), req->m_remote_filter_char_uuid,
			sizeof(esp_bt_uuid_t));
	::memcpy(&(data.notify_descr_uuid), req->m_notify_descr_uuid,
			sizeof(esp_bt_uuid_t));
	
	data.gattc_cb = gattc_event_handler;
	int id = m_pobjOwner->addGattProfileTable(&data);
	id--;

	esp_err_t rc = ::esp_ble_gattc_app_register(id);
	if (rc) {
		ESP_LOGE(TAG, "l(%4d): %s = %x", __LINE__, __PRETTY_FUNCTION__, rc);
		return rc;
	}

	return id;
}

int CGattClient::CExecGattEvents::doExec(const void *a_lpParam, const int a_cnLength)
{
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	ESP_LOGI(TAG, "l(%4d): %s: len=%u", __LINE__, __PRETTY_FUNCTION__, a_cnLength);
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}

	esp_gattc_cb_event_t event = req->mEvent;
	esp_gatt_if_t gatt_if = req->mGattIf;
	esp_ble_gattc_cb_param_t *param = &(req->mParam);

	ESP_LOGI(TAG, "l(%4d): %s: event=%d", __LINE__, __PRETTY_FUNCTION__, event);
	if (event == ESP_GATTC_REG_EVT) {
		if (param->reg.status != ESP_GATT_OK) {
			ESP_LOGW(TAG, "Reg app failed, app_id %04x, status %d",
					param->reg.app_id,
					param->reg.status);
			return ESP_OK;
		}
		ESP_LOGI(TAG, "l(%4d): %s: app_id=%d, gatt_if=%d", __LINE__, __PRETTY_FUNCTION__,
				param->reg.app_id, gatt_if);
		m_pobjOwner->updateGattIfInProfileTable(param->reg.app_id, gatt_if);
	}

	m_pobjOwner->gattcProfileEventHandler(event, gatt_if, param);

	ESP_LOGI(TAG, "l(%4d): %s: event=%d", __LINE__, __PRETTY_FUNCTION__, event);
	return ESP_OK;
}

void CGattClient::setLocalMtu(uint16_t a_hwMtu)
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecSetLocalMtu::CRequest *req =
			new CExecSetLocalMtu::CRequest(a_hwMtu);
	requestAsync(ERequestSetLocalMtu, req, sizeof(*req));
}

int CGattClient::registerApp(esp_bt_uuid_t *remote_filter_service_uuid,
		esp_bt_uuid_t *remote_filter_char_uuid,
		esp_bt_uuid_t *notify_descr_uuid)
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecRegisterApp::CRequest req(remote_filter_service_uuid,
			remote_filter_char_uuid, notify_descr_uuid);
	int id = requestSync(ERequestRegisterApp, &req, sizeof(req));
	ESP_LOGI(TAG, "l(%4d): %s: id=%u", __LINE__, __PRETTY_FUNCTION__, id);
	return id;
}

int CGattClient::registerNotify(uint16_t conn_id) {
	for (int id = 0; id < getItemsOfGattProfileTable(); id++) {
		gatt_profile_inst *inst = getGattProfileTableFromAppId(id);
		gatt_session_inst *sess = getSession(conn_id);
		registerNotify(sess, inst);
	}
	return ESP_OK;
}

int CGattClient::registerNotify(gatt_session_inst *sess, gatt_profile_inst *inst) {

	ESP_LOGI(TAG, "l(%4d): %s: conn_id=%d", __LINE__, __PRETTY_FUNCTION__,
			sess->conn_id);

	if (inst->gatt_if == ESP_GATT_IF_NONE) {
		return ESP_ERR_INVALID_ARG;
	}

/*
esp_gatt_status_t esp_ble_gattc_get_all_char(esp_gatt_if_t gattc_if,
                                             uint16_t conn_id,
                                             uint16_t start_handle,
                                             uint16_t end_handle,
                                             esp_gattc_char_elem_t *result,
                                             uint16_t *count, uint16_t offset);
*/

	uint16_t char_elems_count = 0;
	esp_gattc_char_elem_t *char_elems = NULL;
	esp_gatt_status_t status = ::esp_ble_gattc_get_attr_count(inst->gatt_if,
			sess->conn_id, /*ESP_GATT_DB_ALL*/ESP_GATT_DB_CHARACTERISTIC,
			inst->service_start_handle, inst->service_end_handle,
			INVALID_HANDLE, &char_elems_count);
	if (status != ESP_GATT_OK) {
		ESP_LOGW(TAG, "l(%4d): %s: chars=%d: gatt_if=%d, conn_id=%d, service_start_handle=%d, service_end_handle=%d",
				__LINE__, __PRETTY_FUNCTION__, char_elems_count, inst->gatt_if, sess->conn_id,
				inst->service_start_handle, inst->service_end_handle);
		return status;
	}
	ESP_LOGI(TAG, "l(%4d): %s: chars=%d: gatt_if=%d, conn_id=%d, service_start_handle=%d, service_end_handle=%d",
			__LINE__, __PRETTY_FUNCTION__, char_elems_count, inst->gatt_if, sess->conn_id,
			inst->service_start_handle, inst->service_end_handle);

	if (char_elems_count <= 0) {
		ESP_LOGE(TAG, "no char found");
		return ESP_ERR_NOT_FOUND;
	}

	char_elems = new esp_gattc_char_elem_t[char_elems_count];
	status = ::esp_ble_gattc_get_char_by_uuid(inst->gatt_if,
			sess->conn_id, inst->service_start_handle,
			inst->service_end_handle, inst->remote_filter_char_uuid,
			char_elems, &char_elems_count);
	if (status != ESP_GATT_OK) {
		ESP_LOGE(TAG, "esp_ble_gattc_get_char_by_uuid error");
		return status;
	}
	ESP_LOGI(TAG, "l(%4d): %s: chars=%d", __LINE__, __PRETTY_FUNCTION__,
			char_elems_count);
	for (int i = 0; i < char_elems_count; i++) {
		ESP_LOGI(TAG, "l(%4d): %s: properties[%d]=%08x, handle=%d", __LINE__,
				__PRETTY_FUNCTION__, i, char_elems[i].properties,
				char_elems[i].char_handle);
		if ((char_elems[i].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)) {
			::esp_ble_gattc_register_for_notify(inst->gatt_if,
					sess->remote_bda,
					char_elems[i].char_handle);
		}
	}

	sess->setGattIfProperties(inst->gatt_if, char_elems[0].char_handle,
			char_elems[0].properties);
	delete [] char_elems;

	return ESP_OK;
}

void CGattClient::gattcProfileEventHandler(esp_gattc_cb_event_t event,
		esp_gatt_if_t gatt_if, esp_ble_gattc_cb_param_t *param)
{
	switch (event) {
	case ESP_GATTC_REG_EVT: {
		ESP_LOGI(TAG, "REG_EVT");
		int sc_rc = setScanParam(&ble_scan_params);
		if (sc_rc) {
			ESP_LOGE(TAG, "set scan params error, error code = %x", sc_rc);
		}
		break;
	}
	case ESP_GATTC_CONNECT_EVT: {
		ESP_LOGI(TAG, "ESP_GATTC_CONNECT_EVT conn_id %d, if %d", param->connect.conn_id, gatt_if);

		gatt_profile_inst *inst = getGattProfileTable(gatt_if);
		assert(inst);
		addGattSessionTable(param->connect.conn_id, &param->connect.remote_bda);
		gatt_session_inst *sess = getSession(param->connect.conn_id);
		assert(sess);
		sess->addGattIf(gatt_if);

		if (m_pobjListener) {
			(void) m_pobjListener->onConnect(gatt_if, param->connect.conn_id, param->connect.link_role,
					&param->connect.remote_bda, &param->connect.conn_params);
		}

		break;
	}
	case ESP_GATTC_OPEN_EVT: {
		if (param->open.status != ESP_GATT_OK) {
			ESP_LOGE(TAG, "open failed, status %d", param->open.status);
			break;
		}

		gatt_profile_inst *inst = getGattProfileTable(gatt_if);
		assert(inst);
		ESP_LOGI(TAG, "l(%4d): %s: conn_id: %d", __LINE__, __PRETTY_FUNCTION__,
				param->open.conn_id);
		gatt_session_inst *sess = getSession(param->open.conn_id);
		if (!sess) {
			addGattSessionTable(param->open.conn_id, &param->open.remote_bda);
			sess = getSession(param->open.conn_id);
		}

		if (sess->session_status == ESessionStatusClosed) {
			sess->session_status = ESessionStatusOpened;
			esp_err_t mtu_ret = ::esp_ble_gattc_send_mtu_req(gatt_if, param->open.conn_id);
			if (mtu_ret) {
				ESP_LOGE(TAG, "config MTU error, error code = %x", mtu_ret);
			}
		}

		if (m_pobjListener) {
			(void) m_pobjListener->onOpen(gatt_if, param->open.status,
					param->open.conn_id, param->open.remote_bda, param->open.mtu);
		}

		ESP_LOGI(TAG, "open success");
		break;
	}
	case ESP_GATTC_DIS_SRVC_CMPL_EVT: {
		if (param->dis_srvc_cmpl.status != ESP_GATT_OK) {
			ESP_LOGE(TAG, "discover service failed, status %d", param->dis_srvc_cmpl.status);
			break;
		}
		ESP_LOGI(TAG, "discover service complete conn_id %d", param->dis_srvc_cmpl.conn_id);
		break;
	}
	case ESP_GATTC_CFG_MTU_EVT: {
		if (param->cfg_mtu.status != ESP_GATT_OK) {
			ESP_LOGE(TAG,"config mtu failed, error status = %x", param->cfg_mtu.status);
		}
		ESP_LOGI(TAG, "ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
		gatt_profile_inst *inst = getGattProfileTable(gatt_if);
		assert(inst);
		::esp_ble_gattc_search_service(gatt_if, param->cfg_mtu.conn_id,
				&inst->remote_filter_service_uuid);
		break;
	}
	case ESP_GATTC_SEARCH_RES_EVT: {
		ESP_LOGI(TAG, "SEARCH RES: conn_id = %x is primary service %d", param->search_res.conn_id, param->search_res.is_primary);
		ESP_LOGI(TAG, "start handle %d end handle %d current handle value %d", param->search_res.start_handle, param->search_res.end_handle, param->search_res.srvc_id.inst_id);

		gatt_session_inst *sess = getSession(param->search_res.conn_id);
		assert(sess);

		for (int id = 0; id < getItemsOfGattProfileTable(); id++) {
			gatt_profile_inst *inst = getGattProfileTableFromAppId(id);
			if ((param->search_res.srvc_id.uuid.len == inst->remote_filter_service_uuid.len) &&
					(param->search_res.srvc_id.uuid.uuid.uuid16 == inst->remote_filter_service_uuid.uuid.uuid16)) {
				ESP_LOGI(TAG, "service found");
				inst->service_start_handle = param->search_res.start_handle;
				inst->service_end_handle = param->search_res.end_handle;
				ESP_LOGI(TAG, "UUID16: %x", param->search_res.srvc_id.uuid.uuid.uuid16);
			}
		}
		break;
	}
	case ESP_GATTC_SEARCH_CMPL_EVT: {
		if (param->search_cmpl.status != ESP_GATT_OK) {
			ESP_LOGE(TAG, "search service failed, error status = %x", param->search_cmpl.status);
			if (m_pobjListener) {
				(void) m_pobjListener->onSearchCmplete(gatt_if,
						param->search_cmpl.status, param->search_cmpl.conn_id,
						param->search_cmpl.searched_service_source);
			}
			break;
		}
		if (param->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_REMOTE_DEVICE) {
			ESP_LOGI(TAG, "Get service information from remote device");
		} else if (param->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_NVS_FLASH) {
			ESP_LOGI(TAG, "Get service information from flash");
		} else {
			ESP_LOGI(TAG, "unknown service source");
		}
		ESP_LOGI(TAG, "ESP_GATTC_SEARCH_CMPL_EVT");

		ESP_LOGI(TAG, "l(%4d): %s: gatt_if=%d, conn_id=%d",
				__LINE__, __PRETTY_FUNCTION__, gatt_if,
				param->search_cmpl.conn_id);

		int reg_rc = registerNotify(param->search_cmpl.conn_id);
		ESP_LOGI(TAG, "l(%4d): %s: register=%d", __LINE__, __PRETTY_FUNCTION__, reg_rc);

		if (m_pobjListener) {
			(void) m_pobjListener->onSearchCmplete(gatt_if,
					param->search_cmpl.status, param->search_cmpl.conn_id,
					param->search_cmpl.searched_service_source);
		}

		if (this->multi_session) {
			restartScanningAsync(30);
		}

		break;
	}
	case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
		ESP_LOGI(TAG, "l(%4d): %s: ESP_GATTC_REG_FOR_NOTIFY_EVT: gatt_if=%d, status=%d, handle=%d",
				__LINE__, __PRETTY_FUNCTION__,
				gatt_if, param->reg_for_notify.status, param->reg_for_notify.handle);
		/*
		gatt_profile_inst *inst = getGattProfileTable(gatt_if);
		assert(inst);
		ESP_LOGI(TAG, "ESP_GATTC_REG_FOR_NOTIFY_EVT");
		if (param->reg_for_notify.status != ESP_GATT_OK) {
			ESP_LOGE(TAG, "REG FOR NOTIFY failed: error status = %d", param->reg_for_notify.status);
			break;
		}

		uint16_t count = 0;
		esp_gatt_status_t ret_status = ::esp_ble_gattc_get_attr_count(gatt_if,
				inst->conn_id, ESP_GATT_DB_DESCRIPTOR, inst->service_start_handle,
				inst->service_end_handle, inst->char_handle, &count);
		if (ret_status != ESP_GATT_OK) {
			ESP_LOGE(TAG, "esp_ble_gattc_get_attr_count error");
		}
		if (count > 0) {
			esp_gattc_descr_elem_t *descr_elem_result = new esp_gattc_descr_elem_t[count];
			if (descr_elem_result) {
				ret_status = ::esp_ble_gattc_get_descr_by_char_handle(gatt_if,
						inst->conn_id, param->reg_for_notify.handle,
						inst->notify_descr_uuid, descr_elem_result, &count);
				if (ret_status != ESP_GATT_OK) {
					ESP_LOGE(TAG, "esp_ble_gattc_get_descr_by_char_handle error");
				}
				delete [] descr_elem_result;
			}
		} else {
			ESP_LOGE(TAG, "decsr not found");
		}
		//*/
		break;
	}
	case ESP_GATTC_READ_CHAR_EVT:
	case ESP_GATTC_READ_DESCR_EVT: {
		if (m_pobjListener) {
			m_pobjListener->onRead(gatt_if, param->read.status,
					param->read.conn_id, param->read.handle,
					param->read.value, param->read.value_len);
			delete [] param->read.value;
		}
		break;
	}
	case ESP_GATTC_NOTIFY_EVT:
		if (param->notify.is_notify) {
			ESP_LOGI(TAG, "ESP_GATTC_NOTIFY_EVT, receive notify value:");
		} else {
			ESP_LOGI(TAG, "ESP_GATTC_NOTIFY_EVT, receive indicate value:");
		}
		if (m_pobjListener) {
			m_pobjListener->onNotify(gatt_if, param->notify.conn_id,
					&param->notify.remote_bda, param->notify.handle,
					param->notify.value_len, param->notify.value, param->notify.is_notify);
			delete [] param->notify.value;
		}
		break;
	case ESP_GATTC_WRITE_CHAR_EVT:
	case ESP_GATTC_WRITE_DESCR_EVT: {
		if (param->write.status != ESP_GATT_OK) {
			ESP_LOGE(TAG, "write descr failed, error status = %x", param->write.status);
			break;
		}
		ESP_LOGI(TAG, "write descr success ");
		if (m_pobjListener) {
			m_pobjListener->onWrite(gatt_if, param->write.status,
					param->write.conn_id, param->write.handle, param->write.offset);
		}
		break;
	}
	case ESP_GATTC_EXEC_EVT: {
		if (m_pobjListener) {
			m_pobjListener->onExecWrite(gatt_if, param->exec_cmpl.status,
					param->exec_cmpl.conn_id);
		}
		break;
	}
	case ESP_GATTC_SRVC_CHG_EVT: {
		esp_bd_addr_t bda;
		::memcpy(bda, param->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
		CUtil::dump(bda, sizeof(esp_bd_addr_t));
		ESP_LOGI(TAG, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:");
		gatt_session_inst *sess = getSession(&bda);
		assert(sess);
		int rc = ::esp_ble_gattc_close(gatt_if, sess->conn_id);
		ESP_LOGI(TAG, "l(%4d): %s: esp_ble_gattc_close(gatt_if=%d, conn_id=%d)=%d",
				__LINE__, __PRETTY_FUNCTION__, gatt_if,
				sess->conn_id, rc);
		break;
	}
	case ESP_GATTC_DISCONNECT_EVT: {
		gatt_session_inst *sess = getSession(param->disconnect.conn_id);
		if (sess) {
			sess->session_status = ESessionStatusClosed;
		}
		ESP_LOGI(TAG, "ESP_GATTC_DISCONNECT_EVT, reason = %d", param->disconnect.reason);
		int rc = ESP_OK;
		if (m_pobjListener) {
			rc = m_pobjListener->onDisconnect(gatt_if, param->disconnect.conn_id,
					&param->disconnect.remote_bda, param->disconnect.reason);
		}
		setScanStatus(EScanStatusSetting);
		if (rc == ESP_OK) {
			restartScanningAsync(30);
		}
		delGattSessionTable(param->disconnect.conn_id);
		break;
	}
	default:
		break;
	}
}

int CGattClient::writeChar(esp_gatt_if_t gatt_if,
		uint16_t conn_id, uint16_t handle, uint16_t value_len,
		uint8_t *value, esp_gatt_write_type_t write_type,
		esp_gatt_auth_req_t auth_req) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecWrite::CRequest *req =
			new CExecWrite::CRequest(CExecWrite::EApiChar, gatt_if,
					conn_id, handle, value_len, value, write_type, auth_req);
	requestAsync(ERequestWrite, req, sizeof(*req));
	return 0;
}

int CGattClient::writeCharDescr(esp_gatt_if_t gatt_if, uint16_t conn_id,
		uint16_t handle, uint16_t value_len, uint8_t *value,
		esp_gatt_write_type_t write_type, esp_gatt_auth_req_t auth_req) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecWrite::CRequest *req =
			new CExecWrite::CRequest(CExecWrite::EApiDescriptor, gatt_if,
					conn_id, handle, value_len, value, write_type, auth_req);
	requestAsync(ERequestWrite, req, sizeof(*req));
	return 0;
}

int CGattClient::prepareWrite(esp_gatt_if_t gatt_if, uint16_t conn_id,
		uint16_t handle, uint16_t offset, uint16_t value_len,
		uint8_t *value, esp_gatt_auth_req_t auth_req) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecPrepareWrite::CRequest *req =
			new CExecPrepareWrite::CRequest(CExecPrepareWrite::EApiChar, gatt_if, conn_id,
					handle, offset, value_len, value, auth_req);
	requestAsync(ERequestPrepareWrite, req, sizeof(*req));
	return 0;
}

int CGattClient::prepareWriteCharDescr(esp_gatt_if_t gatt_if,
		uint16_t conn_id, uint16_t handle, uint16_t offset,
		uint16_t value_len, uint8_t *value, esp_gatt_auth_req_t auth_req) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecPrepareWrite::CRequest *req =
			new CExecPrepareWrite::CRequest(CExecPrepareWrite::EApiDescriptor, gatt_if, conn_id,
					handle, offset, value_len, value, auth_req);
	requestAsync(ERequestPrepareWrite, req, sizeof(*req));
	return 0;
}

int CGattClient::readChar(esp_gatt_if_t gatt_if, uint16_t conn_id,
		uint16_t handle, esp_gatt_auth_req_t auth_req) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecReadChar::CRequest *req =
			new CExecReadChar::CRequest(gatt_if, conn_id,
					handle, auth_req);
	requestAsync(ERequestReadChar, req, sizeof(*req));
	return 0;
}

int CGattClient::readByType(esp_gatt_if_t gatt_if, uint16_t conn_id,
		uint16_t start_handle, uint16_t end_handle,
		esp_bt_uuid_t *uuid, esp_gatt_auth_req_t auth_req) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecReadByType::CRequest *req =
			new CExecReadByType::CRequest(gatt_if,
					conn_id, start_handle, end_handle, uuid, auth_req);
	requestAsync(ERequestReadByType, req, sizeof(*req));
	return 0;
}

int CGattClient::readMultiple(esp_gatt_if_t gatt_if, uint16_t conn_id,
		esp_gattc_multi_t *read_multi, esp_gatt_auth_req_t auth_req) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecReadMultiple::CRequest *req =
			new CExecReadMultiple::CRequest(gatt_if, conn_id,
					read_multi, auth_req);
	requestAsync(ERequestReadMultiple, req, sizeof(*req));
	return 0;
}

int CGattClient::readCharDescr(esp_gatt_if_t gatt_if, uint16_t conn_id,
		uint16_t handle, esp_gatt_auth_req_t auth_req) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CExecReadCharDescr::CRequest *req =
			new CExecReadCharDescr::CRequest(gatt_if, conn_id,
					handle, auth_req);
	requestAsync(ERequestReadCharDescr, req, sizeof(*req));
	return 0;
}

int CGattClient::CExecWrite::doExec(const void *a_lpParam, const int a_cnLength) {
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	ESP_LOGI(TAG, "l(%4d): %s: len=%u", __LINE__, __PRETTY_FUNCTION__, a_cnLength);
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}

	EApi api = req->m_api;
	esp_gatt_if_t gatt_if = req->m_gatt_if;
	uint16_t conn_id = req->m_conn_id;
	uint16_t handle = req->m_handle;
	uint16_t value_len = req->m_value_len;
	uint8_t *value = req->m_value;
	esp_gatt_write_type_t write_type = req->m_write_type;
	esp_gatt_auth_req_t auth_req = req->m_auth_req;

	esp_err_t rc = ESP_FAIL;

	switch (api) {
	case EApiChar: {
		rc = ::esp_ble_gattc_write_char(gatt_if, conn_id, handle,
				value_len, value, write_type, auth_req);
		break;
	}
	case EApiDescriptor: {
		rc = ::esp_ble_gattc_write_char_descr(gatt_if, conn_id, handle,
				value_len, value, write_type, auth_req);
		break;
	}
	default:
		assert(false);
		break;
	}

	if (value) {
		delete [] value;
	}
	return rc;
}

int CGattClient::CExecPrepareWrite::doExec(const void *a_lpParam, const int a_cnLength) {
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	ESP_LOGI(TAG, "l(%4d): %s: len=%u", __LINE__, __PRETTY_FUNCTION__, a_cnLength);
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}

	EApi api = req->m_api;
	esp_gatt_if_t gatt_if = req->m_gatt_if;
	uint16_t conn_id = req->m_conn_id;
	uint16_t handle = req->m_handle;
	uint16_t offset = req->m_offset;
	uint16_t value_len = req->m_value_len;
	uint8_t *value = req->m_value;
	esp_gatt_auth_req_t auth_req = req->m_auth_req;

	esp_err_t rc = ESP_FAIL;

	switch (api) {
	case EApiChar: {
		rc = ::esp_ble_gattc_prepare_write(gatt_if, conn_id, handle,
				offset, value_len, value, auth_req);
		break;
	}
	case EApiDescriptor: {
		rc = ::esp_ble_gattc_prepare_write_char_descr(gatt_if, conn_id, handle,
				offset, value_len, value, auth_req);
		break;
	}
	default:
		assert(false);
		break;
	}

	if (value) {
		delete [] value;
	}
	return rc;
}

int CGattClient::CExecReadChar::doExec(const void *a_lpParam, const int a_cnLength) {
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	ESP_LOGI(TAG, "l(%4d): %s: len=%u", __LINE__, __PRETTY_FUNCTION__, a_cnLength);
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}

	esp_gatt_if_t gatt_if = req->m_gatt_if;
	uint16_t conn_id = req->m_conn_id;
	uint16_t handle = req->m_handle;
	esp_gatt_auth_req_t auth_req = req->m_auth_req;

	esp_err_t rc = ::esp_ble_gattc_read_char(gatt_if, conn_id, handle, auth_req);
	// ESP_LOGI(TAG, "l(%4d): %s: err=%d", __LINE__, __PRETTY_FUNCTION__, rc);
	return rc;
}

int CGattClient::CExecReadByType::doExec(const void *a_lpParam, const int a_cnLength) {
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	ESP_LOGI(TAG, "l(%4d): %s: len=%u", __LINE__, __PRETTY_FUNCTION__, a_cnLength);
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}

	esp_gatt_if_t gatt_if = req->m_gatt_if;
	uint16_t conn_id = req->m_conn_id;
	uint16_t start_handle = req->m_start_handle;
	uint16_t end_handle = req->m_end_handle;
	esp_bt_uuid_t *uuid = &req->m_uuid;
	esp_gatt_auth_req_t auth_req = req->m_auth_req;

	return ::esp_ble_gattc_read_by_type(gatt_if, conn_id,
			start_handle, end_handle, uuid, auth_req);
}

int CGattClient::CExecReadMultiple::doExec(const void *a_lpParam, const int a_cnLength) {
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	ESP_LOGI(TAG, "l(%4d): %s: len=%u", __LINE__, __PRETTY_FUNCTION__, a_cnLength);
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}

	esp_gatt_if_t gatt_if = req->m_gatt_if;
	uint16_t conn_id = req->m_conn_id;
	esp_gattc_multi_t *read_multi = &req->m_read_multi;
	esp_gatt_auth_req_t auth_req = req->m_auth_req;

	return ::esp_ble_gattc_read_multiple(gatt_if, conn_id, read_multi, auth_req);
}

int CGattClient::CExecReadCharDescr::doExec(const void *a_lpParam, const int a_cnLength) {
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	ESP_LOGI(TAG, "l(%4d): %s: len=%u", __LINE__, __PRETTY_FUNCTION__, a_cnLength);
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}

	esp_gatt_if_t gatt_if = req->m_gatt_if;
	uint16_t conn_id = req->m_conn_id;
	uint16_t handle = req->m_handle;
	esp_gatt_auth_req_t auth_req = req->m_auth_req;

	return ::esp_ble_gattc_read_char_descr(gatt_if, conn_id, handle, auth_req);
}

bool CGattClient::onDiscoverScan(esp_ble_gap_cb_param_t *param) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	(void) CGap::onDiscoverScan(param);
	if (getSession(&param->scan_rst.bda)) {
		ESP_LOGI(TAG, "l(%4d): %s: already exist bda",
				__LINE__, __PRETTY_FUNCTION__);
		return this->multi_session;
	}
	addGattSessionTable((uint16_t) ~0u, &param->scan_rst.bda);
	int items = getItemsOfGattProfileTable();
	for (int i = 0; i < items; i++) {
		gatt_profile_inst *inst = gl_profile_tab[i];
		int rc = ::esp_ble_gattc_open(inst->gatt_if,
				param->scan_rst.bda, param->scan_rst.ble_addr_type, true);
		ESP_LOGI(TAG, "l(%4d): %s: esp_ble_gattc_open(gatt_if=%d, addr_type=%d)=%d",
				__LINE__, __PRETTY_FUNCTION__, inst->gatt_if,
				param->scan_rst.ble_addr_type, rc);
	}
	return this->multi_session;
}

void CGattClient::onScanStopComplete(esp_ble_gap_cb_param_t *param) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	CGap::onScanStopComplete(param);
}

bool CGattClient::onUndiscoverScan(esp_ble_gap_cb_param_t *param) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	bool retry = CGap::onUndiscoverScan(param);
	if (m_pobjListener) {
		retry = m_pobjListener->onUndiscoverScan();
	}
	return retry;
}

CGattClient::gatt_session_inst::gatt_session_inst(uint16_t conn_id, esp_bd_addr_t *remote_bda):
		session_status(ESessionStatusClosed), gatt_ifs(NULL),
		num_of_gatt_ifs(0) {
	::memcpy(&(this->remote_bda), remote_bda, sizeof(esp_bd_addr_t));
	this->conn_id = conn_id;
}

CGattClient::gatt_session_inst::~gatt_session_inst() {
	if (gatt_ifs) {
		delete [] gatt_ifs;
	}
}

bool CGattClient::gatt_session_inst::equals(uint16_t conn_id) {
	return (this->conn_id == conn_id);
}

void CGattClient::gatt_session_inst::addGattIf(esp_gatt_if_t gatt_if) {
	ESP_LOGI(TAG, "l(%4d): %s: gatt_if=%d, num_of_gatt_ifs=%ld",
			__LINE__, __PRETTY_FUNCTION__, gatt_if, num_of_gatt_ifs);
	if (!gatt_ifs) {
		gatt_ifs = new gatt_if_session_inst_p[1];
		gatt_ifs[0] = new gatt_if_session_inst(gatt_if);
		num_of_gatt_ifs++;
		ESP_LOGI(TAG, "l(%4d): %s: gatt_if=%d, num_of_gatt_ifs=%ld",
				__LINE__, __PRETTY_FUNCTION__, gatt_if, num_of_gatt_ifs);
		return;
	}
	gatt_if_session_inst_p *tmp = new gatt_if_session_inst_p[num_of_gatt_ifs + 1];
	::memcpy(tmp, gatt_ifs, sizeof(gatt_if_session_inst_p) * num_of_gatt_ifs);
	tmp[num_of_gatt_ifs] = new gatt_if_session_inst(gatt_if);
	delete [] gatt_ifs;
	gatt_ifs = tmp;
	num_of_gatt_ifs++;
	ESP_LOGI(TAG, "l(%4d): %s: gatt_if=%d, num_of_gatt_ifs=%ld",
			__LINE__, __PRETTY_FUNCTION__, gatt_if, num_of_gatt_ifs);
	return;
}

CGattClient::gatt_if_session_inst_p CGattClient::gatt_session_inst::getGattIf(esp_gatt_if_t gatt_if) {
	gatt_if_session_inst_p p = NULL;
	if ((!gatt_ifs) || (num_of_gatt_ifs <= 0u)) {
		return p;
	}
	for (uint32_t i = 0; i < num_of_gatt_ifs; i++) {
		if (gatt_ifs[i]->gatt_if != gatt_if) {
			continue;
		}
		p = gatt_ifs[i];
		break;
	}
	return p;
}

bool CGattClient::gatt_session_inst::setGattIfProperties(esp_gatt_if_t gatt_if, uint16_t char_handle,
		esp_gatt_char_prop_t properties) {
	gatt_if_session_inst_p p = getGattIf(gatt_if);
	if (!p) {
		return false;
	}
	p->char_handle = char_handle;
	p->properties = properties;
	return true;
}
