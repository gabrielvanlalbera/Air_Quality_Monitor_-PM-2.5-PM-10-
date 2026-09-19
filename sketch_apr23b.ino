#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <ArduCAM.h>
#include "memorysaver.h"
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>

/* ================= WIFI ================= */
const char* ap_ssid = "ESP32_AIR_MONITOR";
const char* ap_pass = "12345678";

WebServer server(80);

/* ================= TIME ================= */
unsigned long baseMillis = 0;
time_t baseEpoch = 0;

#define IST_OFFSET 19800

/* ================= CAMERA ================= */
#define CAM_CS 5
ArduCAM myCAM(OV2640, CAM_CS);

/* ================= SD ================= */
#define SD_CS   27
#define SD_SCK  14
#define SD_MISO 26
#define SD_MOSI 13

SPIClass SD_SPI(HSPI);

/* ================= PMS ================= */
#define PM_RX 16
#define PM_TX 17

/* ================= TIMING ================= */
#define PM_INTERVAL 10000
#define IMG_INTERVAL 60000

unsigned long lastPMTime = 0;
unsigned long lastImageTime = 0;

/* ================= GLOBAL ================= */
float lastPM25 = 0;
float lastPM10 = 0;

String currentDate = "";
String folder = "";
String csvFile = "";
String latestImage = "";

/* ================= TIME ================= */
time_t getNow()
{
  if (baseEpoch == 0) return 0;
  return baseEpoch + (millis() - baseMillis) / 1000 + IST_OFFSET;
}

String getTime()
{
  if (baseEpoch == 0) return "NOT_SET";

  time_t now = getNow();
  struct tm *t = localtime(&now);

  char buf[10];
  sprintf(buf, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);

  return String(buf);
}

String getDate()
{
  if (baseEpoch == 0) return "NO_DATE";

  time_t now = getNow();
  struct tm *t = localtime(&now);

  char buf[15];
  sprintf(buf, "%04d-%02d-%02d",
          t->tm_year + 1900,
          t->tm_mon + 1,
          t->tm_mday);

  return String(buf);
}

/* ================= PMS ================= */
bool readPMS(float &pm25, float &pm10)
{
  static uint8_t buf[32];

  while (Serial2.available() >= 32)
  {
    if (Serial2.read() == 0x42 && Serial2.peek() == 0x4D)
    {
      buf[0] = 0x42;
      buf[1] = Serial2.read();
      Serial2.readBytes(&buf[2], 30);

      uint16_t sum = 0;
      for (int i = 0; i < 30; i++) sum += buf[i];

      uint16_t checksum = (buf[30] << 8) | buf[31];

      if (sum == checksum)
      {
        pm25 = (buf[6] << 8) | buf[7];
        pm10 = (buf[8] << 8) | buf[9];
        return true;
      }
    }
  }
  return false;
}

/* ================= SD ================= */
void setupNewDay()
{
  if (baseEpoch == 0)
  {
    Serial.println("Time not set yet ❌");
    return;
  }

  currentDate = getDate();

  folder = "/" + currentDate;
  csvFile = folder + "/PM_" + currentDate + ".csv";

  Serial.println("Creating folder: " + folder);

  if (!SD.exists(folder))
  {
    if (SD.mkdir(folder))
      Serial.println("Folder created ✅");
    else
      Serial.println("Folder creation failed ❌");
  }

  if (!SD.exists(csvFile))
  {
    File f = SD.open(csvFile, FILE_WRITE);

    if (f)
    {
      f.println("TIME,PM2.5,PM10");
      f.close();
      Serial.println("CSV created ✅");
    }
    else
    {
      Serial.println("CSV creation failed ❌");
    }
  }
}

void logPM(float pm25, float pm10)
{
  if (csvFile == "")
  {
    Serial.println("CSV path not set ❌");
    return;
  }

  File f = SD.open(csvFile, FILE_APPEND);

  if (!f)
  {
    Serial.println("Failed to open CSV ❌");
    return;
  }

  f.print(getTime());
  f.print(",");
  f.print(pm25);
  f.print(",");
  f.println(pm10);

  f.close();

  Serial.println("PM Logged ✅");
}

