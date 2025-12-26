import requests
import RPi.GPIO as GPIO
import time

THRESHOLD = 50
GPIO_PIN = 17

GPIO.setmode(GPIO.BCM)
GPIO.setup(GPIO_PIN, GPIO.OUT)

current_state = False
wanted_state = False
start_changed = 0
MIN_DELAY = 10

while True:
    try:
        r = requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22current-state%22,%22subcommand%22:%22average-color%22,%22instance%22:0%7D", timeout=1)
        data = r.json()
        red = data["info"].get("red", 0)
        green = data["info"].get("green", 0)
        blue = data["info"].get("blue", 0)
        total = red + green + blue
        has_input = (red + green + blue) > THRESHOLD
        print(f"received {total}, threshold is {THRESHOLD}")

        if (has_input != wanted_state):
            start_changed = time.time()
            wanted_state = has_input

        print(f"current_state {current_state}, wanted_state {wanted_state}")

        # Turn on instantaneously but off could be a false positive
        if (wanted_state != current_state and (wanted_state or (time.time() - start_changed) > MIN_DELAY)):
            current_state = wanted_state
            GPIO.output(GPIO_PIN, GPIO.HIGH if current_state else GPIO.LOW)
            if (wanted_state):
                requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22componentstate%22,%22componentstate%22:%7B%22component%22:%22LEDDEVICE%22,%22state%22:true%7D%7D")
                print(r.get())
            else:
                requests.get("http://localhost.43:8090/json-rpc?request=%7B%22command%22:%22componentstate%22,%22componentstate%22:%7B%22component%22:%22LEDDEVICE%22,%22state%22:false%7D%7D")
                print(r.get())

    except:
        GPIO.output(GPIO_PIN, GPIO.LOW)

    time.sleep(0.5)
