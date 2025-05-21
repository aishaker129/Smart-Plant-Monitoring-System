
// Bkynk configuration
#define BLYNK_TEMPLATE_ID "TMPL6ih-d4FvD"
#define BLYNK_TEMPLATE_NAME "Smart Plant Monitoring System"
#define BLYNK_PRINT Serial

// Included Library
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// LCD Address
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi + Blynk
char auth[] = "UgTP_WdgXzcVfy8xZqd8odhd916mftXz";
char ssid[] = "Brother ";
char pass[] = "Speed@12345";

BlynkTimer timer;

// Pins
#define SOIL_SENSOR A0
#define WATER_PUMP D3
#define DHTPIN D4         // GPIO2
#define PIRPIN D5         // GPIO14

// DHT Sensor Setup
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// State Flags
bool Relay = false;
bool motionDetected = false;
int soilValue = 0;
float temp = 0;
float hum = 0;

// Setup
void setup() {
  Serial.begin(9600);

  pinMode(WATER_PUMP, OUTPUT);
  digitalWrite(WATER_PUMP, HIGH); // Pump OFF by default

  pinMode(PIRPIN, INPUT);

  lcd.init();
  lcd.backlight();

  dht.begin();
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  // Welcome message
  lcd.setCursor(1, 0);
  lcd.print("System Loading");
  for (int a = 0; a <= 15; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(100);
  }
  lcd.clear();

  // Timers
  timer.setInterval(2000L, readSoilMoisture);
  timer.setInterval(3000L, readDHT);
  timer.setInterval(1000L, checkMotion);
  timer.setInterval(2000L, updateLCD); // Show all sensor data on LCD
}

// Pump manual control from Blynk
BLYNK_WRITE(V1) {
  Relay = param.asInt();
  if (Relay == 1) {
    digitalWrite(WATER_PUMP, LOW);
  } else {
    digitalWrite(WATER_PUMP, HIGH);
  }
}

// Read soil sensor
void readSoilMoisture() {
  int value = analogRead(SOIL_SENSOR);
  value = map(value, 0, 1024, 0, 100);
  soilValue = (value - 100) * -1;
  Blynk.virtualWrite(V0, soilValue);
}

// Read DHT11
void readDHT() {
  hum = dht.readHumidity();
  temp = dht.readTemperature();

  if (isnan(hum) || isnan(temp)) {
    Serial.println("DHT read failed");
    return;
  }

  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
}

// Check PIR motion
void checkMotion() {
  int motion = digitalRead(PIRPIN);
  if (motion == HIGH && !motionDetected) {
    motionDetected = true;
    Blynk.virtualWrite(V4, 1);
    Serial.println("Motion Detected");
  } else if (motion == LOW && motionDetected) {
    motionDetected = false;
    Blynk.virtualWrite(V4, 0);
    Serial.println("Motion Stopped");
  }
}

// Update LCD Display
void updateLCD() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.print("C ");

  lcd.print("H:");
  lcd.print(hum, 0);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("M:");
  lcd.print(soilValue);
  lcd.print(" ");

  lcd.print("P:");
  if (motionDetected) {
    lcd.print("Yes");
  } else {
    lcd.print("No ");
  }
}

void loop() {
  Blynk.run();
  timer.run();
}
