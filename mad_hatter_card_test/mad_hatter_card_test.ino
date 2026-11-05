/*
  Mad Hatter Card - hardware test

  Hardware:
    - Adafruit Feather RP2040 ThinkInk (#5727)
    - Adafruit 4.2-inch 300x400 tri-color EPD, SSD1683 (#6382)

  Arduino IDE board: Adafruit Feather RP2040 ThinkInk
  Library: Adafruit EPD (install all prompted dependencies)
*/

#include <Adafruit_ThinkInk.h>

#ifndef ARDUINO_ADAFRUIT_FEATHER_RP2040_THINKINK
#error "Select Tools > Board > Raspberry Pi RP2040 Boards > Adafruit Feather RP2040 ThinkInk."
#endif

// These pins and SPI1 are dedicated to the ThinkInk board's 24-pin EPD socket.
// SRAM_CS is -1 because this board uses its onboard RAM.
ThinkInk_420_Tricolor_MFGNR display(
  PIN_EPD_DC, PIN_EPD_RESET, PIN_EPD_CS, -1, PIN_EPD_BUSY, &SPI1);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting SSD1683 4.2-inch tri-color EPD test");

  display.begin(THINKINK_TRICOLOR);

  // Put the panel in a known, all-white state before drawing the pattern.
  display.clearBuffer();
  display.display();
  delay(2000);

  drawTestPattern();
  display.display();  // A full e-paper refresh takes several seconds.
  Serial.println("Test pattern displayed.");
}

void loop() {
  // E-paper retains this image; repeatedly refreshing would waste battery power.
}

void drawTestPattern() {
  display.clearBuffer();

  const int16_t w = display.width();
  const int16_t h = display.height();

  display.drawRect(0, 0, w, h, EPD_BLACK);
  display.fillRect(16, 16, (w - 48) / 2, h - 32, EPD_BLACK);
  display.fillRect(w / 2 + 8, 16, (w - 48) / 2, h - 32, EPD_RED);

  // White cross-bars make it easy to confirm all three panel colors.
  display.fillRect(16, h / 2 - 6, w - 32, 12, EPD_WHITE);
  display.fillRect(w / 2 - 6, 16, 12, h - 32, EPD_WHITE);

  display.setTextColor(EPD_BLACK);
  display.setTextSize(2);
  display.setCursor(24, 24);
  display.print("BLACK");

  display.setTextColor(EPD_RED);
  display.setTextSize(2);
  display.setCursor(w / 2 + 20, h - 48);
  display.print("RED");
}
