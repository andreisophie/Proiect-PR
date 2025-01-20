from time import sleep
import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient
from re import match
import json
from datetime import datetime, timedelta
import ssl

user_data = {}

def read_user_data():
    print("Refreshing user data...")
    with open("/app/data/keycards.txt", "r") as f:
        lines = f.read().splitlines()
        for line in lines:
            split_line = line.split()
            user_data[split_line[0]] = {"name": split_line[1], "access": int(split_line[2]), "inside": False}


def on_message(client, userdata, message):    
    # Check that topic is valid
    if message.topic.endswith("/access"):
        return
    
    # Print message to stdout
    print(f"Received a message by topic [{message.topic}]")
    print(f"Payload: {message.payload.decode()}")
    
    # Parse message
    split_topic = message.topic.split("/")
    location = split_topic[0]
    station = split_topic[1]
    
    payload = message.payload.decode()
    payload = payload.strip().replace(" ", "-")
    name = user_data[payload]['name']
    print(payload)

    inout = ""

    if (payload in user_data):
        if (user_data[payload]["access"]):
            mqttc.publish(f"{location}/{station}/access", "granted")
            print(f"Access granted to {name}")
            if user_data[payload]["inside"]:
                inout = "out"
                user_data[payload]["inside"] = False
            else:
                inout = "in"
                user_data[payload]["inside"] = True
        else:
            mqttc.publish(f"{location}/{station}/access", "denied")
            inout = "denied"
            print(f"Access denied to {name}")
    else:
        mqttc.publish(f"{location}/{station}/access", "denied")
        print(f"Access denied to unknown user with ID {payload}")
        inout = "unknown"

    timestamp = datetime.now() - timedelta(seconds = 1)
    print(timestamp.strftime("%Y-%m-%dT%H:%M:%S%z"))
    data = []
    
    # Create data point
    data.append({
        "measurement": f"{location}.{station}",
        "tags": { "inout": inout, "name": name },
        "time": timestamp.strftime("%Y-%m-%dT%H:%M:%S%z"),
        "fields": { "inout": inout, "name": name }
    })
    print(f"{location}.{station}.{name} {inout}")

    # Write data to InfluxDB
    influxdbc.write_points(data)
        
            
print("Starting adapter...")

read_user_data()

# Connect to InfluxDB
influxdbc = InfluxDBClient('influxdb', 8086)
databases = influxdbc.get_list_database()
print("Databases: ", databases)
if "proiect-pr2" not in [db["name"] for db in databases]:
    print("Creating database proiect-pr2")
    influxdbc.create_database("proiect-pr2")
influxdbc.switch_database("proiect-pr2")

# Connect to MQTT broker
mqttc = mqtt.Client()
mqttc.username_pw_set("proiect-pr", "difficult")
mqttc.tls_set("/app/data/CA.crt", tls_version=ssl.PROTOCOL_TLSv1_2)
mqttc.on_message = on_message
mqttc.connect("mqtt_broker", port=8883)
mqttc.subscribe("#")
mqttc.loop_forever()
