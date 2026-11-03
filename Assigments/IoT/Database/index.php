
<?php

// ==========================================
// 1. SECURE DATABASE CONNECTION
// ==========================================

require_once __DIR__ . '/../secure/config.php';

$conn = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);

if ($conn->connect_error) {
    die("Database connection failed.");
}

$conn->set_charset("utf8mb4");


// ==========================================
// 2. DATABASE COLUMN NAMES
// ==========================================

$col_data_node = 'node_name';
$col_data_time = 'time_received';
$col_data_temp = 'temperature';
$col_data_hum  = 'humidity';


// ==========================================
// 3. GET REGISTERED SENSOR NODES
// ==========================================

$registerQuery = "
    SELECT *
    FROM sensor_register
    ORDER BY node_name ASC
";

$resultRegister = $conn->query($registerQuery);

if (!$resultRegister) {
    die("Unable to retrieve registered sensor nodes.");
}

$registered_nodes = [];

while ($row = $resultRegister->fetch_assoc()) {
    $registered_nodes[] = $row;
}


// Get valid node names
$valid_nodes = array_column($registered_nodes, 'node_name');


// ==========================================
// 4. GET SELECTED SENSOR NODE
// ==========================================

$target_node = isset($_GET['node'])
    ? trim($_GET['node'])
    : 'node_1';

// Only allow registered nodes
if (!in_array($target_node, $valid_nodes, true)) {

    $target_node = !empty($valid_nodes)
        ? $valid_nodes[0]
        : 'node_1';

}


// ==========================================
// 5. GET ALL RECEIVED SENSOR DATA
// ==========================================

$dataQuery = "
    SELECT *
    FROM sensor_data
    ORDER BY node_name ASC, time_received DESC
";

$resultData = $conn->query($dataQuery);

if (!$resultData) {
    die("Unable to retrieve sensor data.");
}

$received_data = [];

while ($row = $resultData->fetch_assoc()) {
    $received_data[] = $row;
}


// ==========================================
// 6. CALCULATE SENSOR STATISTICS
// ==========================================

$avg_temp = 0;
$avg_hum  = 0;
$max_temp = 0;
$min_hum  = 0;

$statsQuery = "
    SELECT
        AVG(temperature) AS avg_t,
        AVG(humidity) AS avg_h,
        MAX(temperature) AS max_t,
        MIN(humidity) AS min_h
    FROM sensor_data
    WHERE node_name = ?
";

$stmtStats = $conn->prepare($statsQuery);

if (!$stmtStats) {
    die("Unable to prepare statistics query.");
}

$stmtStats->bind_param("s", $target_node);

$stmtStats->execute();

$statsResult = $stmtStats->get_result()->fetch_assoc();

if ($statsResult && $statsResult['avg_t'] !== null) {

    $avg_temp = round((float)$statsResult['avg_t'], 1);

    $avg_hum = $statsResult['avg_h'] !== null
        ? round((float)$statsResult['avg_h'], 1)
        : 0;

    $max_temp = round((float)$statsResult['max_t'], 1);

    $min_hum = $statsResult['min_h'] !== null
        ? round((float)$statsResult['min_h'], 1)
        : 0;

}

$stmtStats->close();


// ==========================================
// 7. CLOSE DATABASE CONNECTION
// ==========================================

$conn->close();


// ==========================================
// 8. HTML ESCAPE HELPER
// ==========================================

function e($value) {
    return htmlspecialchars(
        (string)$value,
        ENT_QUOTES,
        'UTF-8'
    );
}

?>

<!DOCTYPE html>
<html lang="en">

