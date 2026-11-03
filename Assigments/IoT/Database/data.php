
<?php

// ==========================================
// 1. SET JSON HEADER
// ==========================================

header('Content-Type: application/json; charset=utf-8');

// ==========================================
// 2. SECURE DATABASE CONNECTION
// ==========================================

require_once __DIR__ . '/../secure/config.php';

$conn = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);

if ($conn->connect_error) {

    http_response_code(500);

    echo json_encode([
        "error" => "Database connection failed"
    ], JSON_PRETTY_PRINT);

    exit;
}

$conn->set_charset("utf8mb4");

// ==========================================
// 3. GET SELECTED SENSOR NODE
// ==========================================

$node = isset($_GET['node'])
    ? trim($_GET['node'])
    : 'node_1';

// ==========================================
// 4. VERIFY THAT NODE IS REGISTERED
// ==========================================

$check = $conn->prepare(
    "SELECT node_name
     FROM sensor_register
     WHERE node_name = ?"
);

$check->bind_param("s", $node);
$check->execute();

$registered = $check->get_result()->num_rows > 0;

$check->close();

if (!$registered) {

    http_response_code(400);

    echo json_encode([
        "error" => "Invalid sensor node"
    ], JSON_PRETTY_PRINT);

    $conn->close();

    exit;
}

// ==========================================
// 5. RETRIEVE SENSOR DATA
// ==========================================

$query = "
    SELECT temperature, time_received
    FROM sensor_data
    WHERE node_name = ?
    ORDER BY time_received ASC
";

$stmt = $conn->prepare($query);

$stmt->bind_param("s", $node);

$stmt->execute();

$result = $stmt->get_result();

// ==========================================
// 6. STORE RESULTS IN AN ARRAY
// ==========================================

$data = [];

while ($row = $result->fetch_assoc()) {

    $data[] = [
        "time_received" => $row["time_received"],
        "temperature" => (float)$row["temperature"]
    ];

}

// ==========================================
// 7. CLOSE DATABASE CONNECTION
// ==========================================

$stmt->close();

$conn->close();

// ==========================================
// 8. OUTPUT FORMATTED JSON
// ==========================================

echo json_encode($data, JSON_PRETTY_PRINT);

?>
