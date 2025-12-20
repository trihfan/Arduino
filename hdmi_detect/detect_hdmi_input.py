import requests
import RPi.GPIO as GPIO
import time

GPIO_PIN = 17

GPIO.setmode(GPIO.BCM)
GPIO.setup(GPIO_PIN, GPIO.OUT)

while True:
    try:
        r = requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22current-state%22,%22subcommand%22:%22average-color%22,%22instance%22:0%7D", timeout=1)
        data = r.json()
        print(data)
        signal = data.get("signal", False)

        GPIO.output(GPIO_PIN, GPIO.HIGH if signal else GPIO.LOW)

    except:
        GPIO.output(GPIO_PIN, GPIO.LOW)

    time.sleep(1)
