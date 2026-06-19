#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80);

#define BUZZER_PIN 27

float totalAmount = 0;
int paymentCount  = 0;
float lastAmount  = 0;

void beep(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

float extractAmount(String sms) {
  int rsIdx = sms.indexOf("Rs.");
  if (rsIdx == -1) rsIdx = sms.indexOf("Rs ");
  if (rsIdx == -1) rsIdx = sms.indexOf("INR ");
  if (rsIdx == -1) return 0;

  int start = rsIdx + 3;
  if (sms.charAt(start) == ' ') start++;

  String numStr = "";
  for (int i = start; i < sms.length(); i++) {
    char c = sms.charAt(i);
    if (c >= '0' && c <= '9') {
      numStr += c;
    } else if (c == '.' && numStr.length() > 0) {
      numStr += c;
    } else if (numStr.length() > 0) {
      break;
    }
  }
  return numStr.toFloat();
}

void displayUpdate() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Total:Rs.");
  lcd.print((int)totalAmount);

  lcd.setCursor(0, 1);
  if (paymentCount > 0) {
    lcd.print("Last:Rs.");
    lcd.print((int)lastAmount);
    lcd.print(" #");
    lcd.print(paymentCount);
  } else {
    lcd.print("Waiting...");
  }
}

void handleSMS() {
  if (server.hasArg("message")) {
    String sms = server.arg("message");
    Serial.println("SMS: " + sms);

    float amount = extractAmount(sms);
    if (amount > 0) {
      lastAmount = amount;
      totalAmount += amount;
      paymentCount++;
      displayUpdate();
      beep(3); // 3 beeps!
      Serial.println("Amount: " + String(amount));
    }
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "No message");
  }
}

void handleReset() {
  totalAmount  = 0;
  paymentCount = 0;
  lastAmount   = 0;
  displayUpdate();
  server.send(200, "text/plain", "Reset OK");
}

void handleStatus() {
  String json = "{\"total\":" + String(totalAmount) +
                ",\"count\":" + String(paymentCount) +
                ",\"last\":" + String(lastAmount) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Payment Display");
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");

  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi OK!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    beep(1); // WiFi connect ஆனா 1 beep
    delay(4000);
    displayUpdate();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Check settings");
  }

  server.on("/sms", HTTP_GET, handleSMS);
  server.on("/reset", HTTP_GET, handleReset);
  server.on("/status", HTTP_GET, handleStatus);
  server.begin();
}

void loop() {
  server.handleClient();
}

ithuthan da code
