#!/usr/bin/env python3
import RPi.GPIO as GPIO
import subprocess
import time

# Pin Configuration (BCM Mode) - SWAPPED TO PIN 27
LED_PIN = 27

def setup_hardware():
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    GPIO.setup(LED_PIN, GPIO.OUT)
    GPIO.output(LED_PIN, GPIO.LOW) # Start with LED turned off

setup_hardware()
print("Program started. Sniffing live pings on GPIO 27...")
print("Press Ctrl+C to terminate.")

# Launch tshark in live-buffering mode to sniff inbound ICMP Requests
cmd = ["sudo", "tshark", "-i", "wlan0", "-l", "-Y", "icmp.type == 8", "-T", "fields", "-e", "frame.len"]
process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)

try:
    while True:
        line = process.stdout.readline()
        if not line:
            break
        
        frame_len_str = line.strip()
        if frame_len_str.isdigit():
            total_len = int(frame_len_str)
            
            # Windows calculation mapping: 
            # Total Frame Size = 14 (Eth) + 20 (IP) + 8 (ICMP) + Payload
            
            if total_len == 52:   # Windows ping with payload of 10 (-l 10)
                print(f"Turning ON LED (Frame length: {total_len} bytes matched)")
                GPIO.output(LED_PIN, GPIO.HIGH)
                
            elif total_len == 62: # Windows ping with payload of 20 (-l 20)
                print(f"Turning OFF LED (Frame length: {total_len} bytes matched)")
                GPIO.output(LED_PIN, GPIO.LOW)
                
            else:                 # Any other standard ping size
                print(f"A PING message was just received! (Frame length: {total_len} bytes)")

except KeyboardInterrupt:
    print("\nProgram terminated by user.")
finally:
    process.terminate()
    GPIO.cleanup()
