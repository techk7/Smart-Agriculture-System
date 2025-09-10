#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Modbus RTU requests for reading NPK values
const byte nitro[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[] = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[] = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

// A variable used to store NPK values
byte values[8];

SoftwareSerial mod(2, 3);

void setup() {
  // Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Set the baud rate for the Serial port
  Serial.begin(4800);

  // Set the baud rate for the SoftwareSerial object
  mod.begin(4800);
  delay(500);
}

void loop() {
  // Read values
  byte val1, val2, val3;
  val1 = readNutrient(nitro);
  delay(250);
  val2 = readNutrient(phos);
  delay(250);
  val3 = readNutrient(pota);
  delay(250);

  // Display values on the Serial Monitor
  Serial.print("Nitrogen: ");
  Serial.print(val1);
  Serial.println(" mg/kg");
  Serial.print("Phosphorous: ");
  Serial.print(val2);
  Serial.println(" mg/kg");
  Serial.print("Potassium: ");
  Serial.print(val3);
  Serial.println(" mg/kg");

// Display values on the OLED screen
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1, 5);
  display.print("N:");
  display.print(val1);
  display.print("mg/kg");
  display.setCursor(1, 25);
  display.print("P:");
  display.print(val2);
  display.print("mg/kg");
  display.setCursor(1, 45);
  display.print("K:");
  display.print(val3);
  display.print("mg/kg");
  display.display();

  delay(2000);
}

byte readNutrient(const byte *request) {
  // Clear the input buffer
  while (mod.available()) {
    mod.read();
  }

  // Send the request
  mod.write(request, 8);
  delay(300); // Increase the delay for the device to respond

  // Read the response
  int bytesRead = 0;
  unsigned long startTime = millis();
  while (millis() - startTime < 1000) { // Wait up to 1 second for a response
    if (mod.available()) {
      values[bytesRead] = mod.read();
      bytesRead++;
      if (bytesRead >= 7) break; // Stop once we have read 7 bytes
    }
  }

  // Check for a valid response
  if (bytesRead == 7 && values[0] == 0x01 && values[1] == 0x03 && values[2] == 0x02) {
    // Assuming values[3] and values[4] contain the result (high and low byte)
    return values[4]; // Returning only the low byte
  } else {
    Serial.println("Invalid response");
    return 0xFF; // Return 255 if response is invalid
  }
}
