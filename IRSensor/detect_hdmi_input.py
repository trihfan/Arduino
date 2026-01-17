import requests
import RPi.GPIO as GPIO
import time
import sys
from datetime import datetime

GPIO_PIN = 17

GPIO.setmode(GPIO.BCM)
GPIO.setup(GPIO_PIN, GPIO.OUT)

# Add delay when turning off to rpevent false positives
current_state = False
wanted_state = False
start_changed = 0
MIN_DELAY = 5


with open(sys.argv[1], 'a') as log:
    log.write(f"[{datetime.now()}] Sensor start")


while True:
    try:
        request = requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22serverinfo%22%7D")
        data = request.json()

        for priority in data["info"]["priorities"]:
            if priority["componentId"] == "VIDEOGRABBER":
                active = priority.get("active", False)
                
                with open(sys.argv[1], 'a') as log:
                    log.write(f"[{datetime.now()}] video is {active}")

                if (active != wanted_state):
                    start_changed = time.time()
                    wanted_state = active

                can_change = (time.time() - start_changed) > MIN_DELAY

                # Turn on instantaneously but off could be a false positive
                if (wanted_state != current_state and (wanted_state or can_change)):
                    with open(sys.argv[1], 'a') as log:
                        log.write(f"[{datetime.now()}] state change to {wanted_state}")

                    current_state = wanted_state
                    GPIO.output(GPIO_PIN, GPIO.HIGH if current_state else GPIO.LOW)

                    # Turn led on or off
                    if active:
                        requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22componentstate%22,%22componentstate%22:%7B%22component%22:%22LEDDEVICE%22,%22state%22:true%7D%7D")
                    else:
                        requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22componentstate%22,%22componentstate%22:%7B%22component%22:%22LEDDEVICE%22,%22state%22:false%7D%7D")

                break

    except:
        GPIO.output(GPIO_PIN, GPIO.LOW)

    time.sleep(1)
