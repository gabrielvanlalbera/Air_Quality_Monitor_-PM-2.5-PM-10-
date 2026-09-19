# Air_Quality_Monitor_-PM-2.5-PM-10-
In this project i have quantified the amount of Particulate Matter 2.5 and 10 that are present in a single image and seperate them according to the levels of each of their contents into 3 categories :"Okay" , "Hazardous" and "Danger".  

I have also the use of an object for destructing the view of an image to study its various prediction levels using Machine Learning Algorithm, I have found some interesting results and can be further deployed to smartphone cameras. 

You might want to check it out. Hasta Luego amigo.


# 🌬️ Air Quality Analysis & PM2.5 / PM10 Prediction System

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Hardware](https://img.shields.io/badge/Hardware-ESP32%20%7C%20Arducam-blue.svg)]()
[![Platform](https://img.shields.io/badge/Deployment-Mobile%20App-brightgreen.svg)]()

An IoT and Machine Learning-based system designed to capture visual imagery and environmental sensor data to analyze, correlate, and predict particulate matter (**PM2.5** and **PM10**) concentrations. The system uses an **ESP32** microcontroller coupled with an **Arducam** module and particulate sensors to feed data into a predictive model designed for mobile application deployment.

---

## 📌 Key Highlights

- **Simultaneous Data Capture:** Images and sensor readings are captured synchronously at the exact same time every day over a continuous **30-day monitoring period**.
- **Visual & Sensor Data Correlation:** Correlates visual haze/obscuration captured via Arducam with physical PM2.5 and PM10 sensor values.
- **Weather Forecast Integration:** Incorporates meteorological parameters (humidity, temperature, wind speed, precipitation forecast) to refine particulate prediction accuracy.
- **Comparative Baseline Analysis:** Evaluates air pollution metrics **before** and **after** deploying the monitoring system to measure environmental trends and intervention impact.
- **Mobile-Ready Architecture:** Lightweight predictive model pipeline designed for seamless integration into smartphone applications (Flutter / React Native).

---

## 🖼️ System Overview & Setup

The hardware node relies on an ESP32 processing unit connected to an Arducam module alongside PM2.5 and PM10 dust sensors (e.g., SDS011 or PMS5003).

<Image src="image_agent_tag_2870193515202425032" alt="ESP32 and sensor hardware circuit layout diagram" caption="ESP32 Hardware & Sensor Setup Architecture" />

---

## 📊 30-Day Data Collection Methodology

To ensure rigorous dataset quality and eliminate time-of-day bias, data collection followed a strict automated schedule:

```text
       [ Scheduled Trigger ] ➔ (Daily at 12:00 PM for 30 Days)
                │
        ┌───────┴───────┐
        ▼               ▼
  [ Arducam ]     [ PM Sensors ] ───► [ Weather API Integration ]
  (Capture Image)  (PM2.5 & PM10)     (Humidity, Temp, Forecast)
        │               │                       │
        └───────┬───────┴───────────────────────┘
                ▼
  [ Synchronized Daily Data Record ] ───► [ Feature Extraction & ML Model ]
<img width="435" height="437" alt="mi" src="https://github.com/user-attachments/assets/44f1174a-19b9-4ff3-ad1c-f8a8631807eb" />
