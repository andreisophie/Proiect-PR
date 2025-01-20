import paho.mqtt.client as mqtt
from time import sleep
import json
import random
from datetime import datetime

mqttc = mqtt.Client()

mqttc.connect(host="localhost", port=1883)
print("Connected to MQTT broker succesfully")
mqttc.loop_start()

locations = ["UPB", "Andrei", "UNIBuc"]
stations = ["RPi_1", "Gas", "Mongo"]

while True:
    topic = f"{random.choice(locations)}/{random.choice(stations)}"
    
    battery = random.randint(0, 100)
    humidity = random.randint(0, 100)
    temperature = random.uniform(0, 40)

    payload = {
        "BAT": battery,
        "HUMID" : humidity,
        "PRJ" : "SCD",
        "TMP" : temperature,
        "status": "OK" ,
        # "timestamp" : datetime.now().strftime("%Y-%m-%dT%H:%M:%S%z")
    }

    mqttc.publish(topic, payload=json.dumps(payload))
    print(f"Published message to topic {topic}")
    sleep(5)

mqttc.disconnect()
mqttc.loop_stop()