<?php
// db_update.php - Place this inside public_html
// Handles incoming data updates via GET strings securely using CURL

// 1. Load Secure Connection Credentials
require_once __DIR__ . '/../secure/config.php'; 

$conn = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);
if ($conn->connect_error) {
    die("Database offline."); 
}
$conn->set_charset("utf8mb4");

// 2. Capture GET URL parameters
$readingId = isset($_GET['id']) ? (int)$_GET['id'] : 0;
$nodeTemp  = isset($_GET['nodeTemp']) ? trim($_GET['nodeTemp']) : '';
$nodeHum   = isset($_GET['nodeHum']) ? trim($_GET['nodeHum']) : ''; 

// Validate that minimum parameters exist
if (!$readingId || $nodeTemp === '') {
    die("Error: Missing required variables (id and nodeTemp are mandatory).\n");
}

// 3. Requirement 5: Data Constraints / Accepted Ranges Validation
if ((float)$nodeTemp < -40 || (float)$nodeTemp > 130) {
    die("Error: Temperature value falls outside allowed data range thresholds.\n");
}
if ($nodeHum !== '' && ((float)$nodeHum < 0 || (float)$nodeHum > 100)) {
    die("Error: Humidity value falls outside allowed data range thresholds.\n");
}

// 4. Secure Update Execution via Prepared Statements
// We assume your primary key column is named 'reading_id'
$updateQuery = "UPDATE `sensor_data` SET `temperature` = ?, `humidity` = ? WHERE `reading_id` = ?";
$stmtUpdate = $conn->prepare($updateQuery);
$stmtUpdate->bind_param("ddi", $nodeTemp, $nodeHum, $readingId);

if ($stmtUpdate->execute()) {
    if ($stmtUpdate->affected_rows > 0) {
        echo "Success: Record #$readingId updated successfully.\n";
    } else {
        echo "Notice: Record found, but no new values were changed or ID does not exist.\n";
    }
} else {
    echo "Error: Failed to process update: " . $conn->error . "\n";
}

$stmtUpdate->close();
$conn->close();
?>
