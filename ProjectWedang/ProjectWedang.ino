#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

// ThingSpeak API Key untuk channel sensor
String apiKeySensor = "4P5UCEI0BC551FTG";    
const char* server = "api.thingspeak.com";

// WiFi credentials
const char *ssid = "PROJECT IOT";     
const char *pass = "11111111";

// DHT sensor pin
#define DHTPIN D4

// Relay pin
#define RELAY_PIN D2

// Initialize the OLED display
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); 

// Initialize the DHT sensor
DHT dht(DHTPIN, DHT11);

WiFiClient client;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Initialize relay to be off

  Serial.begin(115200); 
  dht.begin();

  // Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.display();
  delay(2000);  
  display.clearDisplay(); 
  display.setTextSize(1);  
  display.setTextColor(SSD1306_WHITE);  

  Serial.println("Connecting to WiFi...");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void loop() {
  float temperature, humidity;

  // Baca data dari sensor DHT
  if (readDHT(temperature, humidity)) {
    // Tampilkan suhu dan kelembaban di serial monitor
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C\t");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");

    // Tampilkan suhu dan kelembaban di OLED
    display.clearDisplay(); 
    display.setCursor(0, 0);
    display.println("Temperature:");
    display.print(temperature, 1);
    display.println(" °C");
    display.println("Humidity:");
    display.print(humidity, 1);
    display.println(" %");
    display.display();

    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);

    // Kirim data ke ThingSpeak (channel sensor)
    sendToThingSpeak(temperature, humidity);
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }

  // Baca data dari ThingSpeak untuk mengontrol relay
  readFromThingSpeak();

  Serial.println("Waiting...");
  // ThingSpeak membutuhkan jeda minimal 15 detik antara pembaruan
  delay(15000);
}

bool readDHT(float &temperature, float &humidity) {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Cek jika pembacaan gagal dan keluar lebih awal (untuk mencoba lagi).
  if (isnan(temperature) || isnan(humidity)) {
    return false;
  }
  return true;
}

void sendToThingSpeak(float temperature, float humidity) {
  if (client.connect(server, 80)) { 
    String postStr = "api_key=" + apiKeySensor;
    postStr += "&field1=" + String(temperature);
    postStr += "&field2=" + String(humidity);

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: " + String(server) + "\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: " + String(postStr.length()) + "\n\n");
    client.print(postStr);

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.println("%. Sent to ThingSpeak.");
    client.stop();
  } else {
    Serial.println("Failed to connect to ThingSpeak");
  }
}

void readFromThingSpeak() {
  // Channel dan API Key untuk relay
  int relayChannelId = 2557568; // Ganti dengan ID channel Anda
  String apiKeyRelay = "1BI69SU9Y0D437VE"; // Ganti dengan API key channel Anda

  if (client.connect(server, 80)) {
    String getStr = "/channels/" + String(relayChannelId) + "/fields/1/last.json?api_key=" + apiKeyRelay;
    
    client.print("GET " + getStr + " HTTP/1.1\r\n" +
                 "Host: " + String(server) + "\r\n" +
                 "Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
      
      // Parse respons JSON untuk mendapatkan nilai field
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, line);
      if (!error) {
        int relayState = doc["field1"].as<int>();
        if (relayState == 1) {
          digitalWrite(RELAY_PIN, HIGH);
        } else {
          digitalWrite(RELAY_PIN, LOW);
        }
      }
    }
    client.stop();
  } else {
    Serial.println("Failed to connect to ThingSpeak");
  }
}
