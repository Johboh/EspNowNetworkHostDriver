#ifndef __LOCAL_DEVICE_H__
#define __LOCAL_DEVICE_H__

#include "Device.h"
#include "EspNowHost.h"

class LocalDevice : public Device {
public:
  /**
   * @brief Main application (your application) need to call this when on connection change. What "connection" mean is
   * up to the caller, but usually this indicate that we are now connected to the Internet or to a MQTT server
   * or some other integration. This can be useful if the Device need to post som initial setup data to MQTT or similar.
   */
  virtual void onConnectionStateChanged(bool connected, EspNowHost &esp_now_host) {}
};

#endif // __LOCAL_DEVICE_H__