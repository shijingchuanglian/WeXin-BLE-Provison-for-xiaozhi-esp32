#include <string.h>
#include <cstdio>
#include "provision_manager.h"

#include <memory>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_mac.h>
#include <esp_netif.h>
#include <lwip/ip_addr.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <cJSON.h>
#include <esp_smartconfig.h>
#include "system_info.h"
#include "settings.h"
#include <esp_lvgl_port.h>
#include "settings.h"
#include "ssid_manager.h"
#include "assets/lang_config.h"

LV_FONT_DECLARE(font_puhui_14_1);

// 根据不同分辨率加载匹配尺寸的二维码头文件并定义宏
#ifdef CONFIG_WEIXIN_PROV_QRCODE_RES_128
#include "prov-image/img_prov_qrcode_sjai_128.h"
#elif CONFIG_WEIXIN_PROV_QRCODE_RES_32
#include "prov-image/img_prov_qrcode_sjai_32.h"
#else /*默认WEIXIN_PROV_QRCODE_RES_64*/
#include "prov-image/img_prov_qrcode_sjai_64.h"
#endif


#define TAG "ProvisionManager"

/* Wi-Fi Provisioning Manager Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <network_provisioning/manager.h>

#ifdef CONFIG_WEIXIN_PROV_TRANSPORT_BLE
#include <network_provisioning/scheme_ble.h>
#endif /* CONFIG_WEIXIN_PROV_TRANSPORT_BLE */

#ifdef CONFIG_WEIXIN_PROV_TRANSPORT_SOFTAP
#include <network_provisioning/scheme_softap.h>
#endif /* CONFIG_WEIXIN_PROV_TRANSPORT_SOFTAP */
#include "qrcode.h"


#if CONFIG_WEIXIN_PROV_SECURITY_VERSION_2
#if CONFIG_WEIXIN_PROV_SEC2_DEV_MODE
#define WEIXIN_PROV_SEC2_USERNAME          "wifiprov"
#define WEIXIN_PROV_SEC2_PWD               "abcd1234"

/* This salt,verifier has been generated for username = "wifiprov" and password = "abcd1234"
 * IMPORTANT NOTE: For production cases, this must be unique to every device
 * and should come from device manufacturing partition.*/
static const char sec2_salt[] = {
    0x03, 0x6e, 0xe0, 0xc7, 0xbc, 0xb9, 0xed, 0xa8, 0x4c, 0x9e, 0xac, 0x97, 0xd9, 0x3d, 0xec, 0xf4
};

static const char sec2_verifier[] = {
    0x7c, 0x7c, 0x85, 0x47, 0x65, 0x08, 0x94, 0x6d, 0xd6, 0x36, 0xaf, 0x37, 0xd7, 0xe8, 0x91, 0x43,
    0x78, 0xcf, 0xfd, 0x61, 0x6c, 0x59, 0xd2, 0xf8, 0x39, 0x08, 0x12, 0x72, 0x38, 0xde, 0x9e, 0x24,
    0xa4, 0x70, 0x26, 0x1c, 0xdf, 0xa9, 0x03, 0xc2, 0xb2, 0x70, 0xe7, 0xb1, 0x32, 0x24, 0xda, 0x11,
    0x1d, 0x97, 0x18, 0xdc, 0x60, 0x72, 0x08, 0xcc, 0x9a, 0xc9, 0x0c, 0x48, 0x27, 0xe2, 0xae, 0x89,
    0xaa, 0x16, 0x25, 0xb8, 0x04, 0xd2, 0x1a, 0x9b, 0x3a, 0x8f, 0x37, 0xf6, 0xe4, 0x3a, 0x71, 0x2e,
    0xe1, 0x27, 0x86, 0x6e, 0xad, 0xce, 0x28, 0xff, 0x54, 0x46, 0x60, 0x1f, 0xb9, 0x96, 0x87, 0xdc,
    0x57, 0x40, 0xa7, 0xd4, 0x6c, 0xc9, 0x77, 0x54, 0xdc, 0x16, 0x82, 0xf0, 0xed, 0x35, 0x6a, 0xc4,
    0x70, 0xad, 0x3d, 0x90, 0xb5, 0x81, 0x94, 0x70, 0xd7, 0xbc, 0x65, 0xb2, 0xd5, 0x18, 0xe0, 0x2e,
    0xc3, 0xa5, 0xf9, 0x68, 0xdd, 0x64, 0x7b, 0xb8, 0xb7, 0x3c, 0x9c, 0xfc, 0x00, 0xd8, 0x71, 0x7e,
    0xb7, 0x9a, 0x7c, 0xb1, 0xb7, 0xc2, 0xc3, 0x18, 0x34, 0x29, 0x32, 0x43, 0x3e, 0x00, 0x99, 0xe9,
    0x82, 0x94, 0xe3, 0xd8, 0x2a, 0xb0, 0x96, 0x29, 0xb7, 0xdf, 0x0e, 0x5f, 0x08, 0x33, 0x40, 0x76,
    0x52, 0x91, 0x32, 0x00, 0x9f, 0x97, 0x2c, 0x89, 0x6c, 0x39, 0x1e, 0xc8, 0x28, 0x05, 0x44, 0x17,
    0x3f, 0x68, 0x02, 0x8a, 0x9f, 0x44, 0x61, 0xd1, 0xf5, 0xa1, 0x7e, 0x5a, 0x70, 0xd2, 0xc7, 0x23,
    0x81, 0xcb, 0x38, 0x68, 0xe4, 0x2c, 0x20, 0xbc, 0x40, 0x57, 0x76, 0x17, 0xbd, 0x08, 0xb8, 0x96,
    0xbc, 0x26, 0xeb, 0x32, 0x46, 0x69, 0x35, 0x05, 0x8c, 0x15, 0x70, 0xd9, 0x1b, 0xe9, 0xbe, 0xcc,
    0xa9, 0x38, 0xa6, 0x67, 0xf0, 0xad, 0x50, 0x13, 0x19, 0x72, 0x64, 0xbf, 0x52, 0xc2, 0x34, 0xe2,
    0x1b, 0x11, 0x79, 0x74, 0x72, 0xbd, 0x34, 0x5b, 0xb1, 0xe2, 0xfd, 0x66, 0x73, 0xfe, 0x71, 0x64,
    0x74, 0xd0, 0x4e, 0xbc, 0x51, 0x24, 0x19, 0x40, 0x87, 0x0e, 0x92, 0x40, 0xe6, 0x21, 0xe7, 0x2d,
    0x4e, 0x37, 0x76, 0x2f, 0x2e, 0xe2, 0x68, 0xc7, 0x89, 0xe8, 0x32, 0x13, 0x42, 0x06, 0x84, 0x84,
    0x53, 0x4a, 0xb3, 0x0c, 0x1b, 0x4c, 0x8d, 0x1c, 0x51, 0x97, 0x19, 0xab, 0xae, 0x77, 0xff, 0xdb,
    0xec, 0xf0, 0x10, 0x95, 0x34, 0x33, 0x6b, 0xcb, 0x3e, 0x84, 0x0f, 0xb9, 0xd8, 0x5f, 0xb8, 0xa0,
    0xb8, 0x55, 0x53, 0x3e, 0x70, 0xf7, 0x18, 0xf5, 0xce, 0x7b, 0x4e, 0xbf, 0x27, 0xce, 0xce, 0xa8,
    0xb3, 0xbe, 0x40, 0xc5, 0xc5, 0x32, 0x29, 0x3e, 0x71, 0x64, 0x9e, 0xde, 0x8c, 0xf6, 0x75, 0xa1,
    0xe6, 0xf6, 0x53, 0xc8, 0x31, 0xa8, 0x78, 0xde, 0x50, 0x40, 0xf7, 0x62, 0xde, 0x36, 0xb2, 0xba
};
#endif

