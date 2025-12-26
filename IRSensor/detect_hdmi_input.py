import requests
import RPi.GPIO as GPIO
import time

THRESHOLD = 33
GPIO_PIN = 17

GPIO.setmode(GPIO.BCM)
GPIO.setup(GPIO_PIN, GPIO.OUT)

current_state = False
wanted_state = False
start_changed = 0
MIN_DELAY = 10

while True:
    try:
        r0 = requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22current-state%22,%22subcommand%22:%22average-color%22,%22instance%22:0%7D", timeout=1)
        r1 = requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22current-state%22,%22subcommand%22:%22average-color%22,%22instance%22:1%7D", timeout=1)

        data0 = r0.json()
        data1 = r0.json()

        total0 = data0["info"].get("red", 0) + data0["info"].get("green", 0) + data0["info"].get("blue", 0)
        total1 = data1["info"].get("red", 0) + data1["info"].get("green", 0) + data1["info"].get("blue", 0)

        has_input0 = total0 > THRESHOLD
        has_input1 = total1 > THRESHOLD
        
        print(f"received {total1}, threshold is {THRESHOLD}")

        if (has_input1 != wanted_state):
            start_changed = time.time()
            wanted_state = has_input1

        print(f"current_state {current_state}, wanted_state {wanted_state}")

        can_change = (time.time() - start_changed) > MIN_DELAY

        # Turn on instantaneously but off could be a false positive
        if (wanted_state != current_state and (wanted_state or can_change)):
            current_state = wanted_state
            GPIO.output(GPIO_PIN, GPIO.HIGH if current_state else GPIO.LOW)

        # Turn led on or off
        if (total0 == 0 and current_state):
            requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22componentstate%22,%22componentstate%22:%7B%22component%22:%22LEDDEVICE%22,%22state%22:true%7D%7D")
        if (total0 > 0 and not current_state):
            requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22componentstate%22,%22componentstate%22:%7B%22component%22:%22LEDDEVICE%22,%22state%22:false%7D%7D")

    except:
        GPIO.output(GPIO_PIN, GPIO.LOW)

    time.sleep(0.5)
