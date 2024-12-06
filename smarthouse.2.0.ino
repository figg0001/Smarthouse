/*

	Program name: Smart House - Air quality monitor
    Version:2.0
    Creator: Figga
    Description: A DIY air quality monitor, that monitors the gas level, humidity and 
    temperature of your space. The OLED display show the actual values. 

*/

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <DHT.h>

// Pin Definitions
#define DHTPIN 2         // Pin for the DHT11 sensor
#define DHTTYPE DHT11    // DHT11 sensor

#define MQ135PIN A0      // Pin for the MQ135 air quality sensor

// OLED Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);  // Use default I2C address

// Initialize DHT11
DHT dht(DHTPIN, DHTTYPE);

// Function to determine air quality
String getAirQualityDescription(int value) {
  if (value < 200) {
    return "GOOD";
  } else if (value < 400) {
    return "OKAY";
  } else if (value < 600) {
    return "BAD";
  } else {
    return "TOXIC";
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Smart House Sensor Monitor Starting...");

  // Initialize DHT11 sensor
  dht.begin();

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Use I2C address 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);  // Infinite loop if OLED initialization fails
  }

  // Display "SMART HOUSE" startup screen
  display.clearDisplay();
  display.setTextSize(2);  // Larger text size for the startup message
  display.setTextColor(SSD1306_WHITE);

  // Center "SMART"
  int16_t x1, y1;
  uint16_t width1, height1;
  display.getTextBounds("SMART", 0, 0, &x1, &y1, &width1, &height1);
  int centerX1 = (SCREEN_WIDTH - width1) / 2;
  display.setCursor(centerX1, 20);  // Position it centered
  display.println("SMART");

  // Center "HOUSE"
  int16_t x2, y2;
  uint16_t width2, height2;
  display.getTextBounds("HOUSE", 0, 0, &x2, &y2, &width2, &height2);
  int centerX2 = (SCREEN_WIDTH - width2) / 2;
  display.setCursor(centerX2, 40);  // Position it centered
  display.println("HOUSE");

  display.display();
  delay(2000);  // Show for 2 seconds

  // Clear the screen for the next readings
  display.clearDisplay();
}

void loop() {
  // Read DHT11 data
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read MQ135 air quality sensor
  int mq135Value = analogRead(MQ135PIN);
  String airQuality = getAirQualityDescription(mq135Value);

  // Print sensor readings to Serial Monitor
  Serial.println("--- Sensor Readings ---");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  
  Serial.print("Air Quality: ");
  Serial.print(airQuality);
  Serial.print(" (Raw Value: ");
  Serial.print(mq135Value);
  Serial.println(")");
  Serial.println("---------------------");

  // Clear the display
  display.clearDisplay();

  // Display "AIR QUALITY" title
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("AIR QUALITY:");

  // Display air quality description first, then value
  display.setTextSize(2);  // Larger text
  display.setCursor(0, 16);  // Position within the yellow section (for dual-color OLED)
  display.print(airQuality);
  display.setTextSize(1);
  display.print(" - ");
  display.println(mq135Value);

  // Display temperature
  display.setTextSize(1);  // Back to normal text size
  display.setCursor(0, 40);
  display.print("Temp: ");
  display.print(temperature);
  display.println(" C");

  // Display humidity
  display.setCursor(0, 50);
  display.print("Humidity: ");
  display.print(humidity);
  display.println(" %");

  // Update display
  display.display();

  // Delay before the next loop
  delay(2000);  // Update every 2 seconds
}