static esp_err_t weixin_prov_get_sec2_salt(const char **salt, uint16_t *salt_len)
{
#if CONFIG_WEIXIN_PROV_SEC2_DEV_MODE
    ESP_LOGI(TAG, "Development mode: using hard coded salt");
    *salt = sec2_salt;
    *salt_len = sizeof(sec2_salt);
    return ESP_OK;
#elif CONFIG_WEIXIN_PROV_SEC2_PROD_MODE
    ESP_LOGE(TAG, "Not implemented!");
    return ESP_FAIL;
#endif
}

static esp_err_t weixin_prov_get_sec2_verifier(const char **verifier, uint16_t *verifier_len)
{
#if CONFIG_WEIXIN_PROV_SEC2_DEV_MODE
    ESP_LOGI(TAG, "Development mode: using hard coded verifier");
    *verifier = sec2_verifier;
    *verifier_len = sizeof(sec2_verifier);
    return ESP_OK;
#elif CONFIG_WEIXIN_PROV_SEC2_PROD_MODE
    /* This code needs to be updated with appropriate implementation to provide verifier */
    ESP_LOGE(TAG, "Not implemented!");
    return ESP_FAIL;
#endif
}
#endif


/* Signal Wi-Fi events on this event-group */
const int WIFI_CONNECTED_EVENT = BIT0;


#define PROV_QR_VERSION         "v1"
#define PROV_TRANSPORT_SOFTAP   "softap"
#define PROV_TRANSPORT_BLE      "ble"
#define QRCODE_BASE_URL         "https://espressif.github.io/esp-jumpstart/qrcode.html"

/* Event handler for catching system events */
void ProvisionManager::event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
 #ifdef CONFIG_WEIXIN_RESET_PROV_MGR_ON_FAILURE
    static int retries;
