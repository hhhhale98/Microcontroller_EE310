// Title: Programming ESP8266
//-----------------------------
//Purpose:The goal of this code is to use a pushbutton (node_1) and a tilt sensor (node_2) as switches that when they are activated, 
//they send the data that is being collected at that exact moment by the ultrasonic sensor and the photoresistor module. The ESP8266 sends
//the new data trhough wifi connection to a database that has that data displayed in tables and graphs located on a website that updates
//everytime a new entry is succesfully received. There are three data tables. The measurements of the table are correctly timestamped using
//pacific time zone. If connection is unsuccesfull and the data cannor reach the database, the code will try to resend the data with same timestamp
//in a window of 60 seconds. After that, the data will be lost. The code also prints in the terminal the basic information (Json) of the data that has been 
//succesfully sent at that exact moment. 
// Dependencies: This is for a ESP8266 and this is the code that goes into the ESP8266. The codes for database and website managing are app.js, app.php, 
//db_insert2.php and index2.php.
// Compiler: Visual Studio Code and PlataformIO.
// Author: Heybee Esquer Cheno 
// VERSION 2.1
//Node_1: Pushbutton activation
//Node_2: Tilt switch activation
// Sensors:
//- HC-SR04 ultrasonic distance sensor
//- Photoresistor light sensor
// Features:
//- Wi-Fi connection
//- NTP time synchronization
//- USB Serial timezone selection
//- HTTPS database transmission
//- Automatic retransmission
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

// 1. WIFI CONFIGURATION

const char* WIFI_SSID = "INSERTACTUALNAMEOFNETWORK"; //insert credentials
const char* WIFI_PASSWORD = "INSERTACTUALPASSWORD"; //insert credentials

// 2. SERVER CONFIGURATION

const char* SERVER_URL =
    "https://alejandraab.com/db_insert2.php"; //where to send the data
const char* NTP_SERVER = "pool.ntp.org"; //where to retrieve time zones from

// 3. PIN CONFIGURATION

// HC-SR04
const int TRIG_PIN = D1;
const int ECHO_PIN = D2;

// Photoresistor
const int LIGHT_PIN = A0;

// Pushbutton
const int BUTTON_PIN = D5;

// Tilt switch
const int TILT_PIN = D6;

// 4. SWITCH DEBOUNCE SETTINGS

const unsigned long BUTTON_DEBOUNCE_MS = 40;
const unsigned long TILT_DEBOUNCE_MS = 1000; //to avoid sudden movements by accidents send data

// 5. TRANSMISSION SETTINGS

const int QUEUE_SIZE = 10;
const unsigned long RETRY_INTERVAL_MS = 30000; //retry send data at 30 seconds if unsuccesfull at 0seconds
const unsigned long RETRY_WINDOW_MS = 60000; //another 30 seconds
const unsigned long HTTP_TIMEOUT_MS = 5000; //after trying last time resend and unsuccesful, close window inmediately

// 6. TIMEZONE CONFIGURATION

struct TimeZone {
    const char* name;
    const char* tz;
};

TimeZone timeZones[] = {
    {"Pacific", "PST8PDT,M3.2.0/2,M11.1.0/2"},
    {"Mountain", "MST7MDT,M3.2.0/2,M11.1.0/2"},
    {"Central", "CST6CDT,M3.2.0/2,M11.1.0/2"},
    {"Eastern", "EST5EDT,M3.2.0/2,M11.1.0/2"},
    {"UTC", "UTC0"}
};

const int TIMEZONE_COUNT =
    sizeof(timeZones) / sizeof(timeZones[0]);
int selectedTimeZone = 0; //my selection of zone 1-5


// 7. TRANSMISSION QUEUE

struct SensorReading {

    char nodeName[16];
    float distance;
    int light;
    char timestamp[20];
    unsigned long firstAttemptTime;
    unsigned long nextAttemptTime;
    int attempts;
};

SensorReading readingQueue[QUEUE_SIZE];

int queueHead = 0;
int queueTail = 0;
int queueCount = 0;

// 8. SWITCH STATE VARIABLES

// Pushbutton
int buttonLastRawState = HIGH;
int buttonStableState = HIGH;

unsigned long buttonLastDebounceTime = 0;

