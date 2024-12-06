#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"

const char* twilioHost = "api.twilio.com";
const int twilioPort = 443;

WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, twilioHost, twilioPort);

// Base64 encoding table
const char b64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Send SMS
  sendSMS("Hello World!");
}

void loop() {
  // Nothing to do here
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