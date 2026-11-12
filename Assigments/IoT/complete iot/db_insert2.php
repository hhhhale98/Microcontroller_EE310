
<?php
// db_insert2.php
// Receives ESP8266 sensor data and stores it in:
// SensorRegister, SensorData, and SensorActivity.

// Use the same timezone as your ESP8266 project.
date_default_timezone_set('America/Los_Angeles');

header('Content-Type: application/json; charset=utf-8');

// Load database configuration from outside public_html.
require_once __DIR__ . '/../secure/config.php';

/**
 * Send a JSON response and stop the script.
 */
function respond($httpCode, $response)
{
    http_response_code($httpCode);
    echo json_encode($response, JSON_PRETTY_PRINT);
    exit;
}

$conn = null;

try {
    // Connect using Object-Oriented MySQLi.
    $conn = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);
    $conn->set_charset('utf8mb4');

    // --------------------------------------------------
    // 1. RECEIVE REGULAR URL PARAMETERS OR BASE64 DATA
    // --------------------------------------------------

    $data = [];

    // Regular URL format:
    // ?nodeId=node_1&distance=25.5&light=350&timeReceived=2026-11-12%2004:46:57

    if (
        isset($_GET['nodeId']) ||
        isset($_GET['distance']) ||
        isset($_GET['light'])
    ) {
        $data = $_GET;
    } else {
        // Base64 format: the encoded message is the query string.
        $rawQuery = $_SERVER['QUERY_STRING'] ?? '';

        // Remove any extra query parameters after the encoded message.
        $encodedMessage = explode('&', $rawQuery, 2)[0];

        // Decode URL-encoded characters before Base64 decoding.
        $encodedMessage = rawurldecode($encodedMessage);

        $decodedMessage = base64_decode($encodedMessage, true);

        if ($decodedMessage === false) {
            respond(400, [
                'status' => 'error',
                'message' => 'Invalid Base64 message.'
            ]);
        }

        // Convert decoded text into PHP parameters.
        parse_str($decodedMessage, $data);
    }

    // --------------------------------------------------
    // 2. EXTRACT SENSOR VALUES
    // --------------------------------------------------

    $nodeId = trim($data['nodeId'] ?? '');
    $distanceInput = $data['distance'] ?? null;
    $lightInput = $data['light'] ?? null;
    $timeReceived = trim($data['timeReceived'] ?? '');

    // All four fields are required.
    if (
        $nodeId === '' ||
        $distanceInput === null ||
        $lightInput === null ||
        $timeReceived === ''
    ) {
        respond(400, [
            'status' => 'error',
            'message' => 'Missing required fields: nodeId, distance, light, or timeReceived.'
        ]);
    }

    // --------------------------------------------------
    // 3. VALIDATE NODE NAME
    // --------------------------------------------------

    if (!preg_match('/^node_[A-Za-z0-9]+$/', $nodeId)) {
        respond(400, [
            'status' => 'error',
            'message' => 'Invalid node name.'
        ]);
    }

    // --------------------------------------------------
    // 4. VALIDATE DISTANCE
    // --------------------------------------------------

    if (!is_numeric($distanceInput)) {
        respond(400, [
            'status' => 'error',
            'message' => 'Distance must be numeric.'
        ]);
    }

    $distance = (float)$distanceInput;

    if (!is_finite($distance) || $distance < 0 || $distance > 400) {
        respond(400, [
            'status' => 'error',
            'message' => 'Distance must be between 0 and 400 cm.'
        ]);
    }

    // --------------------------------------------------
    // 5. VALIDATE LIGHT
    // --------------------------------------------------

    $lightString = (string)$lightInput;

    if (!preg_match('/^\d+$/', $lightString)) {
        respond(400, [
            'status' => 'error',
            'message' => 'Light must be a whole number.'
        ]);
    }

    $light = (int)$lightString;

    if ($light < 0 || $light > 1023) {
        respond(400, [
            'status' => 'error',
            'message' => 'Light must be between 0 and 1023.'
        ]);
    }

    // --------------------------------------------------
    // 6. VALIDATE TIMESTAMP
    // --------------------------------------------------

    // Expected format: YYYY-MM-DD HH:MM:SS
    $date = DateTime::createFromFormat('!Y-m-d H:i:s', $timeReceived);

    if (
        $date === false ||
        $date->format('Y-m-d H:i:s') !== $timeReceived
    ) {
        respond(400, [
            'status' => 'error',
            'message' => 'Invalid timestamp. Expected YYYY-MM-DD HH:MM:SS.'
        ]);
    }

    // --------------------------------------------------
    // 7. CHECK THAT THE NODE IS REGISTERED
    // --------------------------------------------------

    $checkNode = $conn->prepare(
        "SELECT node_name FROM SensorRegister WHERE node_name = ?"
    );

    $checkNode->bind_param("s", $nodeId);
    $checkNode->execute();

    $nodeResult = $checkNode->get_result();
    $registered = $nodeResult->num_rows > 0;

    $checkNode->close();

    if (!$registered) {
        respond(403, [
            'status' => 'error',
            'message' => 'Node is not registered.'
        ]);
    }

    // --------------------------------------------------
    // 8. INSERT READING AND UPDATE ACTIVITY COUNT
    // --------------------------------------------------

    $conn->begin_transaction();

    try {
        $insert = $conn->prepare(
            "INSERT INTO SensorData
             (node_name, time_received, distance, light)
             VALUES (?, ?, ?, ?)"
        );

        $insert->bind_param(
            "ssdi",
            $nodeId,
            $timeReceived,
            $distance,
            $light
        );

        $insert->execute();
        $insert->close();

        // Increase the count only after a new reading is inserted.
        $updateActivity = $conn->prepare(
            "UPDATE SensorActivity
             SET message_count = message_count + 1
             WHERE node_name = ?"
        );

        $updateActivity->bind_param("s", $nodeId);
        $updateActivity->execute();

        if ($updateActivity->affected_rows !== 1) {
            $updateActivity->close();
            throw new Exception('Could not update sensor activity count.');
        }

        $updateActivity->close();

        // Save both operations together.
        $conn->commit();

        respond(200, [
            'status' => 'success',
            'message' => 'Sensor reading stored successfully.',
            'nodeId' => $nodeId,
            'distance' => $distance,
            'light' => $light,
            'timeReceived' => $timeReceived
        ]);

    } catch (mysqli_sql_exception $e) {
        $conn->rollback();

        // MySQL error 1062 means a duplicate unique-key value.
        if ((int)$e->getCode() === 1062) {
            respond(200, [
                'status' => 'already_stored',
                'message' => 'This reading was already stored.',
                'nodeId' => $nodeId,
                'timeReceived' => $timeReceived
            ]);
        }

        throw $e;

    } catch (Exception $e) {
        $conn->rollback();
        throw $e;
    }

} catch (Throwable $e) {
    if ($conn instanceof mysqli) {
        try {
            $conn->rollback();
        } catch (Throwable $ignored) {
            // No active transaction to roll back.
        }
    }

    // Keep database details out of the public response.
    error_log('db_insert2.php error: ' . $e->getMessage());

    respond(500, [
        'status' => 'error',
        'message' => 'Database operation failed.'
    ]);
}

?>
