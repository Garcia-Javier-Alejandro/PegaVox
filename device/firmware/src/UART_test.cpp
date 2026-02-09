#include <Arduino.h>

// Serial terminal flow:
// - On boot: "Begin tests? Y/N:" (Y starts, N stops).
// - Per test: prints baud/invert + expected scope polarity/bit width, then sends probes.
// - After probes: "Continue (Y), Restart (R), or Success (S)?"
//   - R reruns same setting.
//   - S prints SUCCESS and asks "Continue further configs? Y/N:".
//   - Y continues to next config, N stops and idles.
// Note: you must open the IDE's Serial Monitor at 115200 to see these messages.

// Choose pins on your ESP32-S3 board for UART1.
// Wiring: ESP32 TX -> printer RX, ESP32 RX <- printer TX, and GND -> GND.
// Logic levels: ESP32 is 3.3V TTL. If the printer is 5V TTL or true RS-232,
// use a level shifter or RS-232 transceiver to avoid damage.
static const int PIN_UART_TX = 17; // ESP32 TX -> Printer RX
static const int PIN_UART_RX = 18; // ESP32 RX <- Printer TX (optional but useful)
// Set to false when printer TX is likely not connected.
static const bool EXPECT_PRINTER_TX = false;

// Common baud rates to try during auto-probing.
static const uint32_t BAUDS[] = {115200, 57600, 38400, 19200, 9600};

// ESC/POS-ish probes (many thermal printers react to these).
// ESC = Escape (0x1B) command prefix in ESC/POS.
// DLE = Data Link Escape (0x10) for realtime commands.
// EOT = End Of Transmission (0x04) for status requests.
// LF  = Line Feed (0x0A) to advance paper.
static const uint8_t PROBE1[] = {0x1B, 0x40};                 // ESC @ = Initialize printer
static const uint8_t PROBE2[] = {0x10, 0x04, 0x01};           // DLE EOT 1 = Realtime status query
static const uint8_t PROBE3[] = {'A','T','\r','\n'};          // AT = Hayes AT attention (modem-style modules)

// UART instance used for the printer link.
HardwareSerial &P = Serial1;

enum TestAction {
  ACTION_CONTINUE,
  ACTION_RESTART,
  ACTION_SUCCESS
};

bool waitForYesNo(const char *prompt) {
  // Block until user types Y or N in Serial Monitor.
  Serial.print(prompt);
  while (true) {
    if (Serial.available()) {
      int ch = Serial.read();
      if (ch == 'Y' || ch == 'y') {
        Serial.println("Y");
        return true;
      }
      if (ch == 'N' || ch == 'n') {
        Serial.println("N");
        return false;
      }
    }
    delay(10);
  }
}

TestAction waitForAction() {
  // Block until user types Y, R, or S in Serial Monitor.
  Serial.print("Continue (Y), Restart (R), or Success (S)? ");
  while (true) {
    if (Serial.available()) {
      int ch = Serial.read();
      if (ch == 'Y' || ch == 'y') {
        Serial.println("Y");
        return ACTION_CONTINUE;
      }
      if (ch == 'R' || ch == 'r') {
        Serial.println("R");
        return ACTION_RESTART;
      }
      if (ch == 'S' || ch == 's') {
        Serial.println("S");
        return ACTION_SUCCESS;
      }
    }
    delay(10);
  }
}

void sendProbe(const uint8_t *data, size_t len) {
  // Push a probe pattern out the UART and block until sent.
  P.write(data, len);
  P.flush();
  Serial.printf("Sent %u bytes\n", (unsigned)len);
}

void printScopeExpectations(uint32_t baud, bool invert) {
  // Explain what to expect on the scope for this UART configuration.
  float bitUs = 1000000.0f / (float)baud;
  Serial.printf("Scope: idle=%s, start bit=%s, bit width=%.2f us\n",
                invert ? "LOW" : "HIGH",
                invert ? "HIGH" : "LOW",
                bitUs);
  if (EXPECT_PRINTER_TX) {
    Serial.println("Scope: reply bytes may appear on ESP32 RX (from printer TX) if wiring is correct.");
  }
  Serial.println("Reaction: if baud/polarity is correct, printer may reset or feed; otherwise no reaction.");
}