/* ================= CAMERA ================= */
void captureImage()
{
  if (baseEpoch == 0)
  {
    Serial.println("Time not set → skipping image ❌");
    return;
  }

  String name = folder + "/IMG_" + getTime() + ".jpg";
  name.replace(":", "-");

  Serial.println("Saving image: " + name);

  latestImage = name;

  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.start_capture();

  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));

  uint32_t len = myCAM.read_fifo_length();

  if (len == 0)
  {
    Serial.println("Image size 0 ❌");
    return;
  }

  File img = SD.open(name, FILE_WRITE);

  if (!img)
  {
    Serial.println("Image open failed ❌");
    return;
  }

  myCAM.CS_LOW();
  myCAM.set_fifo_burst();

  uint8_t buf[256];

  while (len)
  {
    uint16_t n = min((uint32_t)256, len);

    for (int i = 0; i < n; i++)
      buf[i] = SPI.transfer(0x00);

    img.write(buf, n);
    len -= n;
  }

  myCAM.CS_HIGH();
  img.close();

  Serial.println("Image saved ✅");
}

/* ================= WEB ================= */
void handleRoot()
{
  String html = "<html><head><meta http-equiv='refresh' content='2'></head>";
  html += "<body style='background:black;color:#00ffcc;text-align:center'>";

  html += "<h2>ESP32 AIR MONITOR</h2>";
  html += "<p>Time: " + getTime() + "</p>";
  html += "<p>PM2.5: " + String(lastPM25) + "</p>";
  html += "<p>PM10: " + String(lastPM10) + "</p>";

  html += "<button onclick='syncTime()'>Sync Time</button>";

  if (latestImage != "")
    html += "<br><img src='/image' width='80%'>";

  html += "<script>";
  html += "function syncTime(){";
  html += "let epoch = Math.floor(Date.now()/1000);";
  html += "fetch('/setTime?epoch='+epoch);";
  html += "alert('Time Synced!');";
  html += "}";
  html += "</script>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleImage()
{
  File f = SD.open(latestImage);
  if (!f)
  {
    server.send(404, "text/plain", "No Image");
    return;
  }
  server.streamFile(f, "image/jpeg");
  f.close();
}

void handleSetTime()
{
  if (!server.hasArg("epoch"))
  {
    server.send(400, "text/plain", "Missing time");
    return;
  }

  baseEpoch = server.arg("epoch").toInt();
  baseMillis = millis();

  Serial.println("Time synced from phone (IST) ✅");

  setupNewDay();

  server.send(200, "text/plain", "Time Set");
}

/* ================= SETUP ================= */
void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println("BOOT START 🚀");

  Serial2.begin(9600, SERIAL_8N1, PM_RX, PM_TX);

  SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  delay(500);
  if (!SD.begin(SD_CS, SD_SPI))
  {
    Serial.println("SD FAIL ❌");
    while (1);
  }
  Serial.println("SD OK ✅");

  Wire.begin();
  SPI.begin(18, 19, 23, CAM_CS);

  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);

  Serial.println("AP Started ✅");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/image", handleImage);
  server.on("/setTime", handleSetTime);
  server.begin();

  Serial.println("SYSTEM READY 🚀");
}

/* ================= LOOP ================= */
void loop()
{
  server.handleClient();

  unsigned long now = millis();

  if (baseEpoch != 0 && getDate() != currentDate)
  {
    setupNewDay();
  }

  if (now - lastPMTime >= PM_INTERVAL)
  {
    lastPMTime = now;

    float pm25, pm10;

    if (readPMS(pm25, pm10))
    {
      lastPM25 = pm25;
      lastPM10 = pm10;

      Serial.print(getTime());
      Serial.print(" PM2.5=");
      Serial.print(pm25);
      Serial.print(" PM10=");
      Serial.println(pm10);

      Serial.println("Logging to: " + csvFile);

      if (baseEpoch != 0)
        logPM(pm25, pm10);
    }
  }

  if (now - lastImageTime >= IMG_INTERVAL)
  {
    lastImageTime = now;
    captureImage();
  }
}