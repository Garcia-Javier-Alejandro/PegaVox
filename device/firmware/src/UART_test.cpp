#include <Arduino.h>

// Choose pins on your ESP32-S3 board:
static const int PIN_UART_TX = 17; // ESP32 TX -> Printer RX
static const int PIN_UART_RX = 18; // ESP32 RX <- Printer TX (optional but useful)

// Common baud rates to try
static const uint32_t BAUDS[] = {115200, 57600, 38400, 19200, 9600};

// ESC/POS-ish probes (many thermal printers react to these)
// ESC @   : initialize
// DLE EOT : realtime status (varies by model)
// LF      : newline
static const uint8_t PROBE1[] = {0x1B, 0x40};                 // ESC @
static const uint8_t PROBE2[] = {0x10, 0x04, 0x01};           // DLE EOT 1 (printer status, sometimes)
static const uint8_t PROBE3[] = {'A','T','\r','\n'};          // AT (if a module responds)

HardwareSerial &P = Serial1;

void sendProbe(const uint8_t *data, size_t len) {
  P.write(data, len);
  P.flush();
}

void drainReplies(uint32_t ms) {
  uint32_t t0 = millis();
  while (millis() - t0 < ms) {
    while (P.available()) {
      int b = P.read();
      Serial.printf("%02X ", (uint8_t)b);
    }
    delay(2);
  }
  Serial.println();
}

void tryOneBaud(uint32_t baud, bool invert) {
  Serial.printf("\n--- Trying baud=%lu invert=%s ---\n", (unsigned long)baud, invert ? "true" : "false");

  // Serial.begin signature on ESP32 Arduino supports "invert" as last parameter.
  // If your core doesn't, remove the invert argument and handle inversion externally.
  P.begin(baud, SERIAL_8N1, PIN_UART_RX, PIN_UART_TX, invert);

  delay(200);

  // Send a few different probe patterns
  sendProbe(PROBE1, sizeof(PROBE1));
  delay(100);
  drainReplies(300);

  sendProbe(PROBE2, sizeof(PROBE2));
  delay(100);
  drainReplies(300);

  sendProbe(PROBE3, sizeof(PROBE3));
  delay(100);
  drainReplies(300);

  // Also try a linefeed (some firmwares feed paper on LF)
  P.write('\n');
  delay(100);
  drainReplies(300);

  P.end();
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("UART probe starting. Tie GNDs together. TX->RX at least.");

  // Try normal polarity first, then inverted polarity.
  for (uint32_t baud : BAUDS) {
    tryOneBaud(baud, false);
  }
  for (uint32_t baud : BAUDS) {
    tryOneBaud(baud, true);
  }

  Serial.println("Done probing. If you got hex bytes back on any setting, report which baud/invert.");
}

void loop() {
  delay(1000);
}