#endif
    if (event_base == NETWORK_PROV_EVENT) {
        auto& prov_man = ProvisionManager::GetInstance(); // 获取类的实例
        switch (event_id) {
        case NETWORK_PROV_START:
            ESP_LOGI(TAG, "Provisioning started");
            break;
        case NETWORK_PROV_WIFI_CRED_RECV: {
            if (event_data != NULL) {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                memcpy(prov_man.default_ssid_, wifi_sta_cfg->ssid, sizeof(wifi_sta_cfg->ssid));
                memcpy(prov_man.default_password_, wifi_sta_cfg->password, sizeof(wifi_sta_cfg->password));
                char log_msg[256];   
                snprintf(log_msg, sizeof(log_msg), "Received Wi-Fi credentials\n\tSSID     : %s\n\tPassword : %s\n",
                        (const char *) prov_man.default_ssid_,
                        (const char *) prov_man.default_password_);
                prov_man.alert_prov("配网模式","成功接收到Wi-Fi凭据!","happy");
                ESP_LOGI(TAG,"%s",log_msg);

            } else {
                ESP_LOGE(TAG, "NETWORK_PROV_WIFI_CRED_RECV: event_data is NULL");
            }
            
            break;
        }
        case NETWORK_PROV_WIFI_CRED_FAIL: {
            network_prov_wifi_sta_fail_reason_t *reason = (network_prov_wifi_sta_fail_reason_t *)event_data;
            char log_msg[256];
            
            snprintf(log_msg, sizeof(log_msg), "Provisioning failed!\n\tReason : %s\n\tPlease reset to factory and retry provisioning",
                     (*reason == NETWORK_PROV_WIFI_STA_AUTH_ERROR)?
                     "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
            prov_man.alert_prov("配网消息","Wi-Fi凭证验证失败,请检查密码是否正确。","sad");
            ESP_LOGE(TAG,"%s",log_msg);
#ifdef CONFIG_WEIXIN_RESET_PROV_MGR_ON_FAILURE
            retries++;
            if (retries >= CONFIG_WEIXIN_PROV_MGR_MAX_RETRY_CNT) {
                prov_man.alert_prov("配网消息","Wi-Fi凭证验证失败,请检查密码是否正确。","sad");
                ESP_LOGI(TAG, "Failed to connect with provisioned AP, reseting provisioned credentials");
                network_prov_mgr_reset_wifi_sm_state_on_failure();
                retries = 0;
            }
#endif
            break;
        }
        case NETWORK_PROV_WIFI_CRED_SUCCESS:
        prov_man.alert_prov("配网消息","微信配网成功！","happy");    
        ESP_LOGI(TAG, "Provisioning successful");
#ifdef CONFIG_WEIXIN_RESET_PROV_MGR_ON_FAILURE
            retries = 0;
#endif
            break;
        case NETWORK_PROV_END:
            /* De-initialize manager once provisioning is finished */
            // network_prov_mgr_deinit();
            break;
        default:
            break;
        }
    } else if (event_base == WIFI_EVENT) {
        auto& prov_man = ProvisionManager::GetInstance(); // 获取类的实例
        switch (event_id) {
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            prov_man.alert_prov("Wi-Fi消息","Wi-Fi连接中断,正在重连Wi-Fi......","sad");   
            ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
            esp_wifi_connect();
            break;
#ifdef CONFIG_WEIXIN_PROV_TRANSPORT_SOFTAP
        case WIFI_EVENT_AP_STACONNECTED:
            prov_man.alert_prov("Wi-Fi消息","Wi-Fi热点通信链路建立成功!","happy");   
            ESP_LOGI(TAG, "SoftAP transport: Connected!");
            break;
        case WIFI_EVENT_AP_STADISCONNECTED:
            // prov_man.alert_prov("Wi-Fi消息","Wi-Fi热点通信链路中断!","sad");   
            ESP_LOGI(TAG, "SoftAP transport: Disconnected!");
            break;
#endif
        default:
            break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "Wi-Fi连接成功，IP地址为:" IPSTR, IP2STR(&event->ip_info.ip));
        auto& prov_man = ProvisionManager::GetInstance(); // 获取类的实例
        prov_man.alert_prov("IP消息",log_msg,"happy");   
        ESP_LOGI(TAG,"%s",log_msg);
        //将配网成功的wifi信息存储到nvs中
        Settings settings("wifi", true);
        settings.SetInt("force_ap", 0);
        if(prov_man.default_ssid_[0] != '\0' && prov_man.default_password_[0] != '\0')
        {
            prov_man.Save(prov_man.default_ssid_, prov_man.default_password_); 
        }
        prov_man.SaveAdvancedNetConfigToNVS();
        xEventGroupSetBits(prov_man.wifi_event_group, WIFI_CONNECTED_EVENT);
#ifdef CONFIG_WEIXIN_PROV_TRANSPORT_BLE
    } else if (event_base == PROTOCOMM_TRANSPORT_BLE_EVENT) {
        auto &prov_man = ProvisionManager::GetInstance(); // 获取类的实例
        switch (event_id) {
        case PROTOCOMM_TRANSPORT_BLE_CONNECTED:
            prov_man.alert_prov("蓝牙消息","蓝牙通信链路建立成功!","happy");   
            ESP_LOGI(TAG, "BLE transport: Connected!");
            break;
        case PROTOCOMM_TRANSPORT_BLE_DISCONNECTED:
            prov_man.alert_prov("蓝牙消息","蓝牙通信链路中断!","sad");   
            ESP_LOGI(TAG, "BLE transport: Disconnected!");
            break;
        default:
            break;
        }
#endif
    } else if (event_base == PROTOCOMM_SECURITY_SESSION_EVENT) {
        auto &prov_man = ProvisionManager::GetInstance(); // 获取类的实例
        switch (event_id) {
        case PROTOCOMM_SECURITY_SESSION_SETUP_OK:
            prov_man.alert_prov("蓝牙消息","蓝牙安全校验已启动!","happy");   
            ESP_LOGI(TAG, "Secured session established!");
            break;
        case PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS:
            prov_man.alert_prov("蓝牙消息","收到无效的蓝牙安全验证参数!","sad");   
            ESP_LOGE(TAG, "Received invalid security parameters for establishing secure session!");
            break;
        case PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH:
            prov_man.alert_prov("蓝牙消息","收到无效的蓝牙安全验证参数,请检查用户名或POP是否正确。","sad");   
            ESP_LOGE(TAG, "Received incorrect username and/or PoP for establishing secure session!");
            break;
        default:
            break;
        }
    }
}

ProvisionManager& ProvisionManager::GetInstance() {
    static ProvisionManager instance;
    return instance;
}

ProvisionManager::ProvisionManager()
:event_group_(xEventGroupCreate()),
 wifi_event_group(xEventGroupCreate()),
 ssid_prefix_(""),
 language_(""),
 board_(&Board::GetInstance())
{

    // 通过 Board 类获取 Display 指针
    board_ = &Board::GetInstance();
    if (board_) {
        display_ = board_->GetDisplay();
    }

}

ProvisionManager::~ProvisionManager()
{
    if (event_group_) {
        vEventGroupDelete(event_group_);
    }
    if (wifi_event_group) {
        vEventGroupDelete(wifi_event_group);
    }
    // Unregister event handlers if they were registered
    if (instance_any_id_) {
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id_);
    }
    if (instance_got_ip_) {
        esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip_);
    }
}


bool ProvisionManager::GetEnterProvisionMode(){

    return enter_provision_mode_;

}
void ProvisionManager::SetEnterProvisionMode(bool enter_provision_mode){
    
    enter_provision_mode_ = enter_provision_mode;
    
}

void ProvisionManager::SetLanguage(const std::string &&language)
{
    language_ = language;
}

void ProvisionManager::SetSsidPrefix(const std::string &&ssid_prefix)
{
    ssid_prefix_ = ssid_prefix;
}



void ProvisionManager::InitAdvancedNetConfigInNVS() {
           // 打开NVS
            nvs_handle_t nvs;
            esp_err_t err = nvs_open("wifi", NVS_READWRITE, &nvs);
            if (err != ESP_OK) {
                ESP_LOGI(TAG,"Failed to open NVS");
                return;
            }

            // 初始化OTA URL
             // 读取OTA URL
            char ota_url[256] = {0};;
            size_t ota_url_size = sizeof(ota_url);
            err = nvs_get_str(nvs, "ota_url", ota_url, &ota_url_size);
            if (err == ESP_OK) {
                ota_url_ = ota_url;
            }else{
                ota_url_ = "https://api.tenclass.net/xiaozhi/ota/";
                err = nvs_set_str(nvs, "ota_url", ota_url_.c_str());
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to save OTA URL: %d", err);
                }
            } 

            

            // 初始化WiFi功率72=18dBm（默认最大值）
            err = nvs_get_i8(nvs, "max_tx_power", &max_tx_power_);
            if (err == ESP_OK) {

                ESP_LOGI(TAG, "WiFi max tx power from NVS: %d", max_tx_power_);

                if(max_tx_power_ < 0 || max_tx_power_ > 78){
                    ESP_LOGI(TAG, "WiFi max tx power Reset to: %d", max_tx_power_);
                    max_tx_power_ = 60;
                }
            }

            err = nvs_set_i8(nvs, "max_tx_power", max_tx_power_);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to save WiFi power: %d", err);
            }
            

            // 初始化BSSID记忆设置，默认值为 true
            uint8_t  remember_bssid = 0 ;
            err = nvs_get_u8(nvs, "remember_bssid", &remember_bssid);
            if (err == ESP_OK) {
                remember_bssid_ = remember_bssid;
            } else {
                remember_bssid_ = true; // 默认值
            }
            err = nvs_set_u8(nvs, "remember_bssid", remember_bssid_);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to save remember_bssid: %d", err);
            }

            // 提交更改
            err = nvs_commit(nvs);
            nvs_close(nvs);
            if (err != ESP_OK) {
                ESP_LOGE(TAG,"Failed to save configuration to NVS");
            }
           
           
}