// Tilt switch
int tiltLastRawState = LOW;
int tiltStableState = LOW;

unsigned long tiltLastDebounceTime = 0;

// 9. GENERAL TIMING

unsigned long lastTimeDisplay = 0;
const unsigned long TIME_DISPLAY_INTERVAL = 30000;

// 10. WIFI CONNECTION (function to connect to wifi)

void connectWiFi() {

    Serial.println();
    Serial.println("Connecting to Wi-Fi...");

    WiFi.mode(WIFI_STA);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD); //to enter credentials

    while (WiFi.status() != WL_CONNECTED) {

        delay(500);

        Serial.print(".");
    }

    Serial.println();

    Serial.println("Wi-Fi connected!"); //wifi status

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.print("RSSI: "); //print RSSI
    Serial.print(WiFi.RSSI()); //call function for RSSI calculation
    Serial.println(" dBm");

    WiFi.setAutoReconnect(true);
}


// 11. TIMEZONE SELECTION (function to let the user select a timezone)

void selectTimeZone() {

    Serial.println();
    Serial.println("========== TIMEZONE SELECTION ==========");

    for (int i = 0; i < TIMEZONE_COUNT; i++) {

        Serial.print(i + 1);
        Serial.print(". ");

        Serial.println(timeZones[i].name);
    }

    Serial.println();
    Serial.println("Enter timezone number (1-5):");

    while (!Serial.available()) {
        delay(100);
    }

    int choice = Serial.parseInt();

    while (Serial.available()) {
        Serial.read();
    }

    if (choice < 1 || choice > TIMEZONE_COUNT) {

        Serial.println("Invalid selection. Defaulting to Pacific.");

        choice = 1;
    }

    selectedTimeZone = choice - 1;

    configTime(
        timeZones[selectedTimeZone].tz,
        NTP_SERVER
    );

    Serial.print("Selected timezone: ");

    Serial.println(timeZones[selectedTimeZone].name);

    Serial.println("========================================");
}



// 12. WAIT FOR NTP TIME SYNCHRONIZATION (function that waits for NTP synchronization)

void waitForTimeSync() {

    Serial.println();
    Serial.println("Waiting for NTP time synchronization...");

    time_t now;

    unsigned long startTime = millis();

    while (millis() - startTime < 30000) {

        time(&now);

        if (now > 1704067200) {

            Serial.println("Time synchronization successful!");

            return;
        }

        delay(500);

        Serial.print(".");
    }

    Serial.println();

    Serial.println("WARNING: Time synchronization timed out.");
}

// 13. DISPLAY CURRENT TIME

void displayCurrentTime() {

    time_t now;

    time(&now);

    if (now <= 1704067200) {

        Serial.println("Time is not synchronized.");

        return;
    }

    struct tm utcTime;
    struct tm localTime;

    gmtime_r(&now, &utcTime);

    localtime_r(&now, &localTime);

    char utcBuffer[25];
    char localBuffer[25];

    strftime(
        utcBuffer,
        sizeof(utcBuffer),
        "%Y-%m-%d %H:%M:%S",
        &utcTime
    );

    strftime(
        localBuffer,
        sizeof(localBuffer),
        "%Y-%m-%d %H:%M:%S",
        &localTime
    );

    Serial.println();
    Serial.println("========== CURRENT TIME ==========");

    Serial.print("UTC time: ");
    Serial.println(utcBuffer);

    Serial.print("Selected timezone: ");
    Serial.println(timeZones[selectedTimeZone].name);

    Serial.print("Local time: ");
    Serial.println(localBuffer);

    Serial.println("==================================");
}


// 14. READ DISTANCE SENSOR (function to read the ultrasonic sensor)

float readDistance() {

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    unsigned long duration =
        pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration == 0) {
        return -1;
    }
    float distance = duration * 0.0343 / 2.0;
    if (distance < 0 || distance > 400) {
        return -1;
    }
    return distance;
}

// 15. READ LIGHT SENSOR (function to red the photosensor)

int readLight() {
    return analogRead(LIGHT_PIN);
}

// 16. ADD READING TO QUEUE (function to add readings to the queue)

