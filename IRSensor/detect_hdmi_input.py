import requests
import RPi.GPIO as GPIO
import time

THRESHOLD = 33 # With current config, min for each color is 11
GPIO_PIN = 17

GPIO.setmode(GPIO.BCM)
GPIO.setup(GPIO_PIN, GPIO.OUT)

current_state = False
wanted_state = False
start_changed = 0
MIN_DELAY = 20

GPIO.output(GPIO_PIN, GPIO.LOW)

while True:
    try:
        # Get ambilight color state
        request_ambilight_led_color = requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22current-state%22,%22subcommand%22:%22average-color%22,%22instance%22:0%7D", timeout=1)

        # Get virtual color state, works even when ambilight is disabled
        request_virtual_led_color = requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22current-state%22,%22subcommand%22:%22average-color%22,%22instance%22:1%7D", timeout=1)

        data_ambilight_led_color = request_ambilight_led_color.json()
        data_virtual_led_color = request_virtual_led_color.json()

        total_ambilight_led_color = data_ambilight_led_color["info"].get("red", 0) + data_ambilight_led_color["info"].get("green", 0) + data_ambilight_led_color["info"].get("blue", 0)
        total_virtual_led_color = data_virtual_led_color["info"].get("red", 0) + data_virtual_led_color["info"].get("green", 0) + data_virtual_led_color["info"].get("blue", 0)

        has_input_virtual_led_color = total_virtual_led_color != THRESHOLD
        
        print(f"received {total_virtual_led_color}, threshold is {THRESHOLD}")

        if (has_input_virtual_led_color != wanted_state):
            start_changed = time.time()
            wanted_state = has_input_virtual_led_color

        print(f"current_state {current_state}, wanted_state {wanted_state}")

        can_change = (time.time() - start_changed) > MIN_DELAY

        # Turn on instantaneously but off could be a false positive
        if (wanted_state != current_state and (wanted_state or can_change)):
            current_state = wanted_state
            GPIO.output(GPIO_PIN, GPIO.HIGH if current_state else GPIO.LOW)

        # Turn led on or off
        if (total_ambilight_led_color == 0 and current_state):
            requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22componentstate%22,%22componentstate%22:%7B%22component%22:%22LEDDEVICE%22,%22state%22:true%7D%7D")
        if (total_ambilight_led_color > 0 and not current_state):
            requests.get("http://localhost:8090/json-rpc?request=%7B%22command%22:%22componentstate%22,%22componentstate%22:%7B%22component%22:%22LEDDEVICE%22,%22state%22:false%7D%7D")

    except:
        GPIO.output(GPIO_PIN, GPIO.LOW)

    time.sleep(0.2)