void ProvisionManager::SaveAdvancedNetConfigToNVS() {

            // 打开NVS
            nvs_handle_t nvs;

            esp_err_t openerr = nvs_open("wifi", NVS_READWRITE, &nvs);
            if (openerr != ESP_OK) {
                ESP_LOGI(TAG,"Failed to open NVS");
                return;
            }

            // 保存OTA URL
            if (ota_url_.empty()) {
                //如果为空，则ota_url=CONFIG_OTA_VERSION_URL,否则ota_url="https://api.tenclass.net/xiaozhi/ota/"
                
                if (CONFIG_OTA_VERSION_URL[0] == '\0') {

                ota_url_ = CONFIG_OTA_VERSION_URL;

                }else{

                ota_url_ = "https://api.tenclass.net/xiaozhi/ota/";
            }

            }
            esp_err_t otaerr = nvs_set_str(nvs, "ota_url", ota_url_.c_str());
            if (otaerr != ESP_OK) {
                ESP_LOGE(TAG, "Failed to save OTA URL: %d", otaerr);
            }


            // 保存WiFi功率
            esp_err_t wifi_err = esp_wifi_get_max_tx_power(&max_tx_power_);
            if (wifi_err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to get WiFi max tx power: %d", wifi_err);
                // 设置默认值，假设 72=18dBm 是默认最大值
                max_tx_power_ = 72; 
            }
            ESP_ERROR_CHECK(esp_wifi_get_max_tx_power(&max_tx_power_));
            esp_err_t txerr = nvs_set_i8(nvs, "max_tx_power", max_tx_power_);
            if (txerr != ESP_OK) {
                ESP_LOGE(TAG, "Failed to save WiFi power: %d", txerr);
            }

            // 保存BSSID记忆设置
            if (!remember_bssid_) { 
                remember_bssid_ = true;
            }
            esp_err_t bssiderr = nvs_set_u8(nvs, "remember_bssid", remember_bssid_);
            if (bssiderr != ESP_OK) {
                ESP_LOGE(TAG, "Failed to save remember_bssid: %d", bssiderr);
            }


            // 提交更改
            esp_err_t err = nvs_commit(nvs);
            nvs_close(nvs);
            if (err != ESP_OK) {
                ESP_LOGE(TAG,"Failed to save configuration to NVS");
            }
}






bool ProvisionManager::WaitForConnected(int timeout_ms) {
    auto bits = xEventGroupWaitBits(ProvisionManager::GetInstance().wifi_event_group, WIFI_CONNECTED_EVENT, pdFALSE, pdFALSE, timeout_ms / portTICK_PERIOD_MS);
    return (bits & WIFI_CONNECTED_EVENT) != 0;
}




void ProvisionManager::EnterProvisionMode(std::string ssid_prefix,std::string language,bool force_provisioning_enable){

    
    if (display_) {
        display_->SetStatus(Lang::Strings::WIFI_CONFIG_MODE);
    }

    ssid_prefix_ = ssid_prefix;
    language_ = language;
    enter_provision_mode_ = force_provisioning_enable; 
    
    /* Initialize Advanced Net Config In NVS */
    InitAdvancedNetConfigInNVS();
  

    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());

    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    


    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
