

#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_event.h>
#include <esp_wifi.h>

#include "application.h"
#include "system_info.h"
#include "settings.h"
#include "ssid_manager.h"
#include "provision_manager.h"
#include "system_info.h"

#define TAG "main"

extern "C" void app_main(void)
{
    bool enter_provision_mode = false;
    bool reset_enable = false;
    bool ssid_list_empty = false;


    // Initialize NVS flash for WiFi configuration
    // ESP_ERROR_CHECK(nvs_flash_erase());
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "Erasing NVS flash to fix corruption");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    

    // User can press BOOT button while starting to enter WiFi configuration mode
    //force_ap need reset set to 1, no need reset set to 0

    Settings settings("wifi", true);
    reset_enable = settings.GetInt("force_ap") == 1;

    ESP_LOGI(TAG, "reset_enable: %d", reset_enable);


    // If no WiFi SSID is configured, enter WiFi configuration mode
    auto& ssid_manager = SsidManager::GetInstance();
    auto ssid_list = ssid_manager.GetSsidList();
    if (ssid_list.empty()) {
        ssid_list_empty = true;
    }
    ESP_LOGI(TAG, "ssid_list_empty: %d", ssid_list_empty);

    // If reset_enable is true or ssid_list is empty, Launch the main application
    if (reset_enable==false && ssid_list_empty ==false) {
    
        
        enter_provision_mode = false;


 

        // Initialize the default event loop
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        // Launch the main application
        ESP_LOGI(TAG, "Launch the main application");
        Application::GetInstance().Start();
        return;

    }else{

        
        enter_provision_mode = true;
        ESP_LOGI(TAG, "enter_provision_mode: %d", enter_provision_mode);
        //force_ap need reset set to 1, no need reset set to 0"
        settings.SetInt("force_ap", 0);
       
    }



    if (enter_provision_mode) {

        ESP_LOGI(TAG, "print all namespaces before enter provisioning.");
        // Launch the application in provision mode
        std::string ssid_prefix_ = "PROV_";
        std::string language_ = "zh-CN";
        ProvisionManager::GetInstance().EnterProvisionMode(ssid_prefix_,language_,enter_provision_mode);
        return;
    }


}

