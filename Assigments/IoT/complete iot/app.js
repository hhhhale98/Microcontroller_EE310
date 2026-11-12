$(document).ready(function () {

    let sensorChart = null;
    let currentRequest = null;

    function loadSensorChart() {

        // Get the selected sensor node
        let selectedNode = $("#node").val();

        if (!selectedNode) {
            selectedNode = "node_1";
        }

        // Update chart title
        $("#chartTitle").text("Sensor Node " + selectedNode);

        // Stop previous request if still running
        if (currentRequest) {
            currentRequest.abort();
        }

        // Get sensor readings from data.php
        currentRequest = $.ajax({
            url: "data.php",
            type: "GET",
            data: {
                node: selectedNode,
                _: new Date().getTime()
            },
            dataType: "json",
            cache: false,

            success: function (data) {

                if (!Array.isArray(data)) {
                    console.error("Invalid data returned:", data);
                    return;
                }

                // Get timestamps and temperature values
                let labels = data.map(function (reading) {
                    return reading.time_received;
                });

                let values = data.map(function (reading) {
                    return Number(reading.temperature);
                });

                // Get chart canvas
                let canvas = document.getElementById("mycanvas");

                if (!canvas) {
                    console.error("Chart canvas not found!");
                    return;
                }

                // Destroy previous chart before creating another
                if (sensorChart !== null) {
                    sensorChart.destroy();
                }

                // Create GREEN LINE chart
                sensorChart = new Chart(canvas, {

                    type: "line",

                    data: {
                        labels: labels,

                        datasets: [{
                            label: "Sensor Node " + selectedNode,

                            data: values,

                            // Green line
                            borderColor: "#4b8b3b",
                            borderWidth: 3,

                            // Green data points
                            backgroundColor: "#9bbb59",
                            pointBackgroundColor: "#9bbb59",
                            pointBorderColor: "#4b8b3b",
                            pointRadius: 4,
                            pointHoverRadius: 7,

                            // Line settings
                            fill: false,
                            tension: 0.3
                        }]
                    },

                    options: {

                        responsive: true,
                        maintainAspectRatio: false,

                        plugins: {
                            legend: {
                                display: true,
                                position: "top"
                            },

                            tooltip: {
                                callbacks: {
                                    label: function (context) {
                                        return "Temperature: " +
                                            context.parsed.y + " °F";
                                    }
                                }
                            }
                        },

                        scales: {

                            x: {
                                title: {
                                    display: true,
                                    text: "Time"
                                }
                            },

                            y: {
                                beginAtZero: true,

                                title: {
                                    display: true,
                                    text: "Temperature (°F)"
                                }
                            }

                        }
                    }
                });
            },

            error: function (xhr, status, error) {

                if (status !== "abort") {
                    console.error("Error loading sensor data:", error);
                    console.error(xhr.responseText);
                }
            }
        });
    }

    // Load chart when page opens
    loadSensorChart();

    // Reload chart when another node is selected
    $("#node").on("change", function () {
        loadSensorChart();
    });

});
