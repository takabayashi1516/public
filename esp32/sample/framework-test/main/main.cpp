#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "framework.h"

extern "C" void app_main(void);

/**
 *
 */
class CMain : public CHandlerBase {
public:
    class CExecTest : public CHandlerBase::CExecBase {
    public:
        enum EMessage {
            EMessageTest = 100
        };
    public:
        /**
         */
        class CRequest {
        public:
            ///
            CRequest(uint8_t *a_pbyData, uint32_t a_wLength)
                : m_pbyData(NULL), m_wLength(a_wLength) {
                int l = _RNDUP_REMINDER(m_wLength, 4);
                m_pbyData = new uint8_t[l];
                ::memset(m_pbyData, 0, l);
                ::memcpy(m_pbyData, a_pbyData, m_wLength);
            }
            ///
            virtual ~CRequest() {
                delete [] m_pbyData;
            }
        public:
            ///
            uint8_t     *m_pbyData;
            ///
            uint32_t    m_wLength;
        };

    public:
        ///
        CExecTest(CMain *a_pobjOwner) : m_pobjOwner(a_pobjOwner) {}
        ///
        int doExec(const void *a_lpParam, const int a_cnLength);
    public:
        ///
        CMain   *m_pobjOwner;
    };

public:
    ///
    virtual ~CMain();
    ///
    virtual int onInitialize();
    ///
    virtual int onTerminate();
    ///
    virtual int onTimeout();

    void test(unsigned char *a_pbyData, unsigned short a_hwLength);

    ///
    static CMain *getSingleton();

private:
    ///
    CMain();

private:
    static CMain *m_pobjMain;
    CExecTest *m_pobjExecTest;
};

CMain *CMain::m_pobjMain = NULL;

CMain *CMain::getSingleton()
{
    if (!m_pobjMain) {
        m_pobjMain = new CMain();
    }
    return m_pobjMain;
}

CMain::CMain()
    : CHandlerBase(static_cast<uint32_t>(CONFIG_TEST_TIMEOUT),
            static_cast<uint32_t>(CONFIG_TEST_STACK_SIZE),
            static_cast<uint32_t>(CONFIG_TEST_MESSAGE_SIZE),
            static_cast<uint32_t>(CONFIG_TEST_THREAD_PRIORITY), NULL, false) {
    m_pobjExecTest = new CExecTest(this);
    signalPrepare();
}

CMain::~CMain()
{
}

int CMain::onInitialize()
{
    ::printf("l(%4d): %s\n", __LINE__, __PRETTY_FUNCTION__);

    (void) registerExec(CMain::CExecTest::EMessageTest, m_pobjExecTest);

    return CThreadInterfaceBase::onInitialize();
}

int CMain::onTerminate()
{
    return 0;
}

int CMain::onTimeout()
{
    ::printf("l(%4d): %s\n", __LINE__, __PRETTY_FUNCTION__);
    return 0;
}

void CMain::test(unsigned char *a_pbyData, unsigned short a_hwLength) {
    CMain::CExecTest::CRequest *req =
            new CMain::CExecTest::CRequest(a_pbyData, a_hwLength);
    CUtil::dump(req->m_pbyData, req->m_wLength);
    requestAsync(CMain::CExecTest::EMessageTest, &req, sizeof(req));
}

int CMain::CExecTest::doExec(const void *a_lpParam, const int a_cnLength)
{
    CRequest *req = *reinterpret_cast<CRequest **>(const_cast<void *>(a_lpParam));
    ::printf("l(%4d): %s: len=%lu\n", __LINE__, __PRETTY_FUNCTION__, req->m_wLength);
    return 0;
}


void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    CMain::getSingleton();
    CMain::getSingleton()->test((unsigned char *)"test", (unsigned short)5);

    while (1) {
        IDLE(0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
