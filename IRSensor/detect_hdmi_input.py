import requests
import RPi.GPIO as GPIO
import time

GPIO_PIN = 17

GPIO.setmode(GPIO.BCM)
GPIO.setup(GPIO_PIN, GPIO.OUT)

#GPIO.output(GPIO_PIN, GPIO.LOW)

while True:
    try:
        request = requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22serverinfo%22%7D")
        data = request.json()

        for priority in data["info"]["priorities"]:
            if priority["componentId"] == "VIDEOGRABBER":
                active = priority.get("active", False)
                print(f"state is {active}")
                GPIO.output(GPIO_PIN, GPIO.HIGH if active else GPIO.LOW)

                # Turn led on or off
                if active:
                    requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22componentstate%22,%22componentstate%22:%7B%22component%22:%22LEDDEVICE%22,%22state%22:true%7D%7D")
                else:
                    requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22componentstate%22,%22componentstate%22:%7B%22component%22:%22LEDDEVICE%22,%22state%22:false%7D%7D")
                break

    except:
        GPIO.output(GPIO_PIN, GPIO.LOW)

    time.sleep(1)
