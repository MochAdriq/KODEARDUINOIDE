#include <WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define BOT_TOKEN "6760055305:AAFqvKYjkkOZyqkT0klgqR042KmEXSmtPk8"
#define SSID "realme 3"
#define PASSWORD "123empat"

const char* ssid = SSID;
const char* password = PASSWORD;

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define TRIG1 14
#define ECHO1 27
#define TRIG2 16
#define ECHO2 17
#define TRIG3 18
#define ECHO3 19
const float distanceThreshold = 10.0;

void sendTelegramMessage(String chat_id, String message) {
  Serial.println("Sending message to chat_id: " + chat_id);
  Serial.println("Message: " + message);

  if (bot.sendMessage(chat_id, message, "")) {
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Failed to send message");
  }
}

void sendTelegramOptions(String chat_id) {
  String options = "Pilih opsi:\n";
  options += "1. Status Slot 1\n";
  options += "2. Status Slot 2\n";
  options += "3. Status Slot 3\n";
  options += "4. Status Semua Slot\n";
  sendTelegramMessage(chat_id, options);
}

void checkDistanceAndSendMessage(String chat_id, int slot) {
  float d1 = 0.01723 * readDistance(TRIG1, ECHO1);
  float d2 = 0.01723 * readDistance(TRIG2, ECHO2);
  float d3 = 0.01723 * readDistance(TRIG3, ECHO3);

  Serial.println("d1 = " + String(d1) + " cm");
  Serial.println("d2 = " + String(d2) + " cm");
  Serial.println("d3 = " + String(d3) + " cm");

  if (slot == 1) {
    sendTelegramMessage(chat_id, "Slot 1: " + String(d1) + " cm");
  } else if (slot == 2) {
    sendTelegramMessage(chat_id, "Slot 2: " + String(d2) + " cm");
  } else if (slot == 3) {
    sendTelegramMessage(chat_id, "Slot 3: " + String(d3) + " cm");
  } else {
    sendTelegramMessage(chat_id, "Slot 1: " + String(d1) + " cm\nSlot 2: " + String(d2) + " cm\nSlot 3: " + String(d3) + " cm");
  }
}

float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;
  if (distance == 0 || distance > 400) { // Assumption: Maximum range of sensor is 400 cm
    Serial.println("Invalid distance reading: " + String(distance));
    return -1; // Return an error value
  }
  return distance;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(23, 22);
  lcd.init();
  lcd.backlight();

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  pinMode(TRIG3, OUTPUT);
  pinMode(ECHO3, INPUT);

  // Koneksi ke WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Mengatur sertifikat untuk Telegram (opsional, tergantung pada versi library)
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  Serial.println("Bot is starting...");
}

void handleNewMessages(int numNewMessages) {
  Serial.println("Checking for new messages...");
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    Serial.println("Received message: " + text);

    if (text == "/start") {
      Serial.println("Received /start command");
      sendTelegramOptions(chat_id);
    } else if (text == "Status Slot 1") {
      Serial.println("Received Status Slot 1 command");
      checkDistanceAndSendMessage(chat_id, 1);
    } else if (text == "Status Slot 2") {
      Serial.println("Received Status Slot 2 command");
      checkDistanceAndSendMessage(chat_id, 2);
    } else if (text == "Status Slot 3") {
      Serial.println("Received Status Slot 3 command");
      checkDistanceAndSendMessage(chat_id, 3);
    } else if (text == "Status Semua Slot") {
      Serial.println("Received Status Semua Slot command");
      checkDistanceAndSendMessage(chat_id, 0);
    } else {
      Serial.println("Received unknown command");
      sendTelegramMessage(chat_id, "Opsi tidak dikenal. Silakan pilih opsi yang tersedia.");
      sendTelegramOptions(chat_id);
    }
  }
}

