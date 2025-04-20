#include "DeviceFootPedal.h"
#include <DeviceUtils.h>

DeviceFootPedal::DeviceFootPedal(IMQTTRemote &mqtt_remote, uint64_t mac_address, std::string name_suffix,
                                 std::function<void(uint8_t)> on_click)
    : _mac_address(mac_address), _name_suffix(name_suffix), _mqtt_remote(mqtt_remote), _on_click(on_click) {}

void DeviceFootPedal::onConnectionStateChanged(bool connected, EspNowHost &esp_now_host) {
  if (connected) {
    auto base_path = _mqtt_remote.clientId() + "/" + type() + "/0x" + DeviceUtils::toHex(macAddress());
    _mqtt_remote.subscribe(base_path + "/message", [&](std::string topic, std::string message) {
      // Set payload to node, will be sent to node on next challenge request by the node.
      std::string tmp = "payload: " + message;
      esp_now_host.setPayload(macAddress(), (uint8_t *)tmp.c_str(), tmp.size());
    });
  }
}

bool DeviceFootPedal::onMessage(const uint8_t retries, const uint8_t version, const uint8_t *message) {
  switch (version) {
  case 1:
    publish((EspNowFootPedalV1 *)message, retries);
    return true;
    break;

  default:
    break;
  }
  return false;
}

void DeviceFootPedal::publish(EspNowFootPedalV1 *message, uint8_t retries) {
  auto base_path = _mqtt_remote.clientId() + "/" + type() + "/0x" + DeviceUtils::toHex(macAddress());
  _mqtt_remote.publishMessage(base_path + "/click", std::to_string(message->click));
  _mqtt_remote.publishMessage(base_path + "/temperature", std::to_string(message->temperature));
  _mqtt_remote.publishMessage(base_path + "/battery_voltage", std::to_string(message->battery_voltage));

  if (_on_click) {
    _on_click(message->click);
  }
}