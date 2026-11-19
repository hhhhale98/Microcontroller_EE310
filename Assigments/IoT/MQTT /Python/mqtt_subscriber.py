
import paho.mqtt.client as mqtt
import mysql.connector

from db_config import (
    DB_HOST,
    DB_NAME,
    DB_USER,
    DB_PASSWORD,
    DB_PORT
)

# HiveMQ settings
BROKER = "broker.hivemq.com"
PORT = 8883

# ESP8266 publishing topic
TOPIC = "testtopic/temp/outTopic/2gsfsyyr72658yfh77"


# Connect to Hostinger database
def connect_database():
    try:
        connection = mysql.connector.connect(
            host=DB_HOST,
            database=DB_NAME,
            user=DB_USER,
            password=DB_PASSWORD,
            port=DB_PORT
        )

        print("Connected to Hostinger database!")
        return connection

    except mysql.connector.Error as error:
        print("Database connection failed:", error)
        return None


# Save potentiometer reading in MySQL
def save_reading(value):
    global db_connection

    try:
        if db_connection is None or not db_connection.is_connected():
            db_connection = connect_database()

        if db_connection is None:
            print("Reading was not saved.")
            return

        cursor = db_connection.cursor()

        sql = """
            INSERT INTO mqtt_readings (potentiometer_value)
            VALUES (%s)
        """

        cursor.execute(sql, (value,))
        db_connection.commit()

        print("Reading saved to Hostinger database!")

        cursor.close()

    except mysql.connector.Error as error:
        print("Database insertion failed:", error)


# MQTT connection callback
def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("Connected to HiveMQ successfully!")
        client.subscribe(TOPIC)
        print("Subscribed to:", TOPIC)
    else:
        print("MQTT connection failed:", reason_code)


# MQTT message callback
def on_message(client, userdata, message):
    value = message.payload.decode().strip()

    print("-----------------------------")
    print("Potentiometer value received:", value)

    try:
        potentiometer_value = int(value)

        if 0 <= potentiometer_value <= 1023:
            save_reading(potentiometer_value)
        else:
            print("Invalid potentiometer value.")

    except ValueError:
        print("Received message is not a valid number.")


# Connect to database
db_connection = connect_database()

# Create MQTT client
client = mqtt.Client(
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
    client_id="PythonRelay_2gsfsyyr72658yfh77"
)

client.on_connect = on_connect
client.on_message = on_message

# Enable TLS encryption
client.tls_set()

# Connect to HiveMQ
print("Connecting to HiveMQ...")
client.connect(BROKER, PORT, 60)

# Keep receiving messages
client.loop_forever()
