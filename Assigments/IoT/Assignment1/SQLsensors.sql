-- =========================================================================
--Create Your Own Sensory-Table 
--Course:EE470
--Heybee Esquer Cheno
-- =========================================================================

-- 1. DATABASE CLEANUP (Ensures a clean execution state)
DROP VIEW IF EXISTS sensor_combined;
DROP TABLE IF EXISTS sensor_data;
DROP TABLE IF EXISTS sensor_register;

-- Master Core Registration Table
CREATE TABLE sensor_register (
    node_name VARCHAR(10) NOT NULL,
    manufacturer VARCHAR(10) NOT NULL,
    longitude DECIMAL(15, 6) NOT NULL,
    latitude DECIMAL(15, 6) NOT NULL,
    PRIMARY KEY (node_name)
);

-- Continuous Metrics Logs Table with Foreign Key & Check Rules
CREATE TABLE sensor_data (
    reading_id INT AUTO_INCREMENT,
    node_name VARCHAR(10) NOT NULL,
    time_received DATETIME NOT NULL,
    temperature DECIMAL(6, 2) NOT NULL CHECK (temperature BETWEEN -10.00 AND 100.00),
    humidity DECIMAL(6, 2) NOT NULL CHECK (humidity BETWEEN 0.00 AND 100.00),
    PRIMARY KEY (reading_id),
    FOREIGN KEY (node_name) REFERENCES sensor_register(node_name)
);


-- Populate 5 distinct device configurations with distinct manufacturers
INSERT INTO sensor_register (node_name, manufacturer, longitude, latitude) VALUES
('node_1', 'Sony', -122.419400, 37.774900),
('node_2', 'Samsung', 139.691700, 35.689500),
('node_3', 'Bosch', 13.405000, 52.520000),
('node_4', 'Intel', -0.127800, 51.507400),
('node_5', 'Philips', 4.895200, 52.370200);

-- Populate 4 continuous readings per device spaced every 30 minutes
INSERT INTO sensor_data (node_name, time_received, temperature, humidity) VALUES
-- Transmission Blocks for Device Node 1
('node_1', '2022-10-01 11:00:00', 22.50, 45.00),
('node_1', '2022-10-01 11:30:00', 22.80, 44.50),
('node_1', '2022-10-01 12:00:00', 23.10, 43.00),
('node_1', '2022-10-01 12:30:00', 23.40, 42.10),

-- Transmission Blocks for Device Node 2
('node_2', '2022-10-01 11:00:00', 18.20, 60.00),
('node_2', '2022-10-01 11:30:00', 18.50, 59.00),
('node_2', '2022-10-01 12:00:00', 18.90, 57.50),
('node_2', '2022-10-01 12:30:00', 19.10, 56.00),

-- Transmission Blocks for Device Node 3
('node_3', '2022-10-01 11:00:00', 15.00, 75.00),
('node_3', '2022-10-01 11:30:00', 14.80, 76.20),
('node_3', '2022-10-01 12:00:00', 14.50, 78.00),
('node_3', '2022-10-01 12:30:00', 14.20, 80.00),

-- Transmission Blocks for Device Node 4
('node_4', '2022-10-01 11:00:00', 25.10, 30.50),
('node_4', '2022-10-01 11:30:00', 25.60, 29.00),
('node_4', '2022-10-01 12:00:00', 26.00, 28.50),
('node_4', '2022-10-01 12:30:00', 26.30, 27.00),

-- Transmission Blocks for Device Node 5
('node_5', '2022-10-01 11:00:00', 20.00, 50.00),
('node_5', '2022-10-01 11:30:00', 20.20, 50.50),
('node_5', '2022-10-01 12:00:00', 20.50, 51.00),
('node_5', '2022-10-01 12:30:00', 20.90, 51.50);

CREATE VIEW sensor_combined AS 
SELECT 
    sensor_register.node_name,
    sensor_register.manufacturer,
    sensor_register.longitude,
    sensor_register.latitude,
    sensor_data.time_received,
    sensor_data.temperature,
    sensor_data.humidity
FROM 
    sensor_register
INNER JOIN 
    sensor_data 
ON 
    sensor_register.node_name = sensor_data.node_name;

-- Render the abstraction structure output
SELECT * FROM sensor_combined;

-- =========================================================================
    COMPLIANCE TESTS
-- =========================================================================

-- CONSTRAINT TEST A: Range rules verify valid data arrays
-- (Will pass safely)
INSERT INTO sensor_data(node_name, time_received, temperature, humidity) 
VALUES('node_1', '2022-10-01 13:00:00', 24.50, 41.00);

-- SECURITY TEST B: Unregistered system validation (Foreign Key Rule Proof)
-- (Expected Outcome: BLOCK ENTRY / #1452 Error thrown)
-- INSERT INTO sensor_data (node_name, time_received, temperature, humidity) 
-- VALUES ('node_99', '2022-10-01 13:00:00', 21.00, 50.00);
