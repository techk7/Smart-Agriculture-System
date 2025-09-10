//Smart Agriculture Project

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <DHT.h>

// Replace with your network credentials
const char* ssid = "Galaxy A50B04B";
const char* password = "1234ABCD";

// Firebase settings
#define FIREBASE_HOST "https://smart-agriculture-da1c6-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "RnVnAc8hdnSdgUfSMPyRqURXy2bYD8FkrnLKeQnK"

// FirebaseESP8266 instance
FirebaseData firebaseData;t

// Pin definitions
const int relayPin = D1; // GPIO pin connected to the relay
const int rainSensorPin = D2; // GPIO pin connected to the rain sensor
const int dhtPin = D3; // GPIO pin connected to the DHT11 sensor

// DHT11 settings
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE);

// Soil moisture threshold
const int moistureThreshold = 30; // Adjust this value as needed

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");

  // Connect to Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  // Check Firebase connection
  if (!Firebase.beginStream(firebaseData, "/")) {
    Serial.println("Could not begin stream");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println();
  }

  // Initialize relay pin
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Ensure the relay is off initially

  // Initialize rain sensor pin
  pinMode(rainSensorPin, INPUT);

  // Initialize DHT11 sensor
  dht.begin();
}

void loop() {
  // Read the soil moisture sensor value
  int sensorValue = analogRead(A0);
  
  // Map the sensor value to a percentage (0-100)
  int moistureLevel = map(sensorValue, 1023, 0, 0, 100);

  // Print the value to the Serial Monitor
  Serial.print("Moisture Level: ");
  Serial.println(moistureLevel);

  // Update the soil moisture value in Firebase
  if (Firebase.setInt(firebaseData, "/soilMoisture", moistureLevel)) {
    Serial.println("Firebase update successful");
  } else {
    Serial.println("Firebase update failed");
    Serial.println("REASON: " + firebaseData.errorReason());
  }

  // Read the rain sensor value
  int rainSensorValue = digitalRead(rainSensorPin);

  // Determine rain status
  String rainStatus = (rainSensorValue == LOW) ? "RAINING" : "NO_RAIN";

  // Update the rain sensor status in Firebase
  if (Firebase.setString(firebaseData, "/rainStatus", rainStatus)) {
    Serial.println("Firebase rain status update successful");
  } else {
    Serial.println("Firebase rain status update failed");
    Serial.println("REASON: " + firebaseData.errorReason());
  }

  // Control the relay based on soil moisture level and rain sensor
  String pumpState;
  if (rainSensorValue == LOW) { // Assuming LOW indicates rain detected
    digitalWrite(relayPin, LOW); // Turn off the pump
    pumpState = "OFF";
    Serial.println("Rain detected, Pump OFF");
  } else {
    if (moistureLevel < moistureThreshold) {
      digitalWrite(relayPin, HIGH); // Turn on the pump
      pumpState = "ON";
      Serial.println("Pump ON");
    } else {
      digitalWrite(relayPin, LOW); // Turn off the pump
      pumpState = "OFF";
      Serial.println("Pump OFF");
    }
  }

  // Update the pump state in Firebase
  if (Firebase.setString(firebaseData, "/pumpState", pumpState)) {
    Serial.println("Firebase pump state update successful");
  } else {
    Serial.println("Firebase pump state update failed");
    Serial.println("REASON: " + firebaseData.errorReason());
  }

  // Read temperature and humidity from DHT11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if any reads failed and exit early (to try again).
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Print temperature and humidity to the Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C ");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Update temperature in Firebase
  if (Firebase.setFloat(firebaseData, "/temperature", temperature)) {
    Serial.println("Firebase temperature update successful");
  } else {
    Serial.println("Firebase temperature update failed");
    Serial.println("REASON: " + firebaseData.errorReason());
  }

  // Update humidity in Firebase
  if (Firebase.setFloat(firebaseData, "/humidity", humidity)) {
    Serial.println("Firebase humidity update successful");
  } else {
    Serial.println("Firebase humidity update failed");
    Serial.println("REASON: " + firebaseData.errorReason());
  }

  // Wait for 5 seconds before reading again
  delay(5000);
}