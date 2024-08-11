#include "DeviceFootPedal.h"
#include "esp-now-device-structs.h"
#include <Arduino.h>
#include <Device.h>
#include <DeviceManager.h>
#include <FirmwareChecker.h>
#include <FirmwareKicker.h>
#include <HostDriver.h>
#include <MQTTRemote.h>
#include <OtaHelper.h>
#include <WiFiHelper.h>
#include <esp_wifi.h>
#include <optional>
#include <string>

/*
 * NOTE:
 * This example depend on the following libraries:
 * johboh/MQTTRemote - for the MQTT implementation.
 * johboh/ConnectionHelper - For host OTA and WiFi setup (not for nodes OTA)
 */

using namespace std::placeholders;

#define TAG "example"

const char hostname[] = "my-host-driver";
const char wifi_ssid[] = "my-wifi-ssid";
const char wifi_password[] = "my-wifi-password";
const char mqtt_client_id[] = "my-host-driver";
const char mqtt_host[] = "192.168.1.100";
const char firmware_update_base_url[] = "http://192.168.1.100:8080/";
const char mqtt_username[] = "mqtt-username";
const char mqtt_password[] = "mqtt-password";
const int mqtt_port = 1883;
const int firmware_kicker_port = 82;

// Encyption key used for our own packet encryption (GCM).
// We are not using the esp-now encryption due to the peer limitation.
// The key should be the same for both the host and the node.
const char esp_now_encryption_key[] = "0123456789ABCDEF"; // Must be exact 16 bytes long. \0 does not count.

// Used to validate the integrity of the messages.
// The secret should be the same for both the host and the node.
const char esp_now_encryption_secret[] = "01234567"; // Must be exact 8 bytes long. \0 does not count.

// OTA and WiFI for host.
// Configure OTA and set hostname for identifying this device.
// Otherwise use defaults.
OtaHelper::Configuration ota_configuration = {
    .web_ota =
        {
            .id = hostname,
        },
};
OtaHelper _ota_helper(ota_configuration);
WiFiHelper _wifi_helper(hostname);

// MQTT for host.
MQTTRemote _mqtt_remote(mqtt_client_id, mqtt_host, mqtt_port, mqtt_username, mqtt_password);

// Add Two foot pedals, one left and one right.
// We have these as physical nodes.
DeviceFootPedal _device_foot_pedal_left(_mqtt_remote, 0x543204017648, "Left", [](uint8_t click) {
  switch (click) {
  case CLICK_SINGLE:
    // TODO: Handle click
    break;

  case CLICK_LONG:
    // TODO: Handle long click
    break;
  }
});
DeviceFootPedal _device_foot_pedal_right(_mqtt_remote, 0x543204016bfc, "Right", [](uint8_t click) {
  switch (click) {
  case CLICK_SINGLE:
    // TODO: Handle click
    break;

  case CLICK_LONG:
    // TODO: Handle long click
    break;
  }
});

// List all devices.
std::vector<std::reference_wrapper<Device>> _devices{_device_foot_pedal_left, _device_foot_pedal_right};

// Create Device Manager and Firmware Checker and register devices.
DeviceManager _device_manager(_devices, []() { return _mqtt_remote.connected(); });
FirmwareChecker _firmware_checker(firmware_update_base_url, _devices, {.check_every_ms = 30000});
FirmwareKicker _firmware_kicker(_firmware_checker, firmware_kicker_port);

// Setup host driver.
HostDriver _host_driver(_device_manager,
                        {
                            .wifi_ssid = wifi_ssid,
                            .wifi_password = wifi_password,
                            .esp_now_encryption_key = esp_now_encryption_key,
                            .esp_now_encryption_secret = esp_now_encryption_secret,
                        },
                        [](const std::string message, const std::string sub_path, const bool retain) {
                          _mqtt_remote.publishMessage(_mqtt_remote.clientId() + sub_path, message, retain);
                        });

void setup() {
  Serial.begin(115200);

  // Add logging callbacks when using Arduino framework. When using ESP-IDF, use set_log_level() instead. See
  // constructor and addOnLog().
  _ota_helper.addOnLog([](const std::string message, const esp_log_level_t log_level) {
    Serial.println("OtaHelper: " + String(message.c_str())); // ignoring log_level, logs everything. Noisy.
  });
  _wifi_helper.addOnLog([](const std::string message, const esp_log_level_t log_level) {
    Serial.println("WifiHelper: " + String(message.c_str())); // ignoring log_level, logs everything. Noisy.
  });

  esp_wifi_set_ps(WIFI_PS_NONE); // No sleep on WiFi to be able to receive ESP-NOW packages without being in AP mode.

  bool initialize_nvs = true;
  bool timeout_ms = 10000;
  auto connected = _wifi_helper.connectToAp(wifi_ssid, wifi_password, initialize_nvs, timeout_ms);
  if (connected) {
    _ota_helper.start();
  }

  // Start host driver with Firmware Checker and Firmware Kicker (both optional)
  _host_driver.setup(_firmware_checker, _firmware_kicker);

  // Start firmware kicker
  _firmware_kicker.start();
}

void loop() {
  _device_manager.handle();
  _firmware_checker.handle();
}
