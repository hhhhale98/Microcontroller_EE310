
<?php
require_once __DIR__ . '/../secure/config.php';

$conn = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);

if ($conn->connect_error) {
    die("Database connection failed.");
}

$conn->set_charset("utf8mb4");

// Retrieve database timestamps consistently in UTC.
$conn->query("SET time_zone = '+00:00'");

$sql = "SELECT potentiometer_value, time_received
        FROM mqtt_readings
        ORDER BY time_received DESC
        LIMIT 50";

$result = $conn->query($sql);

$readings = [];

if ($result) {
    while ($row = $result->fetch_assoc()) {
        $readings[] = $row;
    }
}

$readings = array_reverse($readings);

$labels = [];
$values = [];

// Convert UTC database timestamps to California local time.
$utcTimezone = new DateTimeZone('UTC');
$localTimezone = new DateTimeZone('America/Los_Angeles');

foreach ($readings as $reading) {

    $date = new DateTime(
        $reading['time_received'],
        $utcTimezone
    );

    $date->setTimezone($localTimezone);

    $labels[] = $date->format('Y-m-d H:i:s');

    $values[] = (int)$reading['potentiometer_value'];
}

$latestValue = count($values) > 0
    ? $values[count($values) - 1]
    : "No readings";

$latestTime = count($labels) > 0
    ? $labels[count($labels) - 1]
    : "No readings";

$conn->close();
?>

<!DOCTYPE html>
<html lang="en">

<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<title>MQTT Potentiometer Monitoring</title>

<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<script src="https://unpkg.com/mqtt/dist/mqtt.min.js"></script>

<style>

body {
    font-family: Arial, sans-serif;
    background: #f4f7f6;
    margin: 0;
    padding: 30px;
    color: #333;
}

.container {
    max-width: 1000px;
    margin: auto;
}

h1 {
    text-align: center;
    color: #226b45;
}

.card {
    background: white;
    padding: 25px;
    border-radius: 10px;
    margin-bottom: 25px;
    box-shadow: 0 2px 8px rgba(0,0,0,0.08);
}

.reading {
    font-size: 42px;
    font-weight: bold;
    color: #26965b;
}

.refresh {
    background: #26965b;
    color: white;
    border: none;
    padding: 12px 22px;
    border-radius: 6px;
    cursor: pointer;
    font-size: 15px;
}

.refresh:hover {
    background: #1d7546;
}

.chart-container {
    height: 400px;
}

.connection {
    font-weight: bold;
    margin-bottom: 15px;
}

.led-buttons {
    display: flex;
    gap: 15px;
    margin-top: 20px;
}

.led-button {
    flex: 1;
    padding: 16px;
    border: none;
    border-radius: 7px;
    color: white;
    font-size: 17px;
    font-weight: bold;
    cursor: pointer;
}

.on-button {
    background: #26965b;
}

.off-button {
    background: #555;
}

.led-button:hover {
    opacity: 0.85;
}

.status {
    font-size: 22px;
    font-weight: bold;
    margin-top: 15px;
}

</style>
</head>

<body>

<div class="container">

    <h1>MQTT Potentiometer Monitoring</h1>

    <!-- MQTT CONNECTION -->
    <div class="card">

        <h2>MQTT Connection</h2>

        <div class="connection" id="mqttStatus">
            Connecting to HiveMQ...
        </div>

    </div>

    <!-- LATEST POTENTIOMETER READING -->
    <div class="card">

        <h2>Latest Potentiometer Reading</h2>

        <div class="reading" id="liveReading">
            <?php echo htmlspecialchars((string)$latestValue); ?>
        </div>

        <p>Last database reading:
            <?php echo htmlspecialchars((string)$latestTime); ?>
        </p>

        <p id="liveTime">Waiting for live MQTT reading...</p>

        <button class="refresh" onclick="location.reload()">
            Refresh Database Graph
        </button>

    </div>

    <!-- GRAPH -->
    <div class="card">

        <h2>Potentiometer Readings Over Time</h2>

        <?php if (count($values) > 0): ?>

            <div class="chart-container">
                <canvas id="mqttChart"></canvas>
            </div>

        <?php else: ?>

            <p>No readings have been recorded yet.</p>

        <?php endif; ?>

    </div>

    <!-- LED CONTROL -->
    <div class="card">

        <h2>Remote LED Control</h2>

        <p>Use these buttons to control the ESP8266 LED.</p>

        <div class="led-buttons">

            <button
                class="led-button on-button"
                onclick="controlLED('1')">
                TURN ON
            </button>

            <button
                class="led-button off-button"
                onclick="controlLED('0')">
                TURN OFF
            </button>

        </div>

        <p id="commandStatus">
            No LED command sent yet.
        </p>

        <h3>Actual LED State</h3>

        <div class="status" id="ledStatus">
            Waiting for ESP8266 feedback...
        </div>

    </div>

