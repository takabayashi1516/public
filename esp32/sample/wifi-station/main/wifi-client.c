#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
//#include "lwip/err.h"
//#include "lwip/sys.h"

#define TAG ("WIFI-CLIENT")

esp_err_t wifi_unit_client_establish(int *sock, const char *ip, const int port)
{
    esp_err_t ret = 0;
    uint32_t start_ts;
    uint32_t timeout = 10000;
    struct sockaddr_in sock_addr;
    int s;

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = ipaddr_addr(ip);
    sock_addr.sin_port = htons(port);

    start_ts = esp_log_timestamp();
    do {
        s = socket(AF_INET, SOCK_STREAM, 0/*IPPROTO_TCP*/);
        if (s < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        }
        ret = connect(s, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
        if (ret == 0) {
            *sock = s;
            break;
        } else if (s > 0) {
            close(s);
        }
    } while (esp_log_timestamp() - start_ts < timeout);
    return ret;
}

void *wifi_client_thread_routine(void *p_param)
{

    esp_err_t ret = 0;
    char buffer[256];

    while (1) {
        int s;
        ret = wifi_unit_client_establish(&s, CONFIG_SERVER_ADDRESS, CONFIG_SERVER_PORT);
        ESP_LOGI(TAG, "create socket: result %d, errno %d, %s:%d", ret, errno, CONFIG_SERVER_ADDRESS, CONFIG_SERVER_PORT);

        char data[] = "0123456789abcdefghijklmnopqrstuvwxyz";
        write(s, data, sizeof(data));

        while (1) {
            ret = read(s, buffer, sizeof(buffer));
            if (ret < 0) {
                close(s);
                break;
            }
            write(s, data, sizeof(data));
        }

    }

    return NULL;
}
