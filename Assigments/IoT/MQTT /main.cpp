
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// Wi-Fi credentials
const char* WIFI_SSID = "network";
const char* WIFI_PASSWORD = "password";

// MQTT broker settings
const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 8883;

// Unique client ID
const char* CLIENT_ID = "ESP8266_2gsfsyyr72658yfh77";

// MQTT topics
const char* MQTT_TOPIC =
    "testtopic/temp/outTopic/2gsfsyyr72658yfh77";

const char* MQTT_SUB_TOPIC =
    "testtopic/temp/inTopic";

// NEW: LED feedback topic
const char* LED_STATE_TOPIC =
    "testtopic/temp/ledState/2gsfsyyr72658yfh77";

// Hardware pins
const int LED_PIN = D1;
const int BUTTON_PIN = D2;

// Secure MQTT client
BearSSL::WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

// Publishing interval
unsigned long lastPublish = 0;
const unsigned long publishInterval = 15000;

// Button variables
int lastButtonState = HIGH;
int buttonState = HIGH;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

bool buttonMessageActive = false;
unsigned long buttonMessageTime = 0;

// Connect to Wi-Fi
void connectWiFi() {
    Serial.print("Connecting to WiFi");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected!");

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

// NEW: Publish the actual LED pin state
void publishLEDState() {

    if (!mqttClient.connected()) {
        return;
    }

    if (digitalRead(LED_PIN) == HIGH) {

        mqttClient.publish(LED_STATE_TOPIC, "1", true);
        Serial.println("LED feedback published: ON");

    } else {

        mqttClient.publish(LED_STATE_TOPIC, "0", true);
        Serial.println("LED feedback published: OFF");

    }
}

// MQTT callback: receives incoming messages
void callback(char* topic, byte* payload, unsigned int length) {

    String message = "";

    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    Serial.println("-------------------------");
    Serial.print("Message received on: ");
    Serial.println(topic);

    Serial.print("Message: ");
    Serial.println(message);

    if (String(topic) == MQTT_SUB_TOPIC) {

        if (message == "1") {

            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED ON!");

            // NEW: Send LED state feedback
            publishLEDState();
        }

        else if (message == "0") {

            digitalWrite(LED_PIN, LOW);
            Serial.println("LED OFF!");

            // NEW: Send LED state feedback
            publishLEDState();
        }
    }
}

// Connect to MQTT broker and subscribe
void reconnectMQTT() {

    while (!mqttClient.connected()) {

        Serial.println("Connecting to HiveMQ using TLS...");

        if (mqttClient.connect(CLIENT_ID)) {

            Serial.println("MQTT connected successfully!");

            if (mqttClient.subscribe(MQTT_SUB_TOPIC)) {

                Serial.println("Subscribed successfully!");
                Serial.println(MQTT_SUB_TOPIC);

                // NEW: Report current LED state after reconnecting
                publishLEDState();

            } else {

                Serial.println("Subscription failed!");
            }

        } else {

            Serial.print("Connection failed. State: ");
            Serial.println(mqttClient.state());

            delay(3000);
        }
    }
}

// Publish a message
void publishMessage(const char* topic, const char* message) {

    bool success = mqttClient.publish(topic, message);

    Serial.print("Publishing: ");
    Serial.print(message);

    if (success) {
        Serial.println(" - Successful!");
    } else {
        Serial.println(" - Failed!");
    }
}

void setup() {

    Serial.begin(115200);

    // Configure hardware
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Connect to Wi-Fi
    connectWiFi();

    // Temporary testing: skip certificate verification
    espClient.setInsecure();

    // Configure MQTT
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setKeepAlive(60);
    mqttClient.setSocketTimeout(30);

    // Set callback
    mqttClient.setCallback(callback);
}

void loop() {

    // Check Wi-Fi connection
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    }

    // Check MQTT connection
    if (!mqttClient.connected()) {
        reconnectMQTT();
    }

    // Process incoming MQTT messages
    mqttClient.loop();

    // Read physical button
    int reading = digitalRead(BUTTON_PIN);

    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {

        if (reading != buttonState) {

            buttonState = reading;

            // Button pressed
            if (buttonState == LOW) {

                Serial.println("Button pressed!");

                publishMessage(MQTT_TOPIC, "1");

                buttonMessageActive = true;
                buttonMessageTime = millis();
            }
        }
    }

    lastButtonState = reading;

    // Publish 0 five seconds after button press
    if (buttonMessageActive &&
        millis() - buttonMessageTime >= 5000) {

        publishMessage(MQTT_TOPIC, "0");

        buttonMessageActive = false;
    }

    // Publish potentiometer value every 15 seconds
    if (millis() - lastPublish >= publishInterval) {

        lastPublish = millis();

        int sensorValue = analogRead(A0);
        String message = String(sensorValue);

        publishMessage(MQTT_TOPIC, message.c_str());
    }
}
