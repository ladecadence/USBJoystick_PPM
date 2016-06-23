#ifndef __HIDJOYSTICKRPTPARSER_H__
#define __HIDJOYSTICKRPTPARSER_H__

#include <hid.h>
#include "ppm.h"

struct PPMData {
    uint16_t pitch;
    uint16_t roll;
    uint16_t yaw;
    uint16_t throttle;
    uint16_t aux1;
    uint16_t aux2;
    uint16_t aux3;
    uint16_t aux4;
};

struct GamePadEventData {
  uint8_t A, B, C, D, E, F, G;
};

class JoystickEvents {
  public:
    virtual void OnGamePadChanged(const GamePadEventData *evt, PPMData *data);
    virtual void OnHatSwitch(uint8_t hat, PPMData *data);
    virtual void OnButtonUp(uint8_t but_id, PPMData *data);
    virtual void OnButtonDn(uint8_t but_id, PPMData *data);
};

#define RPT_GAMEPAD_LEN		7

class JoystickReportParser : public HIDReportParser {
    JoystickEvents *joyEvents;

    uint8_t oldPad[RPT_GAMEPAD_LEN];
    uint8_t oldHat;
    uint16_t oldButtons;

  public:
    JoystickReportParser(JoystickEvents *evt);
    PPMData ppm_data;

    virtual void Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);
};

#endif // __HIDJOYSTICKRPTPARSER_H__