bool enqueueReading(SensorReading reading) {

    if (queueCount >= QUEUE_SIZE) {
        Serial.println("ERROR: Transmission queue is full.");
        Serial.println("Reading could not be queued.");
        return false;
    }

    reading.attempts = 0;
    reading.firstAttemptTime = millis();
    reading.nextAttemptTime = millis();
    readingQueue[queueTail] = reading;
    queueTail = (queueTail + 1) % QUEUE_SIZE;
    queueCount++;
    Serial.println("Reading added to transmission queue.");
    return true;
}


// 17. CAPTURE NOW SENSOR READING. Timestamp readings as well.

void sendLocalReading(
    const char* nodeName,
    const char* activationType
) {

    float distance = readDistance(); //get distance value from function
    int light = readLight(); //get light value from function
    time_t now;
    time(&now);
    if (now <= 1704067200) {
        Serial.println("ERROR: Valid timestamp unavailable.");
        Serial.println("Reading was not queued.");
        return;
    }
    if (distance < 0) {
        Serial.println("ERROR: Invalid distance reading.");
        Serial.println("Reading was not queued.");
        return;
    }

    struct tm localTime;
    localtime_r(&now, &localTime);
    SensorReading reading = {};
    strncpy(
        reading.nodeName,
        nodeName,
        sizeof(reading.nodeName) - 1
    );
    reading.distance = distance;
    reading.light = light;
    strftime(
        reading.timestamp,
        sizeof(reading.timestamp),
        "%Y-%m-%d %H:%M:%S",
        &localTime
    );

    Serial.println();
    Serial.println("========== SENSOR ACTIVATION ==========");

    Serial.print("Node: ");
    Serial.println(nodeName);

    Serial.print("Activation: ");
    Serial.println(activationType);

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    Serial.print("Light level: ");
    Serial.println(light);

    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    Serial.print("Timestamp: ");
    Serial.println(reading.timestamp);

    enqueueReading(reading);

    Serial.println("=======================================");
}

// 18. SEND READING USING HTTPS (function that sends readings using HTTPS)

bool transmitReading(SensorReading &reading) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi disconnected.");
        return false;
    }
    Serial.println();
    Serial.println("Sending reading to database...");
    // HTTPS client
    BearSSL::WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setTimeout(HTTP_TIMEOUT_MS);

    // Format distance
    char distanceString[16];

    dtostrf(
        reading.distance,
        1,
        2,
        distanceString
    );

    String distanceValue = String(distanceString);
    distanceValue.trim();

    // Encode timestamp space as %20
    char encodedTimestamp[24];

    snprintf(
        encodedTimestamp,
        sizeof(encodedTimestamp),
        "%.10s%%20%.8s",
        reading.timestamp,
        reading.timestamp + 11
    );

    // Build HTTPS URL
    String url = String(SERVER_URL);

    url += "?nodeId=";
    url += reading.nodeName;

    url += "&distance=";
    url += distanceValue;

    url += "&light=";
    url += String(reading.light);

    url += "&timeReceived=";
    url += encodedTimestamp;

    Serial.print("URL: ");
    Serial.println(url);

    if (!http.begin(client, url)) {

        Serial.println("Unable to start HTTPS connection.");

        return false;
    }

    int httpCode = http.GET();

    String response = "";

    if (httpCode > 0) {

        response = http.getString();

        Serial.print("HTTP response code: ");
        Serial.println(httpCode);

        Serial.print("Server response: ");
        Serial.println(response);

    } else {

        Serial.print("HTTP request failed: ");

        Serial.println(http.errorToString(httpCode));
    }

    bool delivered = false;

    if (httpCode == 200) {

        if (
            response.indexOf("\"success\"") >= 0 ||
            response.indexOf("\"already_stored\"") >= 0
        ) {

            delivered = true;
        }
    }

    http.end();

    if (delivered) {

        Serial.println("Transmission confirmed!");

    } else {

        Serial.println("Transmission was not confirmed.");
    }

    return delivered;
}


// 19. PROCESS TRANSMISSION QUEUE (function that manages retransmission on 60 sec windows)

