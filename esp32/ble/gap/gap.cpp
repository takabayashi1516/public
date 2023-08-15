#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_gattc_api.h"

#include "sdkconfig.h"
#include "gap.h"

const char *CGap::TAG = "GAP";

CGap **CGap::m_pobjChannels = NULL;
int CGap::m_nChannels = 0;
CGap::CLock CGap::m_objLock;

esp_ble_adv_params_t CGap::adv_params = {
	.adv_int_min		= 0x20,
	.adv_int_max		= 0x40,
	.adv_type			= ADV_TYPE_IND,
	.own_addr_type		= BLE_ADDR_TYPE_PUBLIC,
	.peer_addr			= {0u},
	.peer_addr_type		= BLE_ADDR_TYPE_PUBLIC,
	.channel_map		= ADV_CHNL_ALL,
	.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

CGap::CLock *CGap::getLock() {
	return const_cast<CLock *>(&m_objLock);
}

const char *CGap::getDeviceName() const {
	return m_cszDeviceName;
}

CGap::EScanStatus CGap::getScanStatus() const {
	return m_nScanStatus;
}

void CGap::setScanStatus(const EScanStatus status) {
	m_nScanStatus = status;
}

bool CGap::onDiscoverScan(esp_ble_gap_cb_param_t *param) {
	stopScanning();
	return false;
}

void CGap::onScanStopComplete(esp_ble_gap_cb_param_t *param) {
	// nothing to do.
	setScanStatus(EScanStatusSetting);
}


bool CGap::onUndiscoverScan(esp_ble_gap_cb_param_t *param) {
	setScanStatus(EScanStatusSetting);
	return true;
}

void CGap::gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	CGap::getChannel(0)->gapEventHandler(event, param);
}

void CGap::addChannel(CGap *pInst) {
	getLock()->lock();
	if (!m_pobjChannels) {
		m_pobjChannels = new pCGap_t[1];
		m_nChannels = 1;
	} else {
		pCGap_t *tmp = new pCGap_t[m_nChannels];
		::memcpy(tmp, m_pobjChannels, sizeof(pCGap_t) * m_nChannels);
		delete [] m_pobjChannels;
		m_pobjChannels = new pCGap_t[m_nChannels + 1];
		::memcpy(m_pobjChannels, tmp, sizeof(pCGap_t) * m_nChannels);
		delete [] tmp;
		m_nChannels++;
	}
	m_pobjChannels[m_nChannels - 1] = pInst;
	getLock()->unlock();
}

void CGap::delChannel(CGap *pInst) {
	getLock()->lock();
	if (m_nChannels > 1) {
		pCGap_t *tmp = new pCGap_t[m_nChannels];
		::memcpy(tmp, m_pobjChannels, sizeof(pCGap_t) * m_nChannels);
		delete [] m_pobjChannels;
		m_pobjChannels = new pCGap_t[m_nChannels - 1];
		int j = 0;
		for (int i = 0; i < m_nChannels; i++) {
			if (j >= (m_nChannels - 1)) {
				break;
			}
			m_pobjChannels[j] = tmp[i];
			if (tmp[i] == pInst) {
				continue;
			}
			j++;
		}
	} else {
		delete [] m_pobjChannels;
		m_pobjChannels = NULL;
	}
	m_nChannels--;
	getLock()->unlock();
}

CGap *CGap::getChannel(int no) {
	if (no < 0) {
		return NULL;
	}
	if (m_nChannels <= no) {
		return NULL;
	}
	return m_pobjChannels[no];
}

int CGap::getChannelNo(CGap *pInst) {
	int i = 0;
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	for (i = 0; i < m_nChannels; i++) {
		if (m_pobjChannels[i] == pInst) {
			break;
		}
	}
	if (i >= m_nChannels) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return -1;
	}
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	return i;
}

int CGap::setScanParam(esp_ble_scan_params_t *params) {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	esp_err_t rc = ESP_OK;
	if (getScanStatus() > EScanStatusStop) {
		return rc;
	}
	rc = ::esp_ble_gap_set_scan_params(params);
	if (rc == ESP_OK) {
		setScanStatus(EScanStatusSetting);
	}
	return rc;
}

CGap::CGap(const char *deviceName,uint32_t a_unRecvTimeout, uint32_t a_unStagesOfStack,
			uint32_t a_unQueues, uint32_t a_unPriority) :
		CHandlerBase(a_unRecvTimeout, a_unStagesOfStack, a_unQueues,
				a_unPriority, NULL, true),
		adv_config_done(0u), m_cszDeviceName(deviceName),
		m_nScanStatus(EScanStatusInit) {
	ESP_LOGI(TAG, "l(%4d): %s: %p", __LINE__, __PRETTY_FUNCTION__, this);

	addChannel(this);
	m_pobjExecStartScanning = new CExecStartScanning(this);
	m_pobjExecGapEvents = new CExecGapEvents(this);
}