#ifdef CONFIG_WEIXIN_PROV_TRANSPORT_BLE
    ESP_ERROR_CHECK(esp_event_handler_register(PROTOCOMM_TRANSPORT_BLE_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
#endif
    ESP_ERROR_CHECK(esp_event_handler_register(PROTOCOMM_SECURITY_SESSION_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    /* Initialize Wi-Fi including netif with default config */
    esp_netif_create_default_wifi_sta();
#ifdef CONFIG_WEIXIN_PROV_TRANSPORT_SOFTAP
    esp_netif_create_default_wifi_ap();
#endif /* CONFIG_WEIXIN_PROV_TRANSPORT_SOFTAP */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Configuration for the provisioning manager */
    network_prov_mgr_config_t config = {
        /* What is the Provisioning Scheme that we want ?
         * network_prov_scheme_softap or network_prov_scheme_ble */
#ifdef CONFIG_WEIXIN_PROV_TRANSPORT_BLE
        .scheme = network_prov_scheme_ble,
#endif /* CONFIG_WEIXIN_PROV_TRANSPORT_BLE */
#ifdef CONFIG_WEIXIN_PROV_TRANSPORT_SOFTAP
        .scheme = network_prov_scheme_softap,
#endif /* CONFIG_WEIXIN_PROV_TRANSPORT_SOFTAP */

        /* Any default scheme specific event handler that you would
         * like to choose. Since our example application requires
         * neither BT nor BLE, we can choose to release the associated
         * memory once provisioning is complete, or not needed
         * (in case when device is already provisioned). Choosing
         * appropriate scheme specific event handler allows the manager
         * to take care of this automatically. This can be set to
         * NETWORK_PROV_EVENT_HANDLER_NONE when using network_prov_scheme_softap*/
#ifdef CONFIG_WEIXIN_PROV_TRANSPORT_BLE
        .scheme_event_handler = NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
#endif /* CONFIG_WEIXIN_PROV_TRANSPORT_BLE */
#ifdef CONFIG_WEIXIN_PROV_TRANSPORT_SOFTAP
        .scheme_event_handler = NETWORK_PROV_EVENT_HANDLER_NONE
#endif /* CONFIG_WEIXIN_PROV_TRANSPORT_SOFTAP */
    };

    /* Initialize provisioning manager with the
     * configuration parameters set above */
    ESP_ERROR_CHECK(network_prov_mgr_init(config));

    bool provisioned = false;

#ifdef CONFIG_WEIXIN_RESET_PROVISIONED
    network_prov_mgr_reset_wifi_provisioning();
#else
    /* Let's find out if the device is provisioned */
    ESP_ERROR_CHECK(network_prov_mgr_is_wifi_provisioned(&provisioned));

#endif
    /* If device is not yet provisioned start provisioning service 
    if device need to reset network,entr_provision_mode_ will be true,start provisioning service
    */

    if (!provisioned || enter_provision_mode_) {
        ESP_LOGI(TAG, "Starting provisioning");

        /* What is the Device Service Name that we want
         * This translates to :
         *     - Wi-Fi SSID when scheme is network_prov_scheme_softap
         *     - device name when scheme is network_prov_scheme_ble
         */
        // char service_name[12];
        get_device_service_name(service_name, sizeof(service_name));
        // ble_service_name_ = service_name;

        /* What is the security level that we want (0, 1, 2):
         *      - NETWORK_PROV_SECURITY_0 is simply plain text communication.
         *      - NETWORK_PROV_SECURITY_1 is secure communication which consists of secure handshake
         *          using X25519 key exchange and proof of possession (pop) and AES-CTR
         *          for encryption/decryption of messages.
         *      - NETWORK_PROV_SECURITY_2 SRP6a based authentication and key exchange
         *        + AES-GCM encryption/decryption of messages
         */
#ifdef CONFIG_WEIXIN_PROV_SECURITY_VERSION_0
        network_prov_security_t security = NETWORK_PROV_SECURITY_0;

        /* Do we want a proof-of-possession (ignored if Security 0 is selected):
         *      - this should be a string with length > 0
         *      - NULL if not used
         */
        const char *pop = NULL;

        /* This is the structure for passing security parameters
         * for the protocomm security 1.
         */
        network_prov_security1_params_t *sec_params = pop;

        // const char *username  = NULL;


#elif CONFIG_WEIXIN_PROV_SECURITY_VERSION_1
        
        network_prov_security_t security = NETWORK_PROV_SECURITY_1;

        /* Do we want a proof-of-possession (ignored if Security 0 is selected):
         *      - this should be a string with length > 0
         *      - NULL if not used
         */
        const char *pop = "abcd1234";

        /* This is the structure for passing security parameters
         * for the protocomm security 1.
         */
        network_prov_security1_params_t *sec_params = pop;

        const char *username  = NULL;

#elif CONFIG_WEIXIN_PROV_SECURITY_VERSION_2
        network_prov_security_t security = NETWORK_PROV_SECURITY_2;
        /* The username must be the same one, which has been used in the generation of salt and verifier */

#if CONFIG_WEIXIN_PROV_SEC2_DEV_MODE
        /* This pop field represents the password that will be used to generate salt and verifier.
         * The field is present here in order to generate the QR code containing password.
         * In production this password field shall not be stored on the device */
        const char *username  = WEIXIN_PROV_SEC2_USERNAME;
        const char *pop = WEIXIN_PROV_SEC2_PWD;
#elif CONFIG_WEIXIN_PROV_SEC2_PROD_MODE
        /* The username and password shall not be embedded in the firmware,
         * they should be provided to the user by other means.
         * e.g. QR code sticker */
        const char *username  = NULL;
        const char *pop = NULL;
#endif
        /* This is the structure for passing security parameters
         * for the protocomm security 2.
         * If dynamically allocated, sec2_params pointer and its content
         * must be valid till NETWORK_PROV_END event is triggered.
         */
        network_prov_security2_params_t sec2_params = {};

        ESP_ERROR_CHECK(weixin_prov_get_sec2_salt(&sec2_params.salt, &sec2_params.salt_len));
        ESP_ERROR_CHECK(weixin_prov_get_sec2_verifier(&sec2_params.verifier, &sec2_params.verifier_len));

        network_prov_security2_params_t *sec_params = &sec2_params;
#endif
        /* What is the service key (could be NULL)
         * This translates to :
         *     - Wi-Fi password when scheme is network_prov_scheme_softap
         *          (Minimum expected length: 8, maximum 64 for WPA2-PSK)
         *     - simply ignored when scheme is network_prov_scheme_ble
         */
        const char *service_key = NULL;

#ifdef CONFIG_WEIXIN_PROV_TRANSPORT_BLE
        /* This step is only useful when scheme is network_prov_scheme_ble. This will
         * set a custom 128 bit UUID which will be included in the BLE advertisement
         * and will correspond to the primary GATT service that provides provisioning
         * endpoints as GATT characteristics. Each GATT characteristic will be
         * formed using the primary service UUID as base, with different auto assigned
         * 12th and 13th bytes (assume counting starts from 0th byte). The client side
         * applications must identify the endpoints by reading the User Characteristic
         * Description descriptor (0x2901) for each characteristic, which contains the
         * endpoint name of the characteristic */
        //前10字节："sjcaixga01" 的ASCII码（每个字符对应1字节）

        // 获取 MAC 地址
        std::string macStr = SystemInfo::GetMacAddress();
        uint8_t mac[6];
        // 假设 MAC 地址字符串格式为 "XX:XX:XX:XX:XX:XX"
        sscanf(macStr.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", 
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

        ESP_LOGE(TAG, "GET efuse MAC address: %s", macStr.c_str());

        uint8_t custom_service_uuid[] = {
            // "s" "j" "c" "a" "i" "x" "g" "a" "0" "1"
            0x73, 0x6A, 0x63, 0x61, 0x69, 0x78, 0x67, 0x61, 0x30, 0x31,
            // 后6字节：设备MAC地址（依次填充）
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
        };


        /* If your build fails with linker errors at this point, then you may have
         * forgotten to enable the BT stack or BTDM BLE settings in the SDK (e.g. see
         * the sdkconfig.defaults in the example project) */
        network_prov_scheme_ble_set_service_uuid(custom_service_uuid);
        char log_msg[100];
        snprintf(log_msg, sizeof(log_msg), "Custom service UUID set to %s", custom_service_uuid);

        
        ESP_LOGI(TAG, "%s",log_msg);
#endif /* CONFIG_WEIXIN_PROV_TRANSPORT_BLE */

        /* An optional endpoint that applications can create if they expect to
         * get some additional custom data during provisioning workflow.
         * The endpoint name can be anything of your choice.
         * This call must be made before starting the provisioning.
         */
        network_prov_mgr_endpoint_create("custom-data");

        /* Do not stop and de-init provisioning even after success,
         * so that we can restart it later. */
#ifdef CONFIG_WEIXIN_REPROVISIONING
        network_prov_mgr_disable_auto_stop(1000);
#endif
        /* Start provisioning service */
        ESP_ERROR_CHECK(network_prov_mgr_start_provisioning(security, (const void *) sec_params, service_name, service_key));

        /* The handler for the optional endpoint created above.
         * This call must be made after starting the provisioning, and only if the endpoint
         * has already been created above.
         */
        network_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);

        /* Uncomment the following to wait for the provisioning to finish and then release
         * the resources of the manager. Since in this case de-initialization is triggered
         * by the default event loop handler, we don't need to call the following */
        // network_prov_mgr_wait();
        // network_prov_mgr_deinit();
        /* Print QR code for provisioning */
   
   
    
#ifdef CONFIG_WEIXIN_PROV_SHOW_QR

#ifdef CONFIG_WEIXIN_PROV_QRCODE_RES_128
        wifi_prov_show_qr(128,128,&img_prov_qrcode_sjai_128);
#elif CONFIG_WEIXIN_PROV_QRCODE_RES_32
        wifi_prov_show_qr(32,32,&img_prov_qrcode_sjai_32);
#else  /* 默认WEIXIN_PROV_QRCODE_RES_64*/
        wifi_prov_show_qr(64,64,&img_prov_qrcode_sjai_64);
#endif  
   
#else
    char prov_alert[256];
    snprintf(prov_alert, sizeof(prov_alert), "设备名称：\n%s\n请扫描二维码或微信搜索小程序“师景AI”，打开配网助手为您的设备配网。", service_name);
    ESP_LOGE(TAG,"%s",prov_alert);
    alert_prov("配网模式",prov_alert,"happy");

#endif /* CONFIG_WEIXIN_PROV_SHOW_QR */


    

    } else {

        alert_prov("Wi-Fi消息","Wi-Fi配网完成，启动Wi-Fi连接...","happy");
        
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");

        /* We don't need the manager as device is already provisioned,
         * so let's release it's resources */
        // network_prov_mgr_deinit();

        // /* Start Wi-Fi station */
        // wifi_init_sta();

    }

    /* Wait for Wi-Fi connection */
    xEventGroupWaitBits(ProvisionManager::GetInstance().wifi_event_group, WIFI_CONNECTED_EVENT, true, true, portMAX_DELAY);

    //force_ap need reprovision set to 1, no need reprovision set to 0
        // Settings settings("wifi", true);
        // settings.SetInt("force_ap", 0);
        // Save(default_ssid_, default_password_);
        // SaveAdvancedNetConfigToNVS();

    /* Start main application now */
#if CONFIG_WEIXIN_RESET_AFTER_PROVISIONED
    while (1) {


        alert_prov("配网消息","恭喜您，配网成功!","happy");

        ESP_LOGI(TAG, "Provisioning successful.");

        vTaskDelay(5000 / portTICK_PERIOD_MS);
        
    
        //auto &board = ProvBoard::GetInstance();
        if(display_!= nullptr){

            display_->SetChatMessage("配网消息","设备将在10秒后重启.....");

        }
        ESP_LOGI(TAG, "The device will reboot in 10 seconds.....");
        for (int i = 10; i > 0; i--) {
            char log_msg[256];   
            snprintf(log_msg, sizeof(log_msg), "设备将在%d秒后重启.....", i);

            alert_prov("配网消息",log_msg,"happy");
            ESP_LOGI(TAG, "%s", log_msg);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        // Reboot the device
        esp_restart();

    }
#else

 while (1) {


        alert_prov("配网消息","恭喜您，配网成功!","happy");

        ESP_LOGI(TAG, "Provisioning successful.");

        vTaskDelay(5000 / portTICK_PERIOD_MS);
        
    }
   
#endif

}


void ProvisionManager::wifi_init_sta(void)
{
    /* Start Wi-Fi in station mode */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void ProvisionManager::get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = ssid_prefix_.c_str();;
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

/* Handler for the optional provisioning endpoint registered by the application.
 * The data format can be chosen by applications. Here, we are using plain ascii text.
 * Applications can choose to use other formats like protobuf, JSON, XML, etc.
 * Note that memory for the response buffer must be allocated using heap as this buffer
 * gets freed by the protocomm layer once it has been sent by the transport layer.
 */
esp_err_t ProvisionManager::custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                   uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    if (inbuf) {
        ESP_LOGI(TAG, "XG custom_prov_data_handler Received data: %.*s", inlen, (char *)inbuf);
    }
    char response[] = "XG custom SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL) {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

void ProvisionManager::wifi_prov_print_qr(const char *name, const char *username, const char *pop, const char *transport)
{
    if (!name || !transport) {
        ESP_LOGW(TAG, "Cannot generate QR code payload. Data missing.");
        return;
    }
    char payload[150] = {0};
    if (pop) {
#if CONFIG_WEIXIN_PROV_SECURITY_VERSION_1
        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\"" \
                 ",\"pop\":\"%s\",\"transport\":\"%s\"}",
                 PROV_QR_VERSION, name, pop, transport);
#elif CONFIG_WEIXIN_PROV_SECURITY_VERSION_2
        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\"" \
                 ",\"username\":\"%s\",\"pop\":\"%s\",\"transport\":\"%s\"}",
                 PROV_QR_VERSION, name, username, pop, transport);
#endif
    } else {
        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\"" \
                 ",\"transport\":\"%s\",\"network\":\"wifi\"}",
                 PROV_QR_VERSION, name, transport);
    }
    //TODO: Add the network protocol type to the QR code payload
#ifdef CONFIG_WEIXIN_PROV_SHOW_QR
    ESP_LOGI(TAG, "Scan this QR code from the provisioning application for Provisioning.");
    esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();

    esp_qrcode_generate(&cfg, payload);
#endif /* CONFIG_WEIXIN_PROV_SHOW_QR */
    ESP_LOGI(TAG, "If QR code is not visible, copy paste the below URL in a browser.\n%s?data=%s", QRCODE_BASE_URL, payload);

    // 屏幕显示 WiFi 配网信息和二维码
    std::string alert_info = "Scan this QR code from the provisioning application for Provisioning.";
    alert_info += "\n\n";
    alert_info += "If QR code is not visible, copy paste the below URL in a browser.";
    alert_info += "\n\n";
    alert_info += QRCODE_BASE_URL;
    alert_info += "?data=";
    alert_info += payload;
    alert_info += "\n\n";
    
    if(display_!= nullptr){

        display_->SetChatMessage("PROV_EVENT",alert_info.c_str());

    }

    
    // 播报配置 WiFi 的提示
    // Application::GetInstance().Alert(Lang::Strings::WIFI_CONFIG_MODE, alert_info.c_str(), "", "");
}

void ProvisionManager::wifi_prov_show_qr(int imgWidth, int imgHeight,const lv_image_dsc_t* QRimageDsc)
{
    char prov_alert[256];
    snprintf(prov_alert, sizeof(prov_alert), "请微信扫描二维码为您的设备配网。如果二维码未展示，请微信搜索小程序“师景AI”打开配网助手。\n设备名称为：\n%s", service_name);
    ESP_LOGE(TAG,"%s",prov_alert);

     if (!display_) {
            ESP_LOGE(TAG, "无法获取显示设备");
            vTaskDelete(NULL);
            return;
        }
        
        // 创建画布（如果不存在）
        if (!HasCanvas()) {
            CreateCanvas();
        }

        // 获取屏幕尺寸
        int screen_width = display_->width();
        int screen_height = display_->height();

        // 计算二维码图片的位置（屏幕上半部分居中）
        int qr_x = (screen_width - imgWidth) / 2;
        int qr_y = (screen_height / 2 - imgHeight) / 2;

        // 绘制二维码图片
        DrawImageOnCanvas(qr_x, qr_y, imgWidth, imgHeight, QRimageDsc);
        ESP_LOGI(TAG, "显示二维码图片");

        // 计算提示信息标签的宽度和高度
        int labelWidth = screen_width * 0.8;
        int labelHeight = screen_height / 2 * 0.8;

        // 计算提示信息标签的位置（屏幕下半部分居中）
        int label_x = (screen_width - labelWidth) / 2;
        int label_y = screen_height / 2 + (screen_height / 2 - labelHeight) / 2;

        // 绘制提示信息标签
        DrawLabelOnCanvas(label_x, label_y, labelWidth, labelHeight, prov_alert);
    
}



Display* ProvisionManager::getDisplay(){
        
    return display_;
}



void ProvisionManager::alert_prov(const char* status, const char* message,const char* emotion) {
    

    ESP_LOGW(TAG, "Alert %s: %s", status, message);
    // auto display = Board::GetInstance().GetDisplay();

    // 若有画布则销毁，防止遮挡 alert 内容
    if (HasCanvas()) {
        DestroyCanvas();
    }

    display_->SetStatus(status);

    display_->SetEmotion(emotion);

    display_->SetChatMessage("system", message);

}

void ProvisionManager::dismiss_alert_prov() {
    
        // auto display = Board::GetInstance().GetDisplay();
        display_->SetStatus(Lang::Strings::STANDBY);
        display_->SetEmotion("neutral");
        display_->SetChatMessage("system", "");
}



void ProvisionManager::CreateCanvas() {
    DisplayLockGuard lock(display_);
    
    // 如果已经有画布，先销毁
    if (canvas_ != nullptr) {
        DestroyCanvas();
    }
    
    // 创建画布所需的缓冲区
    // 每个像素2字节(RGB565)
    size_t buf_size = display_->width() * display_->height() * 2;  // RGB565: 2 bytes per pixel
    
    // 分配内存，优先使用PSRAM
    canvas_buffer_ = heap_caps_malloc(buf_size, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (canvas_buffer_ == nullptr) {
        ESP_LOGE(TAG, "Failed to allocate canvas buffer");
        return;
    }
    
    // 获取活动屏幕
    lv_obj_t* screen = lv_screen_active();
    
    // 创建画布对象
    canvas_ = lv_canvas_create(screen);
    if (canvas_ == nullptr) {
        ESP_LOGE(TAG, "Failed to create canvas");
        heap_caps_free(canvas_buffer_);
        canvas_buffer_ = nullptr;
        return;
    }
    
    // 初始化画布
    lv_canvas_set_buffer(canvas_, canvas_buffer_, display_->width(), display_->height(), LV_COLOR_FORMAT_RGB565);
    
    // 设置画布位置为全屏
    lv_obj_set_pos(canvas_, 0,0);
    lv_obj_set_size(canvas_, display_->width(), display_->height());
    
    // 设置画布为白色（255,255,255，  透明(0,0,0)
    lv_canvas_fill_bg(canvas_, lv_color_make(255, 255, 255), LV_OPA_COVER);

    
    ESP_LOGI(TAG, "Canvas created successfully");
}



void ProvisionManager::DestroyCanvas() {
    DisplayLockGuard lock(display_);
    
    if (canvas_ != nullptr) {
        lv_obj_del(canvas_);
        canvas_ = nullptr;
    }
    
    if (canvas_buffer_ != nullptr) {
        heap_caps_free(canvas_buffer_);
        canvas_buffer_ = nullptr;
    }
    
    ESP_LOGI(TAG, "Canvas destroyed");
}

void ProvisionManager::DrawImageOnCanvas(int x,int y,int width, int height, const lv_image_dsc_t* imageDsc) {
    DisplayLockGuard lock(display_);
    
    // 确保有画布
    if (canvas_ == nullptr) {
        ESP_LOGE(TAG, "Canvas not created");
        return;
    }

        // 检查输入参数有效性
    if (imageDsc == nullptr || width <= 0 || height <= 0) {
        ESP_LOGE(TAG, "Invalid parameters for DrawImageOnCanvas: imageDsc=%p, width=%d, height=%d", imageDsc, width, height);
        return;
    }

    // 使用图层绘制图像到画布上
    lv_layer_t layer;
    lv_canvas_init_layer(canvas_, &layer);
    
    lv_draw_image_dsc_t draw_dsc;
    lv_draw_image_dsc_init(&draw_dsc);
    draw_dsc.src = imageDsc;
    
    // lv_area_t area = {x, y, x + width - 1, y + height - 1};
    // 使用计算好的居中坐标设置绘制区域
    lv_area_t area = {x, y, x + width - 1, y + height - 1};
    
    lv_draw_image(&layer, &draw_dsc, &area);

    lv_canvas_finish_layer(canvas_, &layer);
    
    
}

void ProvisionManager::DrawLabelOnCanvas(int x,int y,int width, int height, const char* message) {
    DisplayLockGuard lock(display_);
    
    // 确保有画布
    if (canvas_ == nullptr) {
        ESP_LOGE(TAG, "Canvas not created");
        return;
    }

    // 检查输入参数有效性
    if (message == nullptr || width <= 0 || height <= 0) {
        ESP_LOGE(TAG, "Invalid parameters for DrawLabelOnCanvas: message=%p, width=%d, height=%d", message, width, height);
        return;
    }
    
    // 使用图层绘制图像到画布上
    lv_layer_t layer;
    lv_canvas_init_layer(canvas_, &layer);

    lv_draw_label_dsc_t dsc;
    lv_draw_label_dsc_init(&dsc);
    dsc.color = lv_palette_main(LV_PALETTE_RED);
    dsc.decor = LV_TEXT_DECOR_UNDERLINE;
    dsc.font = &font_puhui_14_1;
    dsc.text = message;
    dsc.align = LV_TEXT_ALIGN_CENTER; 

    lv_area_t area = {x, y, x + width - 1, y + height - 1};

    lv_draw_label(&layer, &dsc, &area);

    lv_canvas_finish_layer(canvas_, &layer);

}


std::string ProvisionManager::GetSsid()
{
    // Get MAC and use it to generate a unique SSID
    uint8_t mac[6];
#if CONFIG_IDF_TARGET_ESP32P4
    esp_wifi_get_mac(WIFI_IF_AP, mac);
#else
    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP));
#endif
    char ssid[32];
    snprintf(ssid, sizeof(ssid), "%s-%02X%02X", ssid_prefix_.c_str(), mac[4], mac[5]);
    return std::string(ssid);
}

void ProvisionManager::Save(const std::string &ssid, const std::string &password)
{
    if(ssid.empty() || password.empty())
    {
        //日志提醒：保存失败，ssid或password为空
        ESP_LOGE(TAG, "Save failed, ssid or password is empty");
        return;
    }

    ESP_LOGI(TAG, "Save SSID: %s , password: %s", ssid.c_str(), password.c_str());

    SsidManager::GetInstance().AddSsid(ssid, password);
}

bool ProvisionManager::ConnectToWifi(const std::string &ssid, const std::string &password)
{

    if (ssid.empty()) {
        ESP_LOGE(TAG, "SSID cannot be empty");
        return false;
    }
    
    if (ssid.length() > 32) {  // WiFi SSID 最大长度
        ESP_LOGE(TAG, "SSID too long");
        return false;
    }
    
    is_connecting_ = true;
    esp_wifi_scan_stop();
    xEventGroupClearBits(event_group_, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    wifi_config_t wifi_config;
    bzero(&wifi_config, sizeof(wifi_config));
    strcpy((char *)wifi_config.sta.ssid, ssid.c_str());
    strcpy((char *)wifi_config.sta.password, password.c_str());
    wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifi_config.sta.failure_retry_cnt = 1;
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    auto ret = esp_wifi_connect();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect to WiFi: %d", ret);
        is_connecting_ = false;
        return false;
    }
    ESP_LOGI(TAG, "Connecting to WiFi %s", ssid.c_str());

    // Wait for the connection to complete for 5 seconds
    EventBits_t bits = xEventGroupWaitBits(event_group_, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdTRUE, pdFALSE, pdMS_TO_TICKS(10000));
    is_connecting_ = false;

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to WiFi %s", ssid.c_str());
        esp_wifi_disconnect();
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to connect to WiFi %s", ssid.c_str());
        return false;
    }
}




