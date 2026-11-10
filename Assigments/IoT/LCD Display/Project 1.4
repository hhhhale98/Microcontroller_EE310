import RPi.GPIO as GPIO
import time
import socket

# Configuración de Pines GPIO (Modo BCM)
LCD_RS = 25
LCD_E  = 24
LCD_D4 = 23
LCD_D5 = 18
LCD_D6 = 15
LCD_D7 = 14

LCD_WIDTH = 16    # Caracteres por línea
LCD_CHR = True
LCD_CMD = False

LCD_LINE_1 = 0x80 # Dirección RAM para la 1ra línea
LCD_LINE_2 = 0xC0 # Dirección RAM para la 2da línea

E_PULSE = 0.0005
E_DELAY = 0.0005

def lcd_init():
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    GPIO.setup(LCD_E, GPIO.OUT)
    GPIO.setup(LCD_RS, GPIO.OUT)
    GPIO.setup(LCD_D4, GPIO.OUT)
    GPIO.setup(LCD_D5, GPIO.OUT)
    GPIO.setup(LCD_D6, GPIO.OUT)
    GPIO.setup(LCD_D7, GPIO.OUT)

    lcd_byte(0x33, LCD_CMD)
    lcd_byte(0x32, LCD_CMD)
    lcd_byte(0x28, LCD_CMD) # 4-bit, 2 líneas, 5x7 puntos
    lcd_byte(0x0C, LCD_CMD) # Pantalla encendida, cursor apagado
    lcd_byte(0x06, LCD_CMD) # Mover cursor a la derecha
    lcd_byte(0x01, LCD_CMD) # Limpiar pantalla
    time.sleep(E_DELAY)

def lcd_byte(bits, mode):
    GPIO.output(LCD_RS, mode)
    # Bits altos
    GPIO.output(LCD_D4, False)
    GPIO.output(LCD_D5, False)
    GPIO.output(LCD_D6, False)
    GPIO.output(LCD_D7, False)
    if bits&0x10==0x10: GPIO.output(LCD_D4, True)
    if bits&0x20==0x20: GPIO.output(LCD_D5, True)
    if bits&0x40==0x40: GPIO.output(LCD_D6, True)
    if bits&0x80==0x80: GPIO.output(LCD_D7, True)
    lcd_toggle_enable()
    # Bits bajos
    GPIO.output(LCD_D4, False)
    GPIO.output(LCD_D5, False)
    GPIO.output(LCD_D6, False)
    GPIO.output(LCD_D7, False)
    if bits&0x01==0x01: GPIO.output(LCD_D4, True)
    if bits&0x02==0x02: GPIO.output(LCD_D5, True)
    if bits&0x04==0x04: GPIO.output(LCD_D6, True)
    if bits&0x08==0x08: GPIO.output(LCD_D7, True)
    lcd_toggle_enable()

def lcd_toggle_enable():
    time.sleep(E_DELAY)
    GPIO.output(LCD_E, True)
    time.sleep(E_PULSE)
    GPIO.output(LCD_E, False)
    time.sleep(E_DELAY)

def lcd_string(message, line):
    message = message.ljust(LCD_WIDTH, " ")
    lcd_byte(line, LCD_CMD)
    for i in range(LCD_WIDTH):
        lcd_byte(ord(message[i]), LCD_CHR)

def get_ip_address():
    # Intenta obtener la IP activa conectándose temporalmente a una dirección externa
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(('8.8.8.8', 80))
        IP = s.getsockname()[0]
    except Exception:
        IP = 'No Network Connection'
    finally:
        s.close()
    return IP

# Ejecución Principal en Bucle
try:
    lcd_init()
    while True:
        ip_addr = get_ip_address()
        lcd_string("My IP Address:", LCD_LINE_1)
        lcd_string(ip_addr, LCD_LINE_2)
        time.sleep(2) # Actualiza cada 2 segundos
except KeyboardInterrupt:
    pass
finally:
    GPIO.cleanup()
