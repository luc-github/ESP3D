import serial
import time

# Configuration du port série
ser = serial.Serial(
    port='COM4',          # Port série Windows
    baudrate=115200,      # Vitesse en bauds
    timeout=1             # Timeout en secondes
)

def format_char(c):
    # Convertit un caractère en sa représentation lisible
    if c == b'\r':
        return '\\r'
    elif c == b'\n':
        return '\\n'
    else:
        return f"{c.decode('ascii', errors='replace')}({ord(c)})"

try:
    print("Lecture du port série COM4. Ctrl+C pour arrêter.")
    while True:
        if ser.in_waiting > 0:
            # Lire un caractère à la fois
            char = ser.read(1)
            print(format_char(char), end=' ', flush=True)
        time.sleep(0.01)  # Petit délai pour ne pas surcharger le CPU

except KeyboardInterrupt:
    print("\nArrêt du programme")
finally:
    ser.close()
    print("Port série fermé")