#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

#define DHTTYPE DHT11
#define DHTPIN D4

DHT dht(DHTPIN, DHTTYPE);

// Replace with your network credentials
const char* ssid = "Node_Mcu";
const char* password = "#$fst896#$5";
const int fireSensorPin = 14;
int smokevalue;
int fireSensorValue;
int smokepin = A0;
float temperature;
float humidity;
float temp;
float pressure;

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);  // Typical gateway IP address
IPAddress subnet(255, 255, 255, 0);

// Set web server port number to 80
ESP8266WebServer server(80);

// Placeholder functions for reading sensor data
float readTemperature() {
  Serial.print("Sending Smoke Data: ");
  Serial.println(temperature);
  return temperature; 
}

float readSmoke() {
  Serial.print("Sending Smoke Data: ");
  Serial.println(smokevalue);
  return smokevalue; // Simulated data
}

float readFire() {
  Serial.print("Sending Smoke Data: ");
  Serial.println(fireSensorValue);
  return fireSensorValue;
}

float readPressure() {
  temp=temperature+273.15;
  pressure=(humidity)*(461.5)*(temp);// Simulated data
  return pressure;
}

float readHumidity() {
  Serial.print("Sending Smoke Data: ");
  Serial.println(humidity);
  return humidity;
}

// Simple decision tree for classification
int classify(float temperature, float smoke, float fire, float pressure) {
    // Example rules:
    if (temperature > 26.0) {
        if (smoke > 300) {
            return 1; // Class 1: Potential Fire
        } else {
            return 0; // Class 0: Normal
        }
    } else {
        if (fire > 0.5) {
            return 1; // Class 1: Potential Fire
        } else {
            return 0; // Class 0: Normal
        }
    }
}

void setup() {
    // Initialize the DHT sensor
  dht.begin();
  pinMode(smokepin, INPUT); // Smoke sensor pin
  pinMode(fireSensorPin, INPUT);
  Serial.begin(115200); // Start the Serial communication
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output

  // Set the ESP8266 as an access point
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  // Print the IP address
  Serial.println();
  Serial.println("AP mode setup complete.");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Define routing
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  static unsigned long lastUpdateTime = 0;
  unsigned long currentMillis = millis();

  // Update smoke value every 100ms
  if (currentMillis - lastUpdateTime >= 100) {
    // Read humidity
    humidity = dht.readHumidity();
    // Read temperature in Celsius
    temperature = dht.readTemperature();
    smokevalue = analogRead(smokepin);
    fireSensorValue = digitalRead(fireSensorPin);
    lastUpdateTime = currentMillis;
  }
  server.handleClient();
}


void handleRoot() {
  float temperature = readTemperature();
  float smoke = readSmoke();
  float fire = readFire();
  float pressure = readPressure();
  float humidity = readHumidity();

  int prediction = classify(temperature, smoke, fire, pressure);

  String html = "<!DOCTYPE html><html><head><title>Sensor Insights & Predictions</title>";
  html += "<meta http-equiv=\"refresh\" content=\"3\">"; // Auto-refresh every 3 seconds
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  html += "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.3/css/all.min.css\">";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 0; text-align: center; background-color: #f2f2f2; color: #333; }";
  html += "h1 { background-color: #4CAF50; color: white; padding: 20px; margin-bottom: 20px; }";
  html += ".container { display: flex; flex-wrap: wrap; justify-content: center; }";
  html += ".sensor { background: white; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); margin: 10px; padding: 20px; width: 200px; }";
  html += ".sensor i { font-size: 2.5rem; color: #4CAF50; }";
  html += ".sensor p { font-size: 1.2rem; margin: 10px 0 0; }";
  html += ".prediction { font-size: 1.5rem; margin: 20px 0; padding: 10px; border-radius: 10px; position: relative; }";
  html += ".prediction strong { color: #fff; padding: 8px 20px; border-radius: 20px; }"; // Adjust color and padding for strong tag
  html += ".normal { background-color: #007BFF; }"; // Background color for normal prediction
  html += ".fire { background-color: #FF5733; }"; // Background color for potential fire prediction
  html += ".fire strong { background-color: #FF5733; }"; // Match background color for strong tag in fire prediction
  html += ".fire-animation { animation: pulse 1s infinite; }"; // Define animation
  html += "@keyframes pulse {";
  html += "  0% { transform: scale(1); }";
  html += "  50% { transform: scale(1.1); }";
  html += "  100% { transform: scale(1); }";
  html += "}";
  html += "</style></head><body>";
  html += "<h1>Sensor Insights & Predictions</h1>";
  html += "<div class=\"container\">";
  html += "<div class=\"sensor\"><i class=\"fas fa-thermometer-half\"></i><p>Temperature: " + String(temperature) + " &deg;C</p></div>";
  html += "<div class=\"sensor\"><i class=\"fas fa-smog\"></i><p>Smoke Level: " + String(smoke) + "</p></div>";
  html += "<div class=\"sensor\"><i class=\"fas fa-fire\"></i><p>Fire Level: " + String(fire) + "</p></div>";
  html += "<div class=\"sensor\"><i class=\"fas fa-tachometer-alt\"></i><p>Pressure: " + String(pressure) + " hPa</p></div>";
  html += "<div class=\"sensor\"><i class=\"fas fa-water\"></i><p>Humidity: " + String(humidity) + " %</p></div>";
  html += "</div>";
  if (prediction == 1) {
    html += "<div class=\"prediction fire fire-animation\"><strong>Potential Fire!</strong></div>"; // Apply animation and fire style if potential fire
  } else {
    html += "<div class=\"prediction normal\"><strong>Normal</strong></div>"; // Normal prediction style
  }
  html += "</body></html>";
  server.send(200, "text/html", html);
}
void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}