void loop() {
  static unsigned long lastUpdateTime = 0;
  static unsigned long lastMessageTime = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastUpdateTime >= 10000) { // Check for new messages every 10 seconds
    Serial.println("Getting updates...");
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    Serial.println("Number of new messages: " + String(numNewMessages));
    
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      Serial.println("Number of new messages: " + String(numNewMessages));
    }
    lastUpdateTime = currentTime;
  }

  if (currentTime - lastMessageTime >= 60000) { // Send messages every 60 seconds
    float d1 = 0.01723 * readDistance(TRIG1, ECHO1);
    float d2 = 0.01723 * readDistance(TRIG2, ECHO2);
    float d3 = 0.01723 * readDistance(TRIG3, ECHO3);

    if (d1 == -1 || d2 == -1 || d3 == -1) {
      Serial.println("One or more distance readings were invalid. Skipping this loop iteration.");
      delay(1000);
      return;
    }

    Serial.println("d1 = " + String(d1) + " cm");
    Serial.println("d2 = " + String(d2) + " cm");
    Serial.println("d3 = " + String(d3) + " cm");

    lcd.clear();
    lcd.setCursor(0, 0);

    if (d1 > distanceThreshold && d2 > distanceThreshold && d3 > distanceThreshold) {
      Serial.println("3 Slots Free");
      Serial.println("Slot 1, 2, and 3 are Free");
      lcd.print("3 Slots Free");
      lcd.setCursor(0, 1);
      lcd.print("Slot 1, 2, 3 Free");
      sendTelegramMessage("1450805917", "3 Slots Free");
    }
    else if ((d1 > distanceThreshold && d2 > distanceThreshold) || 
             (d2 > distanceThreshold && d3 > distanceThreshold) || 
             (d3 > distanceThreshold && d1 > distanceThreshold)) {
      Serial.println("2 Slots Free");
      if (d1 > distanceThreshold && d2 > distanceThreshold) {
        Serial.println("Slot 1 and 2 are Free");
        lcd.print("2 Slots Free");
        lcd.setCursor(0, 1);
        lcd.print("Slot 1, 2 Free");
        sendTelegramMessage("1450805917", "2 Slots Free: Slot 1, 2 Free");
      }
      else if (d1 > distanceThreshold && d3 > distanceThreshold) {
        Serial.println("Slot 1 and 3 are Free");
        lcd.print("2 Slots Free");
        lcd.setCursor(0, 1);
        lcd.print("Slot 1, 3 Free");
        sendTelegramMessage("1450805917", "2 Slots Free: Slot 1, 3 Free");
      }
      else {
        Serial.println("Slot 2 and 3 are Free");
        lcd.print("2 Slots Free");
        lcd.setCursor(0, 1);
        lcd.print("Slot 2, 3 Free");
        sendTelegramMessage("1450805917", "2 Slots Free: Slot 2, 3 Free");
      }
    }
    else if (d1 < distanceThreshold && d2 < distanceThreshold && d3 < distanceThreshold) {
      Serial.println("No Slots Free");
      Serial.println("Parking Full");
      lcd.print("No Slots Free");
      lcd.setCursor(0, 1);
      lcd.print("Parking Full");
      sendTelegramMessage("1450805917", "No Slots Free: Parking Full");
    }
    else {
      Serial.println("1 Slot Free");
      if (d1 > distanceThreshold) {
        Serial.println("Slot 1 is Free");
        lcd.print("1 Slot Free");
        lcd.setCursor(0, 1);
        lcd.print("Slot 1 Free");
        sendTelegramMessage("1450805917", "1 Slot Free: Slot 1 Free");
      }
      else if (d2 > distanceThreshold) {
        Serial.println("Slot 2 is Free");
        lcd.print("1 Slot Free");
        lcd.setCursor(0, 1);
        lcd.print("Slot 2 Free");
        sendTelegramMessage("1450805917", "1 Slot Free: Slot 2 Free");
      }
      else {
        Serial.println("Slot 3 is Free");
        lcd.print("1 Slot Free");
        lcd.setCursor(0, 1);
        lcd.print("Slot 3 Free");
        sendTelegramMessage("1450805917", "1 Slot Free: Slot 3 Free");
      }
    }

    lastMessageTime = currentTime;
  }

  delay(1000); // Add a small delay to prevent CPU overload
}
