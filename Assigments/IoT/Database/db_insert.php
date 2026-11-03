
<?php

date_default_timezone_set('America/Los_Angeles');

require_once __DIR__ . '/../secure/config.php';

// Connect to database
$conn = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);

if ($conn->connect_error) {
    die("Database connection failed: " . $conn->connect_error);
}

$conn->set_charset("utf8mb4");

// Get values directly from URL parameters
$nodeId = $_GET['nodeId'] ?? '';
$nodeTemp = $_GET['nodeTemp'] ?? '';
$nodeHum = $_GET['nodeHum'] ?? '';
$timeReceived = $_GET['timeReceived'] ?? '';

// Check required parameters
if ($nodeId === '' || $nodeTemp === '') {
    die("Error: nodeId and nodeTemp are required.");
}

// Validate temperature
if (!is_numeric($nodeTemp) || $nodeTemp < -40 || $nodeTemp > 130) {
    die("Error: Invalid temperature.");
}

// Validate humidity if provided
if ($nodeHum !== '') {
    if (!is_numeric($nodeHum) || $nodeHum < 0 || $nodeHum > 100) {
        die("Error: Invalid humidity.");
    }
} else {
    $nodeHum = 0;
}

// Use current time if no timestamp was provided
if ($timeReceived === '') {
    $timeReceived = date('Y-m-d H:i:s');
}

// Check timestamp format
$date = DateTime::createFromFormat('!Y-m-d H:i:s', $timeReceived);

if (!$date || $date->format('Y-m-d H:i:s') !== $timeReceived) {
    die("Error: Invalid timeReceived format. Use YYYY-MM-DD HH:MM:SS.");
}

// Check if node is registered
$stmt = $conn->prepare(
    "SELECT node_name FROM sensor_register WHERE node_name = ?"
);

$stmt->bind_param("s", $nodeId);
$stmt->execute();
$stmt->store_result();

if ($stmt->num_rows === 0) {
    $stmt->close();
    $conn->close();
    die("Error: Node is not registered.");
}

$stmt->close();

// Check for duplicate reading
$stmt = $conn->prepare(
    "SELECT reading_id FROM sensor_data
     WHERE node_name = ? AND time_received = ?"
);

$stmt->bind_param("ss", $nodeId, $timeReceived);
$stmt->execute();
$stmt->store_result();

if ($stmt->num_rows > 0) {
    $stmt->close();
    $conn->close();
    die("Error: This reading already exists.");
}

$stmt->close();

// Insert sensor reading
$stmt = $conn->prepare(
    "INSERT INTO sensor_data
     (node_name, time_received, temperature, humidity)
     VALUES (?, ?, ?, ?)"
);

$stmt->bind_param(
    "ssdd",
    $nodeId,
    $timeReceived,
    $nodeTemp,
    $nodeHum
);

if ($stmt->execute()) {
    echo "Success: Data logged successfully.";
} else {
    echo "Error: " . $stmt->error;
}

$stmt->close();
$conn->close();

?>

