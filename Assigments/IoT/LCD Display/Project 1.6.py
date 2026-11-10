#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time
import socket
import uuid

# Configuración de Pines GPIO (Modo BCM)
LCD_RS = 25
LCD_E  = 24
LCD_D4 = 23
LCD_D5 = 18
LCD_D6 = 15
LCD_D7 = 14

# Pin del Botón (Pulsador)
BUTTON_PIN = 22

LCD_WIDTH = 16    # Caracteres por línea
LCD_CHR = True
LCD_CMD = False

LCD_LINE_1 = 0x80 # Dirección RAM 1ra línea
LCD_LINE_2 = 0xC0 # Dirección RAM 2da línea

E_PULSE = 0.0005
E_DELAY = 0.0005

def lcd_init():
    """Inicializa la pantalla LCD en modo de 4 bits."""
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    GPIO.setup(LCD_E, GPIO.OUT)
    GPIO.setup(LCD_RS, GPIO.OUT)
    GPIO.setup(LCD_D4, GPIO.OUT)
    GPIO.setup(LCD_D5, GPIO.OUT)
    GPIO.setup(LCD_D6, GPIO.OUT)
    GPIO.setup(LCD_D7, GPIO.OUT)
    
    # Configurar pin del botón con resistencia Pull-Up interna
    GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)

    lcd_byte(0x33, LCD_CMD)
    lcd_byte(0x32, LCD_CMD)
    lcd_byte(0x28, LCD_CMD) # 4-bit, 2 líneas, 5x7 puntos
    lcd_byte(0x0C, LCD_CMD) # Pantalla encendida, cursor apagado
    lcd_byte(0x06, LCD_CMD) # Incrementar cursor
    lcd_byte(0x01, LCD_CMD) # Limpiar pantalla
    time.sleep(E_DELAY)

def lcd_byte(bits, mode):
    """Envía un byte de datos a la pantalla LCD."""
    GPIO.output(LCD_RS, mode)
    # Bits altos
    GPIO.output(LCD_D4, False)
    GPIO.output(LCD_D5, False)
    GPIO.output(LCD_D6, False)
    GPIO.output(LCD_D7, False)
    if bits & 0x10 == 0x10: GPIO.output(LCD_D4, True)
    if bits & 0x20 == 0x20: GPIO.output(LCD_D5, True)
    if bits & 0x40 == 0x40: GPIO.output(LCD_D6, True)
    if bits & 0x80 == 0x80: GPIO.output(LCD_D7, True)
    lcd_toggle_enable()
    # Bits bajos
    GPIO.output(LCD_D4, False)
    GPIO.output(LCD_D5, False)
    GPIO.output(LCD_D6, False)
    GPIO.output(LCD_D7, False)
    if bits & 0x01 == 0x01: GPIO.output(LCD_D4, True)
    if bits & 0x02 == 0x02: GPIO.output(LCD_D5, True)
    if bits & 0x04 == 0x04: GPIO.output(LCD_D6, True)
    if bits & 0x08 == 0x08: GPIO.output(LCD_D7, True)
    lcd_toggle_enable()

def lcd_toggle_enable():
    time.sleep(E_DELAY)
    GPIO.output(LCD_E, True)
    time.sleep(E_PULSE)
    GPIO.output(LCD_E, False)
    time.sleep(E_DELAY)

def lcd_string(message, line):
    """Envía una cadena de texto formateada a una línea de la LCD."""
    message = message.ljust(LCD_WIDTH, " ")
    lcd_byte(line, LCD_CMD)
    for i in range(LCD_WIDTH):
        lcd_byte(ord(message[i]), LCD_CHR)

def get_ip_address():
    """Obtiene la IP activa del sistema."""
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(('8.8.8.8', 80))
        IP = s.getsockname()[0]
    except Exception:
        IP = 'No Connection'
    finally:
        s.close()
    return IP

def get_mac_address():
    """Obtiene la dirección MAC en formato estándar (XX:XX:XX:XX:XX:XX)."""
    mac = ':'.join(['{:02x}'.format((uuid.getnode() >> ele) & 0xff) for ele in range(0, 8*6, 8)][::-1])
    return mac.upper()

# Bucle principal de ejecución
try:
    lcd_init()
    while True:
        # Leer el estado del botón (False/0 significa PRESIONADO debido al Pull-Up)
        if GPIO.input(BUTTON_PIN) == GPIO.LOW:
            mac_addr = get_mac_address()
            # Como la dirección MAC tiene 17 caracteres, usaremos dos líneas para que se lea completa
            # Cortamos los primeros 12 caracteres para la línea 1 y los últimos 5 para la línea 2
            lcd_string("MAC: " + mac_addr[0:11], LCD_LINE_1)
            lcd_string(mac_addr[11:], LCD_LINE_2)
        else:
            ip_addr = get_ip_address()
            lcd_string("My IP Address:", LCD_LINE_1)
            lcd_string(ip_addr, LCD_LINE_2)
            
        time.sleep(0.2) # Muestreo rápido para detectar la pulsación del botón al instante
except KeyboardInterrupt:
    pass
finally:
    GPIO.cleanup()