void processPendingReadings() {

    if (queueCount == 0) {
        return;
    }

    SensorReading &reading = readingQueue[queueHead];
    unsigned long currentTime = millis();
    unsigned long elapsedTime =
        currentTime - reading.firstAttemptTime;
    if (currentTime < reading.nextAttemptTime) {
        return;
    }
    if (
        reading.attempts > 0 &&
        elapsedTime > RETRY_WINDOW_MS
    ) {

        Serial.println();
        Serial.println("Retry window expired.");
        Serial.println("Reading could not be delivered.");
        queueHead = (queueHead + 1) % QUEUE_SIZE;
        queueCount--;
        return;
    }

    reading.attempts++;
    Serial.print("Transmission attempt: ");
    Serial.println(reading.attempts);
    bool success = transmitReading(reading);

    if (success) {
        Serial.println("Reading successfully delivered.");
        queueHead = (queueHead + 1) % QUEUE_SIZE;
        queueCount--;
        return;
    }

    unsigned long elapsedAfterAttempt =
        millis() - reading.firstAttemptTime;

    if (
        reading.attempts >= 3 ||
        elapsedAfterAttempt >= RETRY_WINDOW_MS
    ) {
        Serial.println("Retry window expired.");
        Serial.println("Reading could not be delivered.");
        queueHead = (queueHead + 1) % QUEUE_SIZE;
        queueCount--;
        return;
    }

    reading.nextAttemptTime =
        reading.firstAttemptTime +
        (unsigned long)reading.attempts * RETRY_INTERVAL_MS;

    Serial.println("Scheduling retry in 30 seconds.");
}

// 20. CHECK PUSHBUTTON (function that detects button activation)
void checkPushbutton() {
    int rawState = digitalRead(BUTTON_PIN);
    if (rawState != buttonLastRawState) {
        buttonLastDebounceTime = millis();
        buttonLastRawState = rawState;
    }
    if (
        millis() - buttonLastDebounceTime >
        BUTTON_DEBOUNCE_MS
    ) {
        if (rawState != buttonStableState) {
            buttonStableState = rawState;
            // Button pressed: active LOW
            if (buttonStableState == LOW) {
                sendLocalReading(
                    "node_1",
                    "Pushbutton"
                );
            }
        }
    }
}


// 21. CHECK TILT SWITCH (function that detects tilt activation)
void checkTiltSwitch() {
    int rawState = digitalRead(TILT_PIN);
    if (rawState != tiltLastRawState) {
        tiltLastDebounceTime = millis();
        tiltLastRawState = rawState;
    }
    if (
        millis() - tiltLastDebounceTime >
        TILT_DEBOUNCE_MS
    ) {
        if (rawState != tiltStableState) {
            tiltStableState = rawState;
            // Tilt activated: active HIGH
            if (tiltStableState == HIGH) {
                sendLocalReading(
                    "node_2",
                    "Tilt Switch"
                );
            }
        }
    }
}

// 22. SETUP (wifi, buttons, time)
void setup() {

    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println("======================================");
    Serial.println("ESP8266 IoT SENSOR PROJECT");
    Serial.println("VERSION 2.1");
    Serial.println("======================================");
    // Sensor pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    // Switch pins
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(TILT_PIN, INPUT);
    // Initialize switch states
    buttonLastRawState = digitalRead(BUTTON_PIN);
    buttonStableState = buttonLastRawState;
    tiltLastRawState = digitalRead(TILT_PIN);
    tiltStableState = tiltLastRawState;
    // Connect to Wi-Fi
    connectWiFi();
    // Select timezone
    selectTimeZone();
    // Synchronize time
    waitForTimeSync();
    // Display initial time
    displayCurrentTime();
    lastTimeDisplay = millis();
    Serial.println();
    Serial.println("System ready!");
    Serial.println("Press button for Node_1.");
    Serial.println("Activate tilt switch for Node_2.");
    Serial.println();
}

// 23. MAIN LOOP 
void loop() {
    // Check physical switches
    checkPushbutton();
    checkTiltSwitch();
    // Process pending transmissions
    processPendingReadings();
    // Display time every 30 seconds
    if (
        millis() - lastTimeDisplay >=
        TIME_DISPLAY_INTERVAL
    ) {
        displayCurrentTime();
        lastTimeDisplay = millis();
    }
    delay(5);
}
