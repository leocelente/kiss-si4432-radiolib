#include "kiss.h"
#include <RadioLib.h>

// nSEL pin:  2
// nIRQ pin:  4
// SDN pin:   5
Si4432 radio = new Module(2, 4, 5);
volatile bool received_flag = false;
kiss_t host_serial_kiss;
uint8_t rx_radio_buffer[64];
uint8_t rx_serial_buffer[128];


#if defined(ESP8266) || defined(ESP32)
IRAM_ATTR
#endif
void set_flag() {
  received_flag = true;
}

kiss_error_t sender(uint8_t b) {
  Serial.write(b);
  return E_OK;
}

void on_msg(kiss_cmd_t type, uint8_t* buffer, size_t len, bool overflow) {
  radio.transmit(buffer, len);
  delay(1);
}


void setup() {
  Serial.begin(115200);
  host_serial_kiss.sender = sender;
  host_serial_kiss.callback = on_msg;
  host_serial_kiss.rx_buffer = rx_serial_buffer;
  host_serial_kiss.rx_buffer_len = sizeof(rx_serial_buffer);
  kiss_init(&host_serial_kiss);

  auto status = radio.begin(/*Freq(MHz):*/ 434.0, /*BitRate(kbps):*/ 9.6, /*FreqDev(kHz):*/ 25.0, /*RxBandWidth(kHz):*/ 50, /*Power(dBm):*/ -1, /*PreambleLen:*/ 8);
  if (status != RADIOLIB_ERR_NONE) {
    // should change to CMD_CONF or other at some point
    kiss_send(&host_serial_kiss, CMD_DATA, (uint8_t*)"No Radio!", 10);
    while (true)
      ;
  }
  memset(rx_serial_buffer, 0, sizeof(rx_serial_buffer));
  memset(rx_radio_buffer, 0, sizeof(rx_radio_buffer));

  radio.setIrqAction(set_flag);
  status = radio.startReceive();
}


void loop() {
  if (Serial.available()) {
    uint8_t b = Serial.read();
    kiss_ingest_byte(&host_serial_kiss, b);
  }

  if (received_flag) {
    received_flag = false;
    auto status = radio.readData(rx_radio_buffer, 0);
    auto len = radio.getPacketLength();
    if (len) {
      kiss_send(&host_serial_kiss, CMD_DATA, rx_radio_buffer, len);
    }
    radio.setIrqAction(set_flag);
    radio.startReceive();
  }
}
