#include <LiquidCrystal.h>                                 // Include LCD library for 16x2 display support

// LCD pin connections: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(8, 9, 10, A3, A4, A5);                   // Create LCD object with corresponding pins


#define CS_PIN   4                                          // Chip Select pin
#define DATA_PIN 6                                          // Data line (MOSI emulation)
#define CLK_PIN  7                                          // Clock line for bit shifting


#define BTN_WAVE A0                                         // Button to change waveform type
#define BTN_SEL  A1                                         // Button to select digit for editing
#define BTN_INC  A2                                         // Button to increment selected digit


float OutFreq = 5000.0;                                    // Current output frequency in Hz
uint8_t wavetype = 3;                                      // Default waveform type (Sine)


const float digitValues[] = { 0.1, 1.0, 10.0, 100.0, 1000.0, 10000.0 }; // Increment values per digit
const uint8_t NUM_DIGITS = 6;                              // Total adjustable digits (0.1 Hz → 10 kHz)
uint8_t selectedDigit = 2;                                 // Currently selected digit (index from right)


#define FREQ_MAX 50000.0                                   // Max frequency limit
#define FREQ_MIN 0.0                                       // Min frequency limit

#define DEBOUNCE_MS 40                                     // Button debounce delay (ms)


const char* waveNames[] = { "Sqr  ", "Tri  ", "Saw  ", "Sine " }; // Names for display


struct Button {
  uint8_t pin;                                             // Button input pin
  bool lastReading;                                        // Last raw reading from pin
  bool stableState;                                        // Debounced state
  uint32_t lastDebounce;                                   // Time of last state change (for debouncing)
};


Button btnWave = { BTN_WAVE, HIGH, HIGH, 0 };              // Initialize "Wave" button
Button btnSel  = { BTN_SEL,  HIGH, HIGH, 0 };              // Initialize "Select Digit" button
Button btnInc  = { BTN_INC,  HIGH, HIGH, 0 };              // Initialize "Increment" button



bool checkPress(Button& b) {
  bool reading = digitalRead(b.pin);                       // Read button state

  if (reading != b.lastReading) {                          // Detect change in button state
    b.lastDebounce = millis();                             // Reset debounce timer
    b.lastReading = reading;                               // Update last reading
  }

  if ((millis() - b.lastDebounce) >= DEBOUNCE_MS) {        // If input stable longer than debounce time
    if (reading != b.stableState) {                        // And actual state changed after debounce
      b.stableState = reading;                             // Save new stable state
      if (reading == LOW) return true;                     // Register as "press event" (active LOW)
    }
  }

  return false;                                            // No button press detected
}



void clockBit(uint8_t bit) {
  digitalWrite(DATA_PIN, bit & 1);                         // Write one data bit
  delayMicroseconds(3);
  digitalWrite(CLK_PIN, HIGH);                             // Clock rising edge
  delayMicroseconds(5);
  digitalWrite(CLK_PIN, LOW);                              // Clock falling edge
  delayMicroseconds(5);
}



void spiSend32(uint32_t value) {
  for (int i = 31; i >= 0; i--) {
    clockBit((value >> i) & 1);                            // Send each bit from MSB → LSB
  }
}



void sendData(float freq, uint8_t waveType) {
  uint32_t freq_scaled = (uint32_t)(freq * 10.0);          // Convert frequency into tenths of Hz
  uint32_t value = (freq_scaled << 2) | (waveType & 0x03); // Place waveType in LSBs

  digitalWrite(CS_PIN, LOW);                               // Begin SPI transfer
  delayMicroseconds(10);
  spiSend32(value);                                        // Send 32-bit word
  delayMicroseconds(10);
  digitalWrite(CS_PIN, HIGH);                              // End transfer
  delayMicroseconds(10);
}


void updateLCD() {
  lcd.setCursor(0, 0);                                     // Go to first row
  lcd.print(waveNames[wavetype & 0x03]);                   // Display waveform name
  lcd.print(" ");

  char freqBuf[12];
  dtostrf(OutFreq, 7, 1, freqBuf);                         // Format frequency as "XXXXX.X"
  lcd.print(freqBuf);
  lcd.print(" Hz");                                        // Add "Hz" suffix

  // Clear second row before printing cursor indicator
  const uint8_t digitCol[] = { 11, 9, 8, 7, 6, 5 };        // LCD column for each editable digit
  uint8_t cursorCol = digitCol[selectedDigit];             // Determine cursor location

  lcd.setCursor(0, 1);
  lcd.print("                ");                           // Clear second row
  lcd.setCursor(cursorCol, 1);
  lcd.print("^");                                          // Draw cursor under selected digit
}


void setup() {
  Serial.begin(9600);                                      // Start serial monitor

  lcd.begin(16, 2);                                        // Initialize 16x2 LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Initializing  ");                           // Display splash message

  pinMode(CS_PIN, OUTPUT);                                 // Setup SPI pins
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);

  digitalWrite(CS_PIN, HIGH);                              // Set idle SPI states
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(DATA_PIN, LOW);

  pinMode(BTN_WAVE, INPUT_PULLUP);                         // Enable internal pull-ups (active LOW)
  pinMode(BTN_SEL,  INPUT_PULLUP);
  pinMode(BTN_INC,  INPUT_PULLUP);

  delay(1000);                                             // Wait for peripherals to stabilize
  sendData(OutFreq, wavetype);                             // Send initial settings
  updateLCD();                                             // Display initial data
}



void loop() {
  bool changed = false;                                    // Track whether output must be updated

  if (checkPress(btnWave)) {                               // If wave button pressed
    wavetype = (wavetype + 1) & 0x03;                      // Cycle through 4 waveforms
    changed = true;
  }

  if (checkPress(btnSel)) {                                // If select button pressed
    selectedDigit = (selectedDigit + 1) % NUM_DIGITS;      // Move cursor to next digit
    updateLCD();                                           // Update LCD cursor only
  }

  if (checkPress(btnInc)) {                                // If increment button pressed
    float step = digitValues[selectedDigit];               // Step size for selected digit
    float tenStep = step * 10.0;
    float digitNow = fmod(OutFreq, tenStep);               // Current digit value portion

    float candidate;

    if (selectedDigit == 5) {                              // Special handling for 10,000’s place
      float currentDigitVal = floor(OutFreq / 10000.0);
      float nextDigitVal = fmod(currentDigitVal + 1.0, 6.0);
      candidate = fmod(OutFreq, 10000.0) + (nextDigitVal * 10000.0);
    } else {                                               // For other digits
      if (digitNow + step >= tenStep) {
        candidate = OutFreq - digitNow;                    // Wrap around current digit
      } else {
        candidate = OutFreq + step;                        // Increment normally
      }
    }

    if (candidate < FREQ_MIN) candidate = FREQ_MIN;        // Clamp to valid range
    if (candidate > FREQ_MAX) candidate = FREQ_MAX;

    OutFreq = candidate;                                   // Apply new frequency
    changed = true;
  }

  if (changed) {                                           // If anything changed
    sendData(OutFreq, wavetype);                           // Send settings to external module
    updateLCD();                                           // Refresh display
  }
}
