/*
  Mad Hatter Card - portrait card rotation

  Hardware: Adafruit Feather RP2040 ThinkInk (#5727) with the
  4.2-inch 400x300 tri-color SSD1683 e-paper display (#6382).

  Board:   Adafruit Feather RP2040 ThinkInk
  Library: Adafruit EPD (install all dependencies)

  The long edge marked by the user's arrow is the TOP of the card.
*/

#include <Adafruit_ThinkInk.h>
#include <math.h>
#include "mad_hatter_bitmap.h"
#include "queen_of_hearts_bitmap.h"
#include "white_rabbit_bitmap.h"

#ifndef ARDUINO_ADAFRUIT_FEATHER_RP2040_THINKINK
#error "Select Tools > Board > Raspberry Pi RP2040 Boards > Adafruit Feather RP2040 ThinkInk."
#endif

constexpr uint8_t DISPLAY_ROTATION = 0;
constexpr uint32_t HATTER_CARD_INTERVAL_MS = 10UL * 60UL * 1000UL;
constexpr uint32_t OTHER_CARD_INTERVAL_MS = 5UL * 60UL * 1000UL;
constexpr uint8_t CARD_COUNT = 4;

// Set to 0, 1, 2, or 3 to show only that card after upload. Set back to -1
// for the normal five-minute rotation. This avoids unsafe rapid e-paper refreshes.
constexpr int8_t PREVIEW_CARD = -1;

ThinkInk_420_Tricolor_MFGNR display(
  PIN_EPD_DC, PIN_EPD_RESET, PIN_EPD_CS, -1, PIN_EPD_BUSY, &SPI1);

uint8_t currentCard = 0;
uint32_t lastCardChange = 0;

void drawBorder();
void drawHeart(int16_t x, int16_t y, int16_t size, uint16_t color);
void drawClock(int16_t x, int16_t y);
void drawArrow(int16_t x, int16_t y, bool pointsRight, uint16_t color);
void drawWhiteRabbit(int16_t x, int16_t y);
void drawWeddingRings(int16_t x, int16_t y);
void drawThickLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                   uint16_t color, uint8_t thickness);
void drawThickRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                        int16_t radius, uint16_t color, uint8_t thickness);
void drawThickCircle(int16_t x, int16_t y, int16_t radius, uint16_t color,
                     uint8_t thickness);
void drawHatterNumber();
void drawMadHatterRaster();
uint32_t currentCardInterval();
void showCard();
void drawTenSixCard();
void drawWeddingCard();
void drawQueenOfHeartsCard();
void drawQueenOfHeartsRaster();
void drawWhichWayCard();
void drawWhiteRabbitRaster();

void setup() {
  Serial.begin(115200);
  delay(500);

  display.begin(THINKINK_TRICOLOR);
  display.setRotation(DISPLAY_ROTATION); // logical canvas is 300 wide x 400 tall

  if (PREVIEW_CARD >= 0 && PREVIEW_CARD < CARD_COUNT) {
    currentCard = PREVIEW_CARD;
  }
  showCard();
  lastCardChange = millis();
}

void loop() {
  if (PREVIEW_CARD >= 0 && PREVIEW_CARD < CARD_COUNT) {
    return;
  }

  // This is intentionally a slow scene change, not video animation.
  // Tri-color e-paper should not be refreshed more often than every 3 minutes.
  if (millis() - lastCardChange >= currentCardInterval()) {
    currentCard = (currentCard + 1) % CARD_COUNT;
    showCard();
    lastCardChange = millis();
  }
}

uint32_t currentCardInterval() {
  return currentCard == 0 ? HATTER_CARD_INTERVAL_MS : OTHER_CARD_INTERVAL_MS;
}

void showCard() {
  display.clearBuffer();

  if (currentCard == 0) {
    drawMadHatterRaster();
    Serial.println("Showing 10/6 Mad Hatter card");
  } else if (currentCard == 1) {
    drawWeddingCard();
    Serial.println("Showing J + J wedding card");
  } else if (currentCard == 2) {
    drawQueenOfHeartsRaster();
    Serial.println("Showing traditional Queen of Hearts card");
  } else {
    drawWhiteRabbitRaster();
    Serial.println("Showing White Rabbit card");
  }

  display.display(); // Full refresh: expect visible flicker for several seconds.
}

