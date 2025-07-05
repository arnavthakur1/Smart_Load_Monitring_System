#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <HX711_ADC.h>

// WiFi credentials
#define WLAN_SSID       "Wokwi-GUEST"
#define WLAN_PASS       ""

// Adafruit IO credentials
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "arnav_p"
#define AIO_KEY         "aio_cugy85NWMDcXgGZDI2ARXmnF7vqo"

// HX711 pins
const int HX711_dout = 4;
const int HX711_sck = 5;
HX711_ADC LoadCell(HX711_dout, HX711_sck);

// WiFi and MQTT clients
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Adafruit IO Feeds
Adafruit_MQTT_Publish weightFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/weight");
Adafruit_MQTT_Publish alertFeed  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/alert");

void setup() {
  Serial.begin(115200);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  LoadCell.begin();
  LoadCell.start(2000);
  LoadCell.setCalFactor(420.0); // Adjust after calibration
  LoadCell.tare();
}

void loop() {
  if (mqtt.connected() == false) {
    int8_t ret;
    while ((ret = mqtt.connect()) != 0) {
      delay(5000);
    }
  }

  LoadCell.update();
  float weight = LoadCell.getData();
  Serial.print("Weight: ");
  Serial.println(weight);

  // Publish weight to Adafruit IO
  if (!weightFeed.publish(weight)) {
    Serial.println("Failed to send weight");
  }

  // Example: Send alert if thresholds are crossed
  if (weight > 3.0) {
    alertFeed.publish("overload");
  } else if (weight < 2.0) {
    alertFeed.publish("depletion");
  } else {
    alertFeed.publish("normal");
  }

  delay(5000); // Send every 5 seconds
}
