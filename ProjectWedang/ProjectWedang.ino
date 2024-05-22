#include <DHT.h>  
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
String apiKey = "4P5UCEI0BC551FTG";    
 
const char *ssid =  "R";     
const char *pass =  "88888888";
const char* server = "api.thingspeak.com";
 
#define DHTPIN D4

// Initialize the OLED display
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); 

DHT dht(DHTPIN, DHT11);
 
WiFiClient client;
 
void setup() 
{
pinMode(LED_BUILTIN, OUTPUT);  
Serial.begin(9600); 
dht.begin();
  
// Initialize the OLED display
if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
{
Serial.println(F("SSD1306 allocation failed"));
for(;;);
  }
  
 display.display();
 delay(2000);  
 display.clearDisplay(); 
 display.setTextSize(1);  
 display.setTextColor(SSD1306_WHITE);  

 Serial.begin(115200);
 delay(10);
 dht.begin();
 
 Serial.println("Connecting to ");
 Serial.println(ssid);
 
 WiFi.begin(ssid, pass);
 
 while (WiFi.status() != WL_CONNECTED) 
 {
 delay(500);
 Serial.print(".");
   }      
Serial.println("");
Serial.println("WiFi connected");
}
 
void loop() 
{

// Read temperature in Celsius
float temperature = dht.readTemperature();

// Read humidity
float humidity = dht.readHumidity();

// Check if any reads failed and exit early (to try again).
if (isnan(temperature) || isnan(humidity)) {
Serial.println("Failed to read from DHT sensor!");
return;
  }

// Display temperature and humidity on serial monitor
Serial.print("Temperature: ");
Serial.print(temperature);
Serial.print(" °C\t");
Serial.print("Humidity: ");
Serial.print(humidity);
Serial.println("%");

// Display temperature and humidity on OLED
display.clearDisplay(); 
display.setCursor(0,0);
display.println("Temperature:");
display.println(temperature, 1);
display.println("Humidity:");
display.println(humidity, 1);
display.display(); 

digitalWrite(LED_BUILTIN, LOW);
delay(1000);

digitalWrite(LED_BUILTIN, HIGH);
delay(2000);  

  
float h = dht.readHumidity();
float t = dht.readTemperature();
      
if (isnan(h) || isnan(t)) 
{
Serial.println("Failed to read from DHT sensor!");
return;
}
 
if (client.connect(server,80))   
{  
                            
String postStr = apiKey;
postStr +="&field1=";
postStr += String(t);
postStr +="&field2=";
postStr += String(h);
postStr += "\r\n\r\n";
 
client.print("POST /update HTTP/1.1\n");
client.print("Host: api.thingspeak.com\n");
client.print("Connection: close\n");
client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
client.print("Content-Type: application/x-www-form-urlencoded\n");
client.print("Content-Length: ");
client.print(postStr.length());
client.print("\n\n");
client.print(postStr);
 
Serial.print("Temperature: ");
Serial.print(t);
Serial.print(" degrees Celcius, Humidity: ");
Serial.print(h);
Serial.println("%. Send to Thingspeak.");
                        }
client.stop();
 
Serial.println("Waiting...");
  
// thingspeak needs minimum 15 sec delay between updates
delay(1000);
}
