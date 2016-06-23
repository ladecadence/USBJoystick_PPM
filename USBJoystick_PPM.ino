#include <hid.h>
#include <hiduniversal.h>
#include <usbhub.h>

// Satisfy IDE, which only needs to see the include statment in the ino.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

#include "hidjoystickrptparser.h"
#include "ppm.h"

USB Usb;
USBHub Hub(&Usb);
HIDUniversal Hid(&Usb);
JoystickEvents JoyEvents;
JoystickReportParser Joy(&JoyEvents);

unsigned char channel_number = 1;
long channel_value[13] = {
  PPM_MIDDLE, PPM_MIDDLE, PPM_MIDDLE, PPM_MIDDLE, PPM_MIDDLE, PPM_MIDDLE, PPM_MIDDLE,
  PPM_MIDDLE, PPM_MIDDLE, PPM_MIDDLE, PPM_MIDDLE, PPM_MIDDLE, PPM_MIDDLE
};

void setup() {
  #ifdef DEBUG_USB
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  Serial.println("Start");

  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

#endif

  delay(200);
  
#ifdef DEBUG_USB
  if (!Hid.SetReportParser(0, &Joy))
    ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);
#endif

  // setup ppm generation
  //     output-pin used to generate PPM-signal
  pinMode(9, OUTPUT);

  TCCR1A =
    (0 << WGM10) |
    (0 << WGM11) |
    (0 << COM1A1) |
    (1 << COM1A0) | // Toggle pin om compare-match
    (0 << COM1B1) |
    (0 << COM1B0);

  TCCR1B =
    (0 << ICNC1) | //
    (0 << ICES1) | //
    (0 << CS10) | //Prescale 8
    (1 << CS11) | //Prescale 8
    (0 << CS12) | //Prescale 8
    (1 << WGM13) |
    (1 << WGM12); // CTC mode (Clear timer on compare match) with ICR1 as top.

  TIMSK1 =
    (1 << OCIE1A) | // Interrupt on compare A
    (0 << OCIE1B) | // Disable interrupt on compare B
    (0 << TOIE1);

  OCR1A = DEAD_TIME;

  // Frame length set by counter TOP, we use ICR1 as TOP.
  ICR1 = FRAME_LENGTH;
}

void loop() {
  // Read USB HID
  Usb.Task();

  // update variables AETR (discard channel 0)
  channel_value[1] = Joy.ppm_data.roll;
  channel_value[2] = Joy.ppm_data.pitch;
  channel_value[3] = Joy.ppm_data.throttle;
  channel_value[4] = Joy.ppm_data.yaw;
  
  // buttons or hat channels 
  channel_value[5] = Joy.ppm_data.aux1;
  channel_value[6] = Joy.ppm_data.aux2;
  channel_value[7] = Joy.ppm_data.aux3;
  channel_value[8] = Joy.ppm_data.aux4;

}


// ===========================
// Timer 1 compare A vector
// ===========================

ISR(TIMER1_COMPA_vect) {


  if (OCR1A == 0) {
    TCCR1A =
      (0 << WGM10) |
      (0 << WGM11) |
      (1 << COM1A1) |
      (1 << COM1A0) |
      (0 << COM1B1) |
      (0 << COM1B0);

    channel_number = 1;
    OCR1A = DEAD_TIME;
  }


  else {
    if (channel_number == 1) {

      // After first time, when pin have been set hgih, we toggle the pin at each interrupt
      TCCR1A =
        (0 << WGM10) |
        (0 << WGM11) |
        (0 << COM1A1) |
        (1 << COM1A0) |
        (0 << COM1B1) |
        (0 << COM1B0);

    }


    if ((channel_number - 1) < NUMBER_OF_CHANNELS * 2) {

      if ((channel_number - 1) % 2 == 1) {
        OCR1A += DEAD_TIME;
      }

      else {
        OCR1A += channel_value[(channel_number + 1) / 2];
      }

      channel_number++;
    }

    // Used to set the pulse low just before the new frame.



    else {
      OCR1A = 0; //FRAME_LENGTH-5;

    }

  }


}


// ===========================
// Timer 1 compare B vector
// ===========================
ISR(TIMER1_COMPB_vect) {
}
