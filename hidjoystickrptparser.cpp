#include "hidjoystickrptparser.h"


JoystickReportParser::JoystickReportParser(JoystickEvents *evt) :
  joyEvents(evt),
  oldHat(0xDE),
  oldButtons(0) {
  for (uint8_t i = 0; i < RPT_GAMEPAD_LEN; i++)
    oldPad[i] = 0xD;
  ppm_data.aux1=PPM_LOW;
  ppm_data.aux2=PPM_LOW;
  ppm_data.aux3=PPM_LOW;
  ppm_data.aux4=PPM_LOW;
  
}

void JoystickReportParser::Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
  bool match = true;

  // Checking if there are changes in report since the method was last called
  for (uint8_t i = 0; i < RPT_GAMEPAD_LEN; i++)
    if (buf[i] != oldPad[i]) {
      match = false;
      break;
    }

  // Calling Game Pad event handler
  if (!match && joyEvents) {
    joyEvents->OnGamePadChanged((const GamePadEventData*)buf, &ppm_data);

    for (uint8_t i = 0; i < RPT_GAMEPAD_LEN; i++) oldPad[i] = buf[i];
  }

  // locate hat, in this joystick hat is located in field C
  uint8_t hat = (buf[2] & 0x0F);

  // Calling Hat Switch event handler
  if (hat != oldHat && joyEvents) {
    joyEvents->OnHatSwitch(hat, &ppm_data);
    oldHat = hat;
  }

  // Locate buttons, in this joystick buttons are located in field A and low part of field B
  uint16_t buttons = buf[0];

  buttons |= ((buf[1] & 0x0F) << 8);
  uint16_t changes = (buttons ^ oldButtons);

  // Calling Button Event Handler for every button changed
  if (changes) {
    for (uint8_t i = 0; i < 0x0C; i++) {
      uint16_t mask = (0x0001 << i);

      if (((mask & changes) > 0) && joyEvents)
        if ((buttons & mask) > 0)
          joyEvents->OnButtonDn(i + 1, &ppm_data);
        else
          joyEvents->OnButtonUp(i + 1, &ppm_data);
    }
    oldButtons = buttons;
  }
}

void JoystickEvents::OnGamePadChanged(const GamePadEventData *evt, PPMData *data) {
#ifdef DEBUG_USB
  Serial.print("A: ");
  PrintHex<uint8_t > (evt->A, 0x80);
  Serial.print("\tB: ");
  PrintHex<uint8_t > (evt->B, 0x80);
  Serial.print("\tC: ");
  PrintHex<uint8_t > (evt->C, 0x80);
  Serial.print("\tD: ");
  PrintHex<uint8_t > (evt->D, 0x80);
  Serial.print("\tE: ");
  PrintHex<uint8_t > (evt->E, 0x80);
  Serial.print("\tF: ");
  PrintHex<uint8_t > (evt->F, 0x80);
  Serial.print("\tG: ");
  PrintHex<uint8_t > (evt->G, 0x80);
  Serial.println("");
#endif
  
  // mapping, Axis to events
  // also change range from 0x00-0xFF to PPM_LOW-PPM-HIGH
  
  data->pitch = map(evt->D, 0, 0xFF, PPM_LOW, PPM_HIGH);
  data->roll = map(evt->E, 0, 0xFF, PPM_LOW, PPM_HIGH);
  data->yaw = map(evt->F, 0, 0xFF, PPM_LOW, PPM_HIGH);
  data->throttle = map(evt->G, 0, 0xFF, PPM_LOW, PPM_HIGH);

}

void JoystickEvents::OnHatSwitch(uint8_t hat, PPMData *data) {
#ifdef DEBUG_USB
  Serial.print("Hat Switch: ");
  PrintHex<uint8_t > (hat, 0x80);
  Serial.println("");
#endif

  // if you want to change a channel based on hat value, do it here:

  // move camera in steps
  if (hat == 06) {
    data->aux3 -= 300;
    if (data->aux3 < PPM_LOW)
      data->aux3 = PPM_LOW;
  }

  if (hat == 02) {
    data->aux3 += 300;
    if (data->aux3 > PPM_HIGH)
      data->aux3 = PPM_HIGH;
  }

}

void JoystickEvents::OnButtonUp(uint8_t but_id, PPMData *data) {
#ifdef DEBUG_USB
  Serial.print("Up: ");
  Serial.println(but_id);
#endif
}

void JoystickEvents::OnButtonDn(uint8_t but_id, PPMData *data) {
#ifdef DEBUG_USB
  Serial.print("Dn: ");
  Serial.println(but_id, DEC);
#endif

  // if you want to change a channel based on button value, do it here:

  // flaps, gear, etc
  if (but_id == 5) {
    data->aux1 = PPM_LOW;
  }
  
  if (but_id == 6) {
    data->aux1 = PPM_HIGH;
  }

  // flight modes
  if (but_id == 1) {
    data->aux2 = PPM_LOW; // MANUAL
  }
  if (but_id == 4) {
    data->aux2 = 1550; // STAB
  }
  if (but_id == 3) {
    data->aux2 = 1900; // RTL
  }
  if (but_id == 12) {
    data->aux2 = 2100; // FBW
  }

  // center cam
  if (but_id == 2) {
    data->aux3 = 2100; // MANUAL
  }

}