<head>

    <meta charset="UTF-8">

    <meta name="viewport"
          content="width=device-width, initial-scale=1.0">

    <title>SSU IoT Lab</title>


    <!-- ==================================
         CSS STYLING
    =================================== -->

    <style>

        * {
            box-sizing: border-box;
        }

        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 40px 20px;
            background-color: #f9f9f9;
            color: #333;
        }

        .container {
            max-width: 1000px;
            margin: 0 auto;
            background: #ffffff;
            padding: 30px;
            border: 1px solid #ccc;
            box-shadow: 2px 2px 10px rgba(0,0,0,0.1);
            text-align: center;
        }

        h3 {
            text-align: left;
            margin-top: 30px;
            color: #444;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            margin-bottom: 20px;
        }

        th {
            background-color: #9bbb59;
            color: white;
            padding: 10px;
            border: 1px solid #7a943f;
            text-align: left;
        }

        td {
            padding: 10px;
            border: 1px solid #ccc;
            text-align: left;
            background-color: #fdfdfd;
        }

        tr:nth-child(even) td {
            background-color: #f2f6e9;
        }

        .avg-box {
            text-align: left;
            background-color: #f9f9f9;
            border: 1px dashed #9bbb59;
            padding: 15px;
            margin-top: 10px;
            line-height: 1.8;
            margin-bottom: 30px;
        }


        /* ==================================
           CHART CONTAINER
        ================================== */

        .chart-visual-wrapper {
            border: 1px solid #ccc;
            background: #fff;
            padding: 20px;
            margin: 20px 0;
            text-align: left;
            width: 100%;
        }

        .chart-control-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 15px;
            gap: 10px;
        }

        .node-select {
            padding: 5px 8px;
            font-size: 0.9em;
            font-weight: bold;
            border: 1px solid #ccc;
            border-radius: 4px;
            background: #fff;
            color: #333;
            cursor: pointer;
        }

        #chartTitle {
            font-weight: bold;
            font-size: 1.1em;
            color: #333;
            text-align: right;
        }

        #chart-container {
            width: 100%;
            height: 350px;
            position: relative;
        }


        /* ==================================
           LABORATORY TESTING PANEL
        ================================== */

        .test-panel {
            background: #f0f4f8;
            border: 1px solid #b0c4de;
            padding: 20px;
            text-align: left;
            margin-top: 40px;
        }

        .test-btn {
            background-color: #34495e;
            color: white;
            border: none;
            padding: 10px 15px;
            text-decoration: none;
            display: inline-block;
            border-radius: 4px;
        }

        .test-btn:hover {
            background-color: #2c3e50;
        }


        /* ==================================
           MOBILE RESPONSIVENESS
        ================================== */

        @media screen and (max-width: 600px) {

            body {
                padding: 10px;
            }

            .container {
                padding: 15px;
            }

            .chart-visual-wrapper {
                padding: 10px;
            }

            .chart-control-header {
                flex-wrap: wrap;
            }

            #chartTitle {
                font-size: 0.95em;
            }

            #chart-container {
                height: 300px;
            }

            table {
                font-size: 12px;
            }

            th, td {
                padding: 7px;
            }

        }

    </style>

</head>


<body>

<div class="container">


    <!-- ==================================
         DASHBOARD HEADER
    =================================== -->

    <p style="font-size: 1.5em; margin: 0;">
        Welcome to
    </p>

    <h1 style="font-size: 2.2em; font-weight: bold; margin: 0 0 25px 0;">
        SSU IoT Lab
    </h1>


    <!-- ==================================
         TABLE 1: REGISTERED SENSOR NODES
    =================================== -->

    <h3>Registered Sensor Nodes</h3>

    <table>

        <thead>

            <tr>
                <th>Name</th>
                <th>Manufacturer</th>
                <th>Location</th>
            </tr>

        </thead>

        <tbody>

            <?php foreach ($registered_nodes as $row): ?>

                <tr>

                    <td>
                        <?= e($row['node_name']) ?>
                    </td>

                    <td>
                        <?= e($row['manufacturer']) ?>
                    </td>

                    <td>
                        Long: <?= e($row['longitude']) ?>
                        <br>
                        Lat: <?= e($row['latitude']) ?>
                    </td>

                </tr>

            <?php endforeach; ?>

            <?php if (empty($registered_nodes)): ?>

                <tr>
                    <td colspan="3">No registered sensor nodes found.</td>
                </tr>

            <?php endif; ?>

        </tbody>

    </table>


    <!-- ==================================
         TABLE 2: DATA RECEIVED
    =================================== -->

    <h3>Data Received</h3>

    <table>

        <thead>

            <tr>
                <th>Node</th>
                <th>Time</th>
                <th>Sensor Readings</th>
            </tr>

        </thead>

        <tbody>

            <?php foreach ($received_data as $row): ?>

                <tr>

                    <td>
                        <?= e($row['node_name']) ?>
                    </td>

                    <td>
                        <?= e($row['time_received']) ?>
                    </td>

                    <td>
                        Temp: <?= e($row['temperature']) ?> °F
                        <br>
                        Hum: <?= e($row['humidity']) ?>%
                    </td>

                </tr>

            <?php endforeach; ?>

            <?php if (empty($received_data)): ?>

                <tr>
                    <td colspan="3">No sensor data received yet.</td>
                </tr>

            <?php endif; ?>

        </tbody>

    </table>


    <!-- ==================================
         STATISTICS
    =================================== -->

    <div class="avg-box">

        The Average Temperature for
        <strong><?= e($target_node) ?></strong> has been:

        <strong><?= e($avg_temp) ?> °F</strong>

        <br>

        The Average Humidity for
        <strong><?= e($target_node) ?></strong> has been:

        <strong><?= e($avg_hum) ?> %</strong>

        <br>

        The Max Temperature for
        <strong><?= e($target_node) ?></strong> has been:

        <strong><?= e($max_temp) ?> °F</strong>

        <br>

        The Minimum Humidity for
        <strong><?= e($target_node) ?></strong> has been:

        <strong><?= e($min_hum) ?> %</strong>

    </div>


    <!-- ==================================
         CHART.JS GRAPH AND NODE DROPDOWN
    =================================== -->

    <div class="chart-visual-wrapper">

        <div class="chart-control-header">

            <!-- SENSOR NODE SELECTION -->

            <form method="GET" action="index.php">

                <label for="node"
                       style="font-weight: bold; font-size: 0.9em;">

                    Select Node:

                </label>

                <select
                    id="node"
                    name="node"
                    class="node-select"
                    onchange="this.form.submit()">

                    <?php foreach ($valid_nodes as $nodeOpt): ?>

                        <option
                            value="<?= e($nodeOpt) ?>"
                            <?= ($nodeOpt === $target_node) ? 'selected' : '' ?>>

                            <?= e($nodeOpt) ?>

                        </option>

                    <?php endforeach; ?>

                </select>

            </form>


            <!-- SELECTED NODE TITLE -->

            <div id="chartTitle">

                Sensor Node <?= e($target_node) ?>

            </div>

        </div>


        <!-- CHART CANVAS -->

        <div id="chart-container">

            <canvas id="mycanvas"></canvas>

        </div>

    </div>


    <!-- ==================================
         LABORATORY TESTING TOOL
    =================================== -->

    <div class="test-panel">

        <h4>🛠️ Laboratory Testing Tool</h4>

        <p>
            Click the button below to automatically execute
            a test entry simulation into your database.
        </p>

        <a
            href="db_insert.php?nodeId=node_1&nodeTemp=76.4&nodeHum=52.1"
            target="_blank"
            class="test-btn">

            🚀 Send Simulation Data

        </a>

    </div>


</div>


<!-- ==================================
     JQUERY LIBRARY
=================================== -->

<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.6.0/jquery.min.js"></script>


<!-- ==================================
     CHART.JS LIBRARY
=================================== -->

<script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.7.1/chart.min.js"></script>


<!-- ==================================
     CUSTOM CHART JAVASCRIPT
=================================== -->

<script src="Chartjs/app.js?v=3"></script>


</body>

</html>