void ProvisionManager::rebootAndEnterProvisionMode(){
    Settings settings("wifi", true);
    settings.SetInt("force_ap", 1);

    display_->SetChatMessage("系统消息","设备即将重启，并进入配网模式......");
    vTaskDelay(pdMS_TO_TICKS(1000));
    // Reboot the device
    esp_restart();


}
void ProvisionManager::rebootAndclaerNVS(){

    Settings settings("wifi", true);
    settings.SetInt("force_ap", 0);
    ESP_ERROR_CHECK(nvs_flash_erase());
    ESP_ERROR_CHECK(nvs_flash_init());   

    display_->SetChatMessage("系统消息","设备即将重置，并进入配网模式......");
    vTaskDelay(pdMS_TO_TICKS(1000));
    // Reboot the device
    esp_restart();


}
void ProvisionManager::rebootAndEnterMainApplication(){

    auto& ssid_manager = SsidManager::GetInstance();
    auto ssid_list = ssid_manager.GetSsidList();
    if (ssid_list.empty()) {
        display_->SetChatMessage("系统消息","当前配网信息为空，进入聊天模式前需要先完成设备配网。");
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
    }

    Settings settings("wifi", true);
    settings.SetInt("force_ap", 0);

    SaveAdvancedNetConfigToNVS(); 
    display_->SetChatMessage("系统消息","设备即将重启，并进入聊天模式......");
    vTaskDelay(pdMS_TO_TICKS(1000));
    // Reboot the device
    esp_restart();


}


bool ProvisionManager::Lock(int timeout_ms) {
    return lvgl_port_lock(timeout_ms);
}

void ProvisionManager::Unlock() {
    lvgl_port_unlock();
}