</div>

<!-- GRAPH JAVASCRIPT -->
<?php if (count($values) > 0): ?>

<script>

const labels = <?php
echo json_encode(
    $labels,
    JSON_HEX_TAG | JSON_HEX_APOS | JSON_HEX_QUOT | JSON_HEX_AMP
);
?>;

const values = <?php echo json_encode($values); ?>;

const ctx = document.getElementById('mqttChart');

new Chart(ctx, {

    type: 'line',

    data: {

        labels: labels,

        datasets: [{

            label: 'Potentiometer Value',

            data: values,

            borderColor: '#26965b',

            backgroundColor: 'rgba(38, 150, 91, 0.15)',

            borderWidth: 2,

            fill: true,

            tension: 0.2

        }]

    },

    options: {

        responsive: true,

        maintainAspectRatio: false,

        scales: {

            y: {
                min: 0,
                max: 1023,
                title: {
                    display: true,
                    text: 'Potentiometer Value'
                }
            },

            x: {
                title: {
                    display: true,
                    text: 'Time Received (California Time)'
                }
            }

        }

    }

});

</script>

<?php endif; ?>


<!-- MQTT JAVASCRIPT -->
<script>

// HiveMQ connection
const MQTT_BROKER = 'wss://broker.hivemq.com:8884/mqtt';

// Your existing MQTT topics
const POT_TOPIC =
    'testtopic/temp/outTopic/2gsfsyyr72658yfh77';

const LED_COMMAND_TOPIC =
    'testtopic/temp/inTopic';

// ESP8266 LED feedback topic
const LED_STATE_TOPIC =
    'testtopic/temp/ledState/2gsfsyyr72658yfh77';

const mqttStatus = document.getElementById('mqttStatus');
const liveReading = document.getElementById('liveReading');
const liveTime = document.getElementById('liveTime');
const commandStatus = document.getElementById('commandStatus');
const ledStatus = document.getElementById('ledStatus');


// Create MQTT client
const client = mqtt.connect(MQTT_BROKER, {

    clientId: 'WebDashboard_' +
        Math.random().toString(16).substring(2, 10),

    clean: true,

    connectTimeout: 10000,

    reconnectPeriod: 3000

});


// When connected to HiveMQ
client.on('connect', function () {

    mqttStatus.textContent = 'Connected to HiveMQ';
    mqttStatus.style.color = '#26965b';

    client.subscribe(POT_TOPIC);

    client.subscribe(LED_STATE_TOPIC);

});


// Connection error
client.on('error', function (error) {

    mqttStatus.textContent =
        'MQTT connection error: ' + error.message;

    mqttStatus.style.color = 'red';

});


// When disconnected
client.on('offline', function () {

    mqttStatus.textContent = 'MQTT disconnected';
    mqttStatus.style.color = 'red';

});


// Receive MQTT messages
client.on('message', function (topic, message) {

    const value = message.toString().trim();

    // Potentiometer readings
    if (topic === POT_TOPIC) {

        const number = Number(value);

        if (Number.isInteger(number) &&
            number >= 0 &&
            number <= 1023) {

            liveReading.textContent = number;

            liveTime.textContent =
                'Live MQTT reading received: ' +
                new Date().toLocaleTimeString();

        }

    }

    // Actual LED state feedback from ESP8266
    if (topic === LED_STATE_TOPIC) {

        if (value === '1') {

            ledStatus.textContent = 'LED is ON';
            ledStatus.style.color = '#26965b';

        } else if (value === '0') {

            ledStatus.textContent = 'LED is OFF';
            ledStatus.style.color = '#555';

        }

    }

});


// Send LED command
function controlLED(command) {

    if (!client.connected) {

        commandStatus.textContent =
            'Cannot send command: MQTT is disconnected.';

        return;

    }

    client.publish(
        LED_COMMAND_TOPIC,
        command,
        { qos: 0 },
        function (error) {

            if (error) {

                commandStatus.textContent =
                    'Failed to send LED command.';

            } else {

                if (command === '1') {

                    commandStatus.textContent =
                        'ON command sent to ESP8266.';

                } else {

                    commandStatus.textContent =
                        'OFF command sent to ESP8266.';

                }

            }

        }
    );

}

</script>

</body>
</html>