CGap::~CGap() {
	delete m_pobjExecStartScanning;
	delete m_pobjExecGapEvents;
	delChannel(this);
}

int CGap::initBle() {
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
	if (getChannelNo(this) != 0) {
		return ESP_OK;
	}

	esp_err_t rc = ::esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
	ESP_ERROR_CHECK(rc);
	if (rc) {
		ESP_LOGE(TAG, "l(%4d): %s: %s", __LINE__, __PRETTY_FUNCTION__, ::esp_err_to_name(rc));
		return rc;
	}

	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	rc = ::esp_bt_controller_init(&bt_cfg);
	if (rc) {
		ESP_LOGE(TAG, "l(%4d): %s: %s", __LINE__, __PRETTY_FUNCTION__, ::esp_err_to_name(rc));
		return rc;
	}

	rc = ::esp_bt_controller_enable(ESP_BT_MODE_BLE);
	if (rc) {
		ESP_LOGE(TAG, "l(%4d): %s: %s", __LINE__, __PRETTY_FUNCTION__, ::esp_err_to_name(rc));
		return rc;
	}
	rc = ::esp_bluedroid_init();
	if (rc) {
		ESP_LOGE(TAG, "l(%4d): %s: %s", __LINE__, __PRETTY_FUNCTION__, ::esp_err_to_name(rc));
		return rc;
	}
	rc = ::esp_bluedroid_enable();
	if (rc) {
		ESP_LOGE(TAG, "l(%4d): %s: %s", __LINE__, __PRETTY_FUNCTION__, ::esp_err_to_name(rc));
		return rc;
	}

	rc = ::esp_ble_gap_register_callback(CGap::gap_event_handler);
	if (rc) {
		ESP_LOGE(TAG, "gap register error, error code = %x", rc);
		return rc;
	}

	return rc;
}

int CGap::onInitialize()
{
	ESP_LOGI(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);

	(void) registerExec(ERequestStartScanning, m_pobjExecStartScanning);
	(void) registerExec(ERequestGapEvents, m_pobjExecGapEvents);

	return ESP_OK;
}

void CGap::gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	CExecGapEvents::CRequest *req =
			new CExecGapEvents::CRequest(event, param);
	(void) requestSync(ERequestGapEvents, req, sizeof(*req));
	delete req;
}

void CGap::restartScanningAsync(uint32_t duration) {
	ESP_LOGI(TAG, "l(%4d): %s: duration=%lu, status=%d",
			__LINE__, __PRETTY_FUNCTION__, duration, getScanStatus());
	CExecStartScanning::CRequest *req =
			new CExecStartScanning::CRequest(duration);
	getChannel(0)->requestAsync(ERequestStartScanning, req, sizeof(*req));
}

int CGap::startScanning(uint32_t duration) {
	ESP_LOGI(TAG, "l(%4d): %s: duration=%lu, status=%d",
			__LINE__, __PRETTY_FUNCTION__, duration, getScanStatus());
	if (getScanStatus() != EScanStatusSetting) {
		return ESP_FAIL;
	}
	setScanStatus(EScanStatusScanning);
	ESP_LOGI(TAG, "l(%4d): %s: duration=%lu, status=%d",
			__LINE__, __PRETTY_FUNCTION__, duration, getScanStatus());
	return ::esp_ble_gap_start_scanning(duration);
}

int CGap::stopScanning() {
	ESP_LOGI(TAG, "l(%4d): %s: status=%d",
			__LINE__, __PRETTY_FUNCTION__, getScanStatus());
	if (getScanStatus() != EScanStatusScanning) {
		return ESP_FAIL;
	}
	setScanStatus(EScanStatusSetting);
	ESP_LOGI(TAG, "l(%4d): %s: status=%d",
			__LINE__, __PRETTY_FUNCTION__, getScanStatus());
	return ::esp_ble_gap_stop_scanning();
}

int CGap::CExecStartScanning::doExec(const void *a_lpParam, const int a_cnLength) {
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}

	uint32_t duration = req->duration;

	int rc = m_pobjOwner->startScanning(duration);
	if (rc) {
		ESP_LOGW(TAG, "l(%4d): %s: startScanning()=%d", __LINE__, __PRETTY_FUNCTION__, rc);
	}
	return ESP_OK;
}

int CGap::CExecGapEvents::doExec(const void *a_lpParam, const int a_cnLength) {
	CRequest *req = reinterpret_cast<CRequest *>(const_cast<void *>(a_lpParam));
	if (a_cnLength != sizeof(CRequest)) {
		ESP_LOGE(TAG, "l(%4d): %s", __LINE__, __PRETTY_FUNCTION__);
		return ESP_FAIL;
	}

	esp_gap_ble_cb_event_t event = req->mEvent;
	esp_ble_gap_cb_param_t *param = &(req->mParam);

	int rc = m_pobjOwner->gapEventHandlerInst(event, param);
	ESP_LOGI(TAG, "l(%4d): %s: event=%u, rc=%d", __LINE__, __PRETTY_FUNCTION__, event, rc);

	return rc;
}

