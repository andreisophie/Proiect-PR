from time import sleep
# import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient
from re import match
import json
from datetime import datetime, timedelta
import ssl
from random import choice

user_data = {}

def read_user_data():
    print("Refreshing user data...")
    with open("/app/data/keycards.txt", "r") as f:
        lines = f.read().splitlines()
        for line in lines:
            split_line = line.split()
            user_data[split_line[0]] = {"name": split_line[1], "access": int(split_line[2]), "inside": False}
            
print("Starting adapter...")

# read_user_data()

# Connect to InfluxDB
influxdbc = InfluxDBClient('influxdb', 8086)
databases = influxdbc.get_list_database()
print("Databases: ", databases)
if "proiect-pr" not in [db["name"] for db in databases]:
    influxdbc.create_database("proiect-pr")
influxdbc.switch_database("proiect-pr")

today = datetime.now().date()

names = ["Lion", "Man", "Fish", "CEO", "Thief", "Fired"]

for day_diff in range(1, 14):
    day = today - timedelta(days=day_diff)
    for i in range(0, 100):
        location = "andrei"
        station = "scan1"
        name = choice(names)
        inout = choice(["in", "out", "denied", "unknown"])
        timestamp = datetime(day.year, day.month, day.day, choice(range(8, 20)), choice(range(0, 60)), choice(range(0, 60)))
        print(f"{location} {name} {timestamp}")

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