# ☀️ Smart PV Surplus Heating Control via IR Power Meter

A smart energy project that controls a heating element based on surplus PV energy, measured via an infrared (IR) smart meter using the SML protocol. The Wemos D1 Mini (ESP8266) reads data from the meter, sends it via MQTT, and activates a heating relay if feed-in exceeds 1.4 kW.

---

## 🔧 Features

- Reads **SML data** from IR smart meter
- Extracts:
  - Total energy consumption (kWh)
  - Current power consumption (W)
  - Feed-in power (W)
- Sends data via **MQTT**
- Controls a **heating element via relay** if feed-in > 1.4 kW

---

## ⚙️ Hardware

| Component         | Description                              |
|-------------------|------------------------------------------|
| ESP8266 (Wemos D1)| Microcontroller with Wi-Fi and GPIO      |
| IR Head           | IR sensor for SML (usually UART, 9600bps)|
| Power Meter       | e.g. EasyMeter, Hager, EMH, eHZ (SML)     |
| Relay Module (SSR)| For switching heating element (230V)     |
| Heating Element   | e.g. water heater, boiler, etc.          |

---

## 🔌 Wiring

```
Wemos D1 Mini        IR Head
--------------       ------------
D7 (GPIO13 / RX)  -> TX (IR Head)
GND              -> GND
3.3V             -> VCC (3.3V or 5V depending on IR head)

D1 (GPIO5)       -> Relay IN (heating control)
```

---

## 🧰 Software

### ⚠️ Prerequisites

- [Arduino IDE](https://www.arduino.cc/en/software) or PlatformIO
- Libraries to install:
  - `ESP8266WiFi`
  - `PubSubClient`
  - `SoftwareSerial`
  - `SmlReader` ([GitHub: Whandall/SmlReader](https://github.com/Whandall/SmlReader)) or alternative SML parser

### 🔍 Common OBIS Codes

| OBIS        | Description                  |
|-------------|------------------------------|
| `1-0:1.8.0` | Total consumption (kWh)      |
| `1-0:2.8.0` | Total feed-in (kWh)          |
| `1-0:16.7.0`| Current power (W)            |

---

## 🚀 Setup

1. Connect the IR head to the ESP as shown above.
2. Flash the provided Arduino sketch:
   - Adjust Wi-Fi and MQTT credentials in code
   - Use your meter's appropriate parser
3. Monitor serial output for debugging
4. MQTT topics:
   - `power/total_energy_kWh`
   - `power/current_consumption_W`
   - `power/feed_in_W`
5. Relay is activated (GPIO HIGH) when feed-in exceeds 1400 W

---

## 📡 MQTT Topics

| Topic                         | Description               |
|------------------------------|---------------------------|
| `power/total_energy_kWh`     | Total energy in kWh       |
| `power/current_consumption_W`| Instantaneous power in W  |
| `power/feed_in_W`            | Feed-in power (W, negative)|

---

## 📊 Visualization

Suggested tools:
- **Home Assistant** (with MQTT sensor)
- **Node-RED Dashboard**
- **Grafana + InfluxDB**

---

## ⚠️ Notes

- Feed-in power is often a **negative value** → watch for sign!
- Use a proper **relay (SSR)** and safe **AC protection**!
- IR head must align precisely with your meter’s IR diode.
- If you see no data: check baud rate (usually 9600), OBIS codes.

---

## 📚 References

- [SML Protocol on Wikipedia](https://en.wikipedia.org/wiki/Smart_Message_Language)
- [Volkszähler Project](https://www.volkszaehler.org/)
- [Whandall/SmlReader GitHub](https://github.com/Whandall/SmlReader)

---


## 📄 License

MIT License – use at your own risk!
