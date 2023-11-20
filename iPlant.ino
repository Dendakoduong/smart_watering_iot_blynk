//The Firmware Configuration as above
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""
// Include libraries
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Initialize LCD, DHT sensor, and set initial parameters
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(D4, DHT11);

// Blynk authentication 
char auth[] = "";
//WiFi credentials
char ssid[] = "Example";
char pass[] = "password";

// Pin assignments for sensors and actuators
int soilMoisturePin = A0;
int pumpPin = D3;
int soilMoistureThreshold = 30;  // Define the threshold below which to turn on the pump
bool isPumpOn = false;  // Initialize the pump state
bool isSmartWaterEnabled = true; // Initialize Smart Water as enabled by default

BlynkTimer timer; // Blynk timer for periodic function calls

void setup() {
  // Initialize LCD and clear the display
  lcd.init();
  lcd.backlight();
  lcd.clear();  // Clear the display
  lcd.setCursor(0, 0);
  lcd.print("            ");
  // Display project name on LCD
  lcd.setCursor(0, 1);
  lcd.print("iPlant");
  // Initialize DHT sensor and set pump pin as an output
  dht.begin();
  pinMode(pumpPin, OUTPUT);  // Set pump pin as an output

  // Connect to Blynk server
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  
  // Display loading message on LCD during setup
  lcd.setCursor(1, 0);
  lcd.print("System Loading");
  for (int a = 0; a <= 15; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(100);//fix from 500 to more faster startup
  }
  lcd.clear();

   // Set up periodic function call for soil moisture sensing
  timer.setInterval(100L, soilMoistureSensor);
}

   // Blynk function to handle the "Smart Water" button toggle
BLYNK_WRITE(V4) {
  int smartWaterState = param.asInt();
   // Set Smart Water state based on button toggle
  if (smartWaterState == 1) {
    isSmartWaterEnabled = true;
  } else {
    isSmartWaterEnabled = false;
  }
}

// Function to read soil moisture, update Blynk widgets, and control the pump
void soilMoistureSensor() {
  int value = analogRead(soilMoisturePin);
  value = map(value, 0, 1024, 0, 100);
  value = (value - 100) * -1;

  // Update Blynk widgets with data
  Blynk.virtualWrite(V0, value); // Update soil moisture value
  
  // Check and control pump based on Smart Water state and soil moisture threshold
  // If "Smart Water" is enabled, control the pump automatically
  if (isSmartWaterEnabled) {
  if (value < soilMoistureThreshold) {
    // Soil moisture is below threshold, turn ON the pump and update Blynk widget
    Blynk.virtualWrite(V1, 1); // Update the pump widget to ON
    digitalWrite(pumpPin, LOW); // Turn ON the pump
    isPumpOn = true; // Update pump state
  } else {
    // Soil moisture is above or equal to threshold, turn OFF the pump and update Blynk widget
    Blynk.virtualWrite(V1, 0); // Update the pump widget to OFF
    digitalWrite(pumpPin, HIGH); // Turn OFF the pump
    isPumpOn = false; // Update pump state
  }
}

// Read temperature and humidity from DHT11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

// Update Blynk widgets with temperature and humidity data
  Blynk.virtualWrite(V2, temperature); // Update the temperature widget
  Blynk.virtualWrite(V3, humidity); // Update the humidity widget

// Display temperature, humidity, and soil moisture on LCD
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature, 1); 
  lcd.print(" H:");
  lcd.print(humidity, 0);
  lcd.print("%    "); 

  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(value); 
  lcd.print("% P:");
  lcd.print(isPumpOn ? "ON       " : "OFF      ")
}

// Blynk function to handle manual pump control
BLYNK_WRITE(V1) {
  int manualControlState = param.asInt();
// Control the pump based on manual button state
  if (manualControlState == 1) {
    digitalWrite(pumpPin, LOW);  // Turn on the pump
    isPumpOn = true;
  } else {
    digitalWrite(pumpPin, HIGH);  // Turn off the pump
    isPumpOn = false;
  }
}

void loop() {
// Run Blynk and timer functions
  Blynk.run();
  timer.run();
}
