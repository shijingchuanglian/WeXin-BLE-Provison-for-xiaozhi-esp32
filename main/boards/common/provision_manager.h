#ifndef _WIFI_CONFIGURATION_AP_H_
#define _WIFI_CONFIGURATION_AP_H_

#include <string>
#include <vector>
#include <mutex>

#include <esp_http_server.h>
#include <esp_event.h>
#include <esp_timer.h>
#include <esp_netif.h>
#include <esp_wifi_types_generic.h>
#include "board.h"
#include "display/display.h"
#include "dns_server.h"
#include "cJSON.h"
#include "qrcode.h"

class ProvisionManager{
public:
    static ProvisionManager& GetInstance();
    void SetSsidPrefix(const std::string &&ssid_prefix);
    void SetLanguage(const std::string &&language);

    void EnterProvisionMode(std::string ssid_prefix = "PROV_",std::string language = "zh-CN",bool force_provisioning_enable = true);

    bool WaitForConnected(int timeout_ms) ;
    bool GetEnterProvisionMode();
    void SetEnterProvisionMode(bool enter_provision_mode);



    Display* getDisplay();
    void CreateCanvas();
    void DestroyCanvas();
    void DrawImageOnCanvas(int x,int y,int width, int height, const lv_image_dsc_t* imageDsc);
    void DrawLabelOnCanvas(int x,int y,int width, int height, const char* message);
    bool HasCanvas() const { return canvas_ != nullptr; }


    std::string GetSsid();
    std::string GetWebServerUrl();
    void Save(const std::string &ssid, const std::string &password);
    void InitAdvancedNetConfigInNVS();
    void SaveAdvancedNetConfigToNVS(); 
    
    void rebootAndEnterProvisionMode();
    void rebootAndclaerNVS();
    void rebootAndEnterMainApplication();

    void alert_prov(const char* status, const char* message,const char* emotion = "") ;


    // Delete copy constructor and assignment operator
    ProvisionManager(const ProvisionManager&) = delete;
    ProvisionManager& operator=(const ProvisionManager&) = delete;



private:
    // Private constructor
    ProvisionManager();
    ~ProvisionManager();

    EventGroupHandle_t event_group_;
    EventGroupHandle_t wifi_event_group;
    std::string ssid_prefix_;
    std::string language_;
    Board* board_;
    Display* display_;

    DnsServer dns_server_;
    httpd_handle_t server_ = NULL;

    esp_event_handler_instance_t instance_any_id_;
    esp_event_handler_instance_t instance_got_ip_;

    bool is_connecting_ = false;
    bool enter_provision_mode_ = false;
    esp_netif_t* ap_netif_ = nullptr;


    std::string ota_url_;
    int8_t max_tx_power_;
    bool remember_bssid_;

    char default_ssid_[33];
    char default_password_[65];
    char service_name[12];

    lv_obj_t* canvas_ = nullptr;
    void* canvas_buffer_ = nullptr;

    friend class DisplayLockGuard;
    bool Lock(int timeout_ms = 0);
    void Unlock();

    static const int WIFI_CONNECTED_BIT = BIT0;
    static const int WIFI_FAIL_BIT = BIT1;


    void wifi_init_sta(void);
    void get_device_service_name(char *service_name, size_t max);
    void wifi_prov_print_qr(const char *name, const char *username, const char *pop, const char *transport);
    void wifi_prov_show_qr(int imgWidth, int imgHeight,const lv_image_dsc_t* image);
    void dismiss_alert_prov();
    bool ConnectToWifi(const std::string &ssid, const std::string &password);
    


    // Event handlers
    static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);

    static esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                   uint8_t **outbuf, ssize_t *outlen, void *priv_data);


   
};

#endif // _WIFI_CONFIGURATION_AP_H_