void logSuccess(uint32_t baud, bool invert) {
  float bitUs = 1000000.0f / (float)baud;
  Serial.printf("SUCCESS: baud=%lu, invert=%s, bit width=%.2f us, idle=%s, start=%s\n",
                (unsigned long)baud,
                invert ? "true" : "false",
                bitUs,
                invert ? "LOW" : "HIGH",
                invert ? "HIGH" : "LOW");
}

void drainReplies(uint32_t ms) {
  // Listen for any reply bytes for a fixed window and print as hex.
  if (!EXPECT_PRINTER_TX) {
    Serial.println("Printer TX -> ESP32 RX not expected; skipping reply window.");
    return;
  }
  Serial.printf("Listening for %lu ms...\n", (unsigned long)ms);
  bool gotData = false;
  uint32_t t0 = millis();
  while (millis() - t0 < ms) {
    while (P.available()) {
      int b = P.read();
      Serial.printf("%02X ", (uint8_t)b);
      gotData = true;
    }
    delay(2);
  }
  if (!gotData) {
    Serial.println("No data received on ESP32 RX from printer TX during this window.");
  } else {
    Serial.println();
  }
}

TestAction tryOneBaud(uint32_t baud, bool invert) {
  // Configure the UART for a given baud/inversion pair and probe it.
  while (true) {
    Serial.printf("\n--- Trying baud=%lu invert=%s ---\n", (unsigned long)baud, invert ? "true" : "false");
    Serial.printf("Configuring UART1 on RX=%d TX=%d\n", PIN_UART_RX, PIN_UART_TX);
    printScopeExpectations(baud, invert);

    // Serial.begin signature on ESP32 Arduino supports "invert" as last parameter.
    // If your core doesn't, remove the invert argument and handle inversion externally. (sacar "invert" si se queja cuando compila)
    P.begin(baud, SERIAL_8N1, PIN_UART_RX, PIN_UART_TX, invert);

    delay(200); // Let the peripheral settle after reconfiguring UART.
    Serial.println("UART configured. Starting probes...");

    // Send a few different probe patterns and read any response.
    sendProbe(PROBE1, sizeof(PROBE1));
    delay(100);
    drainReplies(300);

    sendProbe(PROBE2, sizeof(PROBE2));
    delay(100);
    drainReplies(300);

    sendProbe(PROBE3, sizeof(PROBE3));
    delay(100);
    drainReplies(300);

    // Also try a linefeed (some firmwares feed paper on LF). (linefeed es salto de línea, es decir, avanzar papel)
    P.write('\n');
    delay(100);
    drainReplies(300);

    Serial.println("Finished probes for this setting.");
    P.end(); // Release UART so we can reconfigure for the next attempt.

    TestAction action = waitForAction();
    if (action != ACTION_RESTART) {
      return action;
    }
    Serial.println("Restarting this same test setting...");
  }
}

void setup() {
  // Debug console for seeing probe results.
  Serial.begin(115200);
  delay(300);
  Serial.println("UART probe starting. Tie GNDs together. TX->RX at least.");
  if (!EXPECT_PRINTER_TX) {
    Serial.println("Printer TX -> ESP32 RX not expected; reply windows will be skipped.");
  }
  Serial.println("Will try each baud rate with normal and inverted polarity.");
  if (!waitForYesNo("Begin tests? Y/N: ")) {
    Serial.println("Tests not started.");
    while (true) {
      delay(1000);
    }
  }

  // Try normal polarity first, then inverted polarity.
  for (uint32_t baud : BAUDS) {
    TestAction action = tryOneBaud(baud, false);
    if (action == ACTION_SUCCESS) {
      logSuccess(baud, false);
      if (!waitForYesNo("Continue further configs? Y/N: ")) {
        Serial.println("Stopping probe sequence.");
        while (true) {
          delay(1000);
        }
      }
    }
  }
  for (uint32_t baud : BAUDS) {
    TestAction action = tryOneBaud(baud, true);
    if (action == ACTION_SUCCESS) {
      logSuccess(baud, true);
      if (!waitForYesNo("Continue further configs? Y/N: ")) {
        Serial.println("Stopping probe sequence.");
        while (true) {
          delay(1000);
        }
      }
    }
  }

  Serial.println("Done probing. If you got hex bytes back on any setting, report which baud/invert.");
}

void loop() {
  // Nothing to do; probing happens once in setup().
  delay(1000);
}
