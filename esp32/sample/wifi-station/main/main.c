#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "nvs_flash.h"

extern void *wifi_setup_thread_routine(void *p_param);
extern void *wifi_client_thread_routine(void *p_param);

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    pthread_t pth;
    pthread_create(&pth, NULL, wifi_setup_thread_routine, NULL);
    pthread_join(pth, NULL);
    printf("thread terminate !\n");

    memset(&pth, 0, sizeof(pth));
    pthread_create(&pth, NULL, wifi_client_thread_routine, NULL);
    pthread_join(pth, NULL);
    printf("thread terminate !\n");
}