int CGap::gapEventHandlerInst(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {

	ESP_LOGI(TAG, "l(%4d): %s: event=%u", __LINE__, __PRETTY_FUNCTION__, event);

	switch (event) {
#ifdef CONFIG_SET_RAW_ADV_DATA
	case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
		adv_config_done &= (~adv_config_flag);
		if (adv_config_done == 0) {
			ESP_LOGI(TAG, "l(%4d): %s: start_advertising", __LINE__, __PRETTY_FUNCTION__);
			::esp_ble_gap_start_advertising(&adv_params);
		}
		break;
	case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
		adv_config_done &= (~scan_rsp_config_flag);
		if (adv_config_done == 0) {
			ESP_LOGI(TAG, "l(%4d): %s: start_advertising", __LINE__, __PRETTY_FUNCTION__);
			::esp_ble_gap_start_advertising(&adv_params);
		}
		break;
#else
	case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
		adv_config_done &= (~adv_config_flag);
		if (adv_config_done == 0) {
			ESP_LOGI(TAG, "l(%4d): %s: start_advertising", __LINE__, __PRETTY_FUNCTION__);
			::esp_ble_gap_start_advertising(&adv_params);
		}
		break;
	case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
		adv_config_done &= (~scan_rsp_config_flag);
		if (adv_config_done == 0) {
			ESP_LOGI(TAG, "l(%4d): %s: start_advertising", __LINE__, __PRETTY_FUNCTION__);
			::esp_ble_gap_start_advertising(&adv_params);
		}
		break;
#endif
	case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
		//advertising start complete event to indicate advertising start successfully or failed
		if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
			ESP_LOGE(TAG, "Advertising start failed");
		}
		break;

	case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
		if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
			ESP_LOGE(TAG, "adv stop failed, error status = %x", param->adv_stop_cmpl.status);
			break;
		}
		ESP_LOGI(TAG, "stop adv successfully");
		break;

	case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
		 ESP_LOGI(TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
				  param->update_conn_params.status,
				  param->update_conn_params.min_int,
				  param->update_conn_params.max_int,
				  param->update_conn_params.conn_int,
				  param->update_conn_params.latency,
				  param->update_conn_params.timeout);
		break;

	case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
		//the unit of the duration is second
		startScanning(30);
		break;
	}
	case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
		//scan start complete event to indicate scan start successfully or failed
		if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
			ESP_LOGE(TAG, "scan start failed, error status = %x", param->scan_start_cmpl.status);
			break;
		}
		ESP_LOGI(TAG, "scan start success");

		break;
	case ESP_GAP_BLE_SCAN_RESULT_EVT: {
		esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
		ESP_LOGD(TAG, "l(%4d): %s: search_evt=%u", __LINE__, __PRETTY_FUNCTION__,
				scan_result->scan_rst.search_evt);
		switch (scan_result->scan_rst.search_evt) {
		case ESP_GAP_SEARCH_INQ_RES_EVT: {
			uint8_t *adv_name = NULL;
			uint8_t adv_name_len = 0;
			ESP_LOGD(TAG, "searched Adv Data Len %d, Scan Response Len %d",
					scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);
			adv_name = ::esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
								ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
			ESP_LOGD(TAG, "searched Device Name Len %d", adv_name_len);
			esp_log_buffer_char(TAG, adv_name, adv_name_len);

#if CONFIG_EXAMPLE_DUMP_ADV_DATA_AND_SCAN_RESP
			if (scan_result->scan_rst.adv_data_len > 0) {
				ESP_LOGD(TAG, "adv data:");
			}
			if (scan_result->scan_rst.scan_rsp_len > 0) {
				ESP_LOGD(TAG, "scan resp:");
			}
#endif
			if (adv_name != NULL) {
				const char * targetDevName = getDeviceName();
				if (::strlen(targetDevName) == adv_name_len && ::strncmp((char *)adv_name, targetDevName, adv_name_len) == 0) {
					ESP_LOGD(TAG, "searched device %s", targetDevName);
					ESP_LOGD(TAG, "connect to the remote device.");
					if (onDiscoverScan(scan_result)) {
						restartScanningAsync(30);
					}
				}
			}
			break;
		}
		case ESP_GAP_SEARCH_INQ_CMPL_EVT: {
			bool retry = onUndiscoverScan(param);
			if (retry) {
				startScanning(30);
			}
			break;
		}
		default:
			break;
		}
		break;
	}

	case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT: {
		if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
			ESP_LOGE(TAG, "scan stop failed, error status = %x", param->scan_stop_cmpl.status);
			break;
		}
		ESP_LOGI(TAG, "stop scan successfully");
		onScanStopComplete(param);
		break;
	}

	default:
		break;
	}

	return 0;
}
