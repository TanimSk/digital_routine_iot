from flask import Flask
import datetime
import requests
import pytz
import json


app = Flask(__name__)

@app.route("/update")
def home():

    url = ""

    response = requests.get(url).json()

    bst = pytz.timezone('Asia/Dhaka')
    current_time = datetime.datetime.now(bst)

    humidity = str(response['main']['humidity']) + '%'
    temp = str(response['main']['temp'] - 273.16)[:4] + "°C"
    weather_type = response['weather'][0]['main']


    return {
        "time": {
            "H": current_time.hour if current_time.hour < 12 else current_time.hour - 12,
            "M": current_time.minute,
            "S": current_time.second
        },
        "date": current_time.strftime("%a, %d"),
        "temp": temp,
        "humidity": humidity,
        "weather_type": weather_type,
        "task" : check_task(current_time)
    }


def check_task(time_obj):
    with open("tasks.json", 'r') as f:
        json_data = json.load(f)

    day_name = time_obj.strftime("%A")
    time_hour = time_obj.hour
    time_min = time_obj.minute

    for mins in range(6):
        task = json_data[day_name].get(f"{time_hour}:{time_min-mins}")
        print(day_name, time_hour ,time_min-mins)
        if not task == None:
            return task
    
    return ""