void drawBorder() {
  const int16_t w = display.width();
  const int16_t h = display.height();

  display.drawRoundRect(4, 4, w - 8, h - 8, 10, EPD_BLACK);
  display.drawRoundRect(10, 10, w - 20, h - 20, 7, EPD_BLACK);
  display.drawFastHLine(24, 22, w - 48, EPD_BLACK);
  display.drawFastHLine(24, h - 23, w - 48, EPD_BLACK);

  // Small red suit accents.
  drawHeart(27, 30, 11, EPD_RED);
  drawHeart(w - 27, 30, 11, EPD_RED);
  drawHeart(27, h - 30, 11, EPD_RED);
  drawHeart(w - 27, h - 30, 11, EPD_RED);
}

void drawHeart(int16_t x, int16_t y, int16_t size, uint16_t color) {
  display.fillCircle(x - size / 2, y - size / 3, size / 2, color);
  display.fillCircle(x + size / 2, y - size / 3, size / 2, color);
  display.fillTriangle(x - size, y - size / 6, x + size, y - size / 6, x, y + size, color);
}

void drawClock(int16_t x, int16_t y) {
  display.drawCircle(x, y, 42, EPD_BLACK);
  display.drawCircle(x, y, 38, EPD_BLACK);
  display.drawCircle(x, y, 3, EPD_RED);
  display.drawLine(x, y, x + 22, y - 16, EPD_BLACK); // minute hand
  display.drawLine(x, y, x - 11, y - 25, EPD_RED);   // hour hand
  display.drawCircle(x, y - 51, 8, EPD_BLACK);       // winding crown

  for (uint8_t mark = 0; mark < 12; mark++) {
    const float angle = mark * 0.523599f;
    const int16_t x1 = x + cos(angle) * 31;
    const int16_t y1 = y + sin(angle) * 31;
    const int16_t x2 = x + cos(angle) * 35;
    const int16_t y2 = y + sin(angle) * 35;
    display.drawLine(x1, y1, x2, y2, EPD_BLACK);
  }
}

void drawArrow(int16_t x, int16_t y, bool pointsRight, uint16_t color) {
  display.fillRect(x, y, 102, 28, color);
  display.fillTriangle(x + (pointsRight ? 102 : 0), y - 12,
                       x + (pointsRight ? 102 : 0), y + 40,
                       x + (pointsRight ? 130 : -28), y + 14, color);
  // White inset leaves a clear label area inside the sign.
  display.fillRect(x + 8, y + 5, 82, 18, EPD_WHITE);
}

void drawWhiteRabbit(int16_t x, int16_t y) {
  // x/y are the center of the rabbit's head. The rounded forms stay legible
  // after an e-paper full refresh.
  display.fillRoundRect(x - 18, y - 50, 12, 36, 6, EPD_BLACK);
  display.fillRoundRect(x + 6, y - 50, 12, 36, 6, EPD_BLACK);
  display.fillRoundRect(x - 14, y - 46, 4, 24, 2, EPD_RED);
  display.fillRoundRect(x + 10, y - 46, 4, 24, 2, EPD_RED);

  display.fillCircle(x, y, 22, EPD_BLACK);
  display.fillCircle(x, y, 17, EPD_WHITE);
  display.fillCircle(x - 7, y - 3, 2, EPD_BLACK);
  display.fillCircle(x + 7, y - 3, 2, EPD_BLACK);
  display.fillCircle(x, y + 5, 3, EPD_RED);

  display.fillCircle(x, y + 39, 25, EPD_BLACK);
  display.fillCircle(x, y + 35, 19, EPD_WHITE);
  display.drawLine(x - 16, y + 27, x - 34, y + 17, EPD_BLACK);
  display.drawLine(x + 16, y + 27, x + 34, y + 17, EPD_BLACK);

  // The White Rabbit's hand-held sign.
  display.fillRect(x - 66, y + 1, 58, 27, EPD_RED);
  display.drawRect(x - 66, y + 1, 58, 27, EPD_BLACK);
  display.setTextColor(EPD_WHITE);
  display.setTextSize(1);
  display.setCursor(x - 62, y + 7);
  display.print("I'M LATE!");
}

void drawThickLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                   uint16_t color, uint8_t thickness) {
  const int16_t half = thickness / 2;
  for (int16_t offset = -half; offset <= half; offset++) {
    display.drawLine(x0 + offset, y0, x1 + offset, y1, color);
    display.drawLine(x0, y0 + offset, x1, y1 + offset, color);
  }
}

void drawThickRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                        int16_t radius, uint16_t color, uint8_t thickness) {
  for (uint8_t inset = 0; inset < thickness; inset++) {
    display.drawRoundRect(x + inset, y + inset, w - inset * 2, h - inset * 2,
                          radius, color);
  }
}

void drawThickCircle(int16_t x, int16_t y, int16_t radius, uint16_t color,
                     uint8_t thickness) {
  for (uint8_t inset = 0; inset < thickness; inset++) {
    display.drawCircle(x, y, radius - inset, color);
  }
}

void drawHatterNumber() {
  // Hand-drawn 10/6: fixed vector strokes are more theatrical and reliable
  // than enlarging Arduino's pixel font.
  const int16_t y = 145;
  const uint16_t ink = EPD_RED;
  const uint8_t stroke = 6;

  // 1
  drawThickLine(35, y + 17, 55, y + 4, ink, stroke);
  drawThickLine(55, y + 4, 55, y + 85, ink, stroke);
  drawThickLine(38, y + 85, 72, y + 85, ink, stroke);

  // 0
  drawThickRoundRect(83, y + 2, 48, 87, 18, ink, stroke);

  // /  -- intentionally tall and slanted like a hatmaker's price tag
  drawThickLine(151, y + 88, 180, y, ink, stroke);

  // 6
  drawThickLine(250, y + 6, 220, y + 42, ink, stroke);
  drawThickCircle(239, y + 64, 27, ink, stroke);
}

void drawWeddingRings(int16_t x, int16_t y) {
  // Two linked rings add visual weight to the lower portion of the wedding card.
  display.drawCircle(x - 19, y, 24, EPD_BLACK);
  display.drawCircle(x - 19, y, 22, EPD_BLACK);
  display.drawCircle(x + 19, y, 24, EPD_RED);
  display.drawCircle(x + 19, y, 22, EPD_RED);
}

void drawTenSixCard() {
  drawBorder();

  display.setTextColor(EPD_BLACK);
  display.setTextSize(3);
  display.setCursor(33, 86);
  display.print("IN THIS STYLE");

  drawHatterNumber();
}

void drawMadHatterRaster() {
  display.drawBitmap(0, 0, mad_hatter_black, 300, 400, EPD_BLACK);
  display.drawBitmap(0, 0, mad_hatter_red, 300, 400, EPD_RED);
}

void drawWeddingCard() {
  drawBorder();
  display.setTextColor(EPD_BLACK);
  display.setTextSize(2);
  display.setCursor(78, 72);
  display.print("A WONDERFUL DAY");

  display.setTextColor(EPD_RED);
  display.setTextSize(7);
  display.setCursor(45, 105);
  display.print("J + J");

  drawHeart(150, 184, 19, EPD_RED);

  display.setTextColor(EPD_BLACK);
  display.setTextSize(4);
  display.setCursor(78, 212);
  display.print("OCT 24");

  display.setTextColor(EPD_RED);
  display.setTextSize(3);
  display.setCursor(114, 252);
  display.print("2026");
  display.setCursor(69, 286);
  display.print("CONGRATS!");

  drawWeddingRings(150, 338);
}

void drawQueenOfHeartsCard() {
  drawBorder();

  // Original court-card queen: mirrored, high-contrast, and deliberately
  // simplified for a 300x400 three-color e-paper panel.
  display.setTextColor(EPD_RED);
  display.setTextSize(3);
  display.setCursor(25, 42);
  display.print("Q");
  drawHeart(38, 78, 8, EPD_RED);
  display.setCursor(252, 332);
  display.print("Q");
  drawHeart(264, 316, 8, EPD_RED);

  // Upper queen: crown, hair, face, robe, and a heart-topped scepter.
  display.drawLine(105, 91, 195, 91, EPD_BLACK);
  display.drawLine(108, 91, 116, 58, EPD_BLACK);
  display.drawLine(116, 58, 138, 80, EPD_BLACK);
  display.drawLine(138, 80, 150, 49, EPD_BLACK);
  display.drawLine(150, 49, 162, 80, EPD_BLACK);
  display.drawLine(162, 80, 184, 58, EPD_BLACK);
  display.drawLine(184, 58, 192, 91, EPD_BLACK);
  display.fillCircle(150, 70, 4, EPD_RED);
  display.fillCircle(122, 84, 3, EPD_RED);
  display.fillCircle(178, 84, 3, EPD_RED);
  display.fillCircle(150, 111, 30, EPD_BLACK);
  display.fillCircle(150, 111, 24, EPD_WHITE);
  display.fillCircle(141, 107, 2, EPD_BLACK);
  display.fillCircle(159, 107, 2, EPD_BLACK);
  display.drawLine(142, 123, 158, 123, EPD_RED);
  display.fillTriangle(102, 184, 198, 184, 150, 133, EPD_RED);
  display.drawLine(202, 110, 220, 182, EPD_BLACK);
  drawHeart(202, 106, 6, EPD_RED);

  // Central split makes the two mirrored court-card halves unmistakable.
  display.drawFastHLine(58, 200, 184, EPD_BLACK);

  // Lower queen is the upper design inverted, as on a traditional court card.
  display.fillTriangle(102, 216, 198, 216, 150, 267, EPD_RED);
  display.fillCircle(150, 289, 30, EPD_BLACK);
  display.fillCircle(150, 289, 24, EPD_WHITE);
  display.fillCircle(141, 293, 2, EPD_BLACK);
  display.fillCircle(159, 293, 2, EPD_BLACK);
  display.drawLine(142, 277, 158, 277, EPD_RED);
  display.drawLine(105, 309, 195, 309, EPD_BLACK);
  display.drawLine(108, 309, 116, 342, EPD_BLACK);
  display.drawLine(116, 342, 138, 320, EPD_BLACK);
  display.drawLine(138, 320, 150, 351, EPD_BLACK);
  display.drawLine(150, 351, 162, 320, EPD_BLACK);
  display.drawLine(162, 320, 184, 342, EPD_BLACK);
  display.drawLine(184, 342, 192, 309, EPD_BLACK);
  display.fillCircle(150, 330, 4, EPD_RED);
  display.fillCircle(122, 316, 3, EPD_RED);
  display.fillCircle(178, 316, 3, EPD_RED);
  display.drawLine(98, 290, 80, 218, EPD_BLACK);
  drawHeart(98, 294, 6, EPD_RED);
}

void drawQueenOfHeartsRaster() {
  // The two masks are generated from the supplied traditional card reference:
  // dark blue/black/yellow details become black, and red stays red.
  display.drawBitmap(0, 0, queen_of_hearts_black, 300, 400, EPD_BLACK);
  display.drawBitmap(0, 0, queen_of_hearts_red, 300, 400, EPD_RED);
}

void drawWhiteRabbitRaster() {
  display.drawBitmap(0, 0, white_rabbit_black, 300, 400, EPD_BLACK);
  display.drawBitmap(0, 0, white_rabbit_red, 300, 400, EPD_RED);
}

void drawWhichWayCard() {
  drawBorder();

  display.setTextColor(EPD_BLACK);
  display.setTextSize(4);
  display.setCursor(30, 54);
  display.print("WHICH WAY?");

  // The signpost deliberately points in contradictory Wonderland directions.
  display.drawFastVLine(150, 133, 195, EPD_BLACK);
  display.fillCircle(150, 132, 7, EPD_RED);

  drawArrow(35, 145, true, EPD_BLACK);
  display.setTextColor(EPD_BLACK);
  display.setTextSize(1);
  display.setCursor(49, 152);
  display.print("TEA PARTY");

  drawArrow(163, 196, false, EPD_RED);
  display.setTextColor(EPD_RED);
  display.setTextSize(1);
  display.setCursor(179, 203);
  display.print("WEDDING");

  drawArrow(35, 247, true, EPD_BLACK);
  display.setTextColor(EPD_BLACK);
  display.setTextSize(1);
  display.setCursor(51, 254);
  display.print("THIS WAY");

  drawWhiteRabbit(237, 292);

  display.setTextColor(EPD_RED);
  display.setTextSize(1);
  display.setCursor(93, 370);
  display.print("WE'RE ALL MAD HERE");
}
