/*
   Program: Smart House SMS Air Quality Monitor
   Version: 3.0
   Creators: Figga
   Description: A DIY air quality monitor that monitors gas level, 
   humidity, and temperature, with SMS reporting capabilities.
*/

#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <DHT.h>
#include "arduino_secrets.h"

// Twilio Configuration
const char* twilioHost = "api.twilio.com";
const int twilioPort = 443;
WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, twilioHost, twilioPort);

// Pin Definitions
#define DHTPIN 2         // Pin for the DHT11 sensor
#define DHTTYPE DHT11    // DHT11 sensor
#define MQ135PIN A0      // Pin for the MQ135 air quality sensor

// OLED Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Initialize DHT11
DHT dht(DHTPIN, DHTTYPE);

// Base64 encoding table
const char b64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Function to determine air quality
String getAirQualityDescription(int value) {
  if (value < 151) {
    return "GOOD";
  } else if (value < 200) {
    return "OKAY";
  } else if (value < 300) {
    return "BAD";
  } else {
    return "TOXIC";
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Initialize DHT11 sensor
  dht.begin();

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  // Display startup screen
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  // Center "SMART"
  int16_t x1, y1;
  uint16_t width1, height1;
  display.getTextBounds("SMART", 0, 0, &x1, &y1, &width1, &height1);
  int centerX1 = (SCREEN_WIDTH - width1) / 2;
  display.setCursor(centerX1, 20);
  display.println("SMART");

  // Center "HOUSE"
  int16_t x2, y2;
  uint16_t width2, height2;
  display.getTextBounds("HOUSE", 0, 0, &x2, &y2, &width2, &height2);
  int centerX2 = (SCREEN_WIDTH - width2) / 2;
  display.setCursor(centerX2, 40);
  display.println("HOUSE");

  display.display();
  delay(2000);

  // Clear the screen for sensor readings
  display.clearDisplay();

  // Optional: Send initial SMS
  sendSMS("Smart House Monitor Online!");
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
  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print(airQuality);
  display.setTextSize(1);
  display.print(" - ");
  display.println(mq135Value);

  // Display temperature
  display.setTextSize(1);
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

  // Optional: Send SMS with critical readings
  if (airQuality == "BAD") {
  String smsMessage = "ALERT: The air quality is BAD.\n**It's time to open a window!**";
  sendSMS(smsMessage);
  } else if (airQuality == "TOXIC") {
  String smsMessage = "URGENT: TOXIC air levels detected.\n**You need to open a window now!**";
  sendSMS(smsMessage);
}

  // Delay before next loop
  delay(2000);
}

// Simple base64 encoding function
String base64Encode(String input) {
  String output = "";
  int paddingCount = (3 - (input.length() % 3)) % 3;
  int i = 0;
  
  while (i < input.length()) {
    uint32_t octetA = i < input.length() ? input.charAt(i++) : 0;
    uint32_t octetB = i < input.length() ? input.charAt(i++) : 0;
    uint32_t octetC = i < input.length() ? input.charAt(i++) : 0;

    uint32_t triple = (octetA << 16) + (octetB << 8) + octetC;

    output += b64Chars[(triple >> 18) & 0x3F];
    output += b64Chars[(triple >> 12) & 0x3F];
    output += b64Chars[(triple >> 6) & 0x3F];
    output += b64Chars[triple & 0x3F];
  }

  // Add padding if needed
  while (paddingCount--) {
    output.setCharAt(output.length() - 1 - paddingCount, '=');
  }

  return output;
}

void sendSMS(String message) {
  // Create the authentication header
  String auth = String(TWILIO_ACCOUNT_SID) + ":" + String(TWILIO_AUTH_TOKEN);
  String authHeader = "Basic " + base64Encode(auth);

  // Create the POST data
  String postData = "To=" + urlEncode(TWILIO_TO_NUMBER) +
                   "&From=" + urlEncode(TWILIO_FROM_NUMBER) +
                   "&Body=" + urlEncode(message);

  // Create the request path
  String path = "/2010-04-01/Accounts/" + String(TWILIO_ACCOUNT_SID) + "/Messages.json";

  // Make the request
  client.beginRequest();
  client.post(path);
  client.sendHeader("Authorization", authHeader);
  client.sendHeader("Content-Type", "application/x-www-form-urlencoded");
  client.sendHeader("Content-Length", postData.length());
  client.beginBody();
  client.print(postData);
  client.endRequest();

  // Read the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}

String urlEncode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
  }
  return encodedString;
}