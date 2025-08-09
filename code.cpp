\
/*
  Itron ACE3000 Typ 260 (IEC 62056-21 Mode C-a) -> MQTT (HiveMQ ohne TLS)
  - Pins Wemos D1 mini: RX=D7(GPIO13), TX=D6(GPIO12)
  - 300 baud, 7E1
  - Hysterese-Steuerung für Heizpatrone am RELAY_PIN
  - MQTT unverschlüsselt (Port 1883)
*/

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoOTA.h>

// === Pins (Wemos D1 mini) ===
// RX vom ZÄHLER -> ESP D6 (GPIO12)
// TX zum ZÄHLER -> ESP D7 (GPIO13)
#define RX_PIN D6
#define TX_PIN D7

// Falls dein IR-Kopf invertierte Pegel liefert -> auf true setzen
#define INVERT_SERIAL false

// ======= WLAN =======
const char* WIFI_SSID = "TP-LINK_A32AEC";
const char* WIFI_PASS = "51914691";

// ======= MQTT (HiveMQ ohne TLS) =======
const char* MQTT_HOST   = "6ae771c4cb604a779b7284e9e68022e0.s2.eu.hivemq.cloud"; // z.B. broker.hivemq.com oder deine IP
const uint16_t MQTT_PORT= 8883;
const char* MQTT_USER   = "JohannesK1208";   // optional
const char* MQTT_PASS   = "ggWyR3jBBb9Y9uG";   // optional

// ===== Topics =====
const char* T_IMPORT_KWH    = "home/pv/Bezug_kWh";
const char* T_EXPORT_KWH    = "home/pv/Einspeisung_Gesamt_kWh";
const char* T_NET_W         = "home/pv/Durchschnittsleistung_kWh";
const char* T_SERIAL        = "home/pv/Seriennummer";
const char* T_STATUS        = "home/pv/status";
const char* T_HEATER_STATE  = "home/pv/Patrone_Status";
const char* T_IMPORT_W     = "home/pv/Leistung_Import_W";
const char* T_EXPORT_W     = "home/pv/Leistung_Export_W";

// ===== Relais / Steuerung =====
#define RELAY_PIN D1
#define RELAY_ON_THRESHOLD   -1400
#define RELAY_OFF_THRESHOLD   -200


// === Root CA für HiveMQ ===
static const char ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFBTCCAu2gAwIBAgIQS6hSk/eaL6JzBkuoBI110DANBgkqhkiG9w0BAQsFADBP
MQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFy
Y2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBYMTAeFw0yNDAzMTMwMDAwMDBa
Fw0yNzAzMTIyMzU5NTlaMDMxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBF
bmNyeXB0MQwwCgYDVQQDEwNSMTAwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK
AoIBAQDPV+XmxFQS7bRH/sknWHZGUCiMHT6I3wWd1bUYKb3dtVq/+vbOo76vACFL
YlpaPAEvxVgD9on/jhFD68G14BQHlo9vH9fnuoE5CXVlt8KvGFs3Jijno/QHK20a
/6tYvJWuQP/py1fEtVt/eA0YYbwX51TGu0mRzW4Y0YCF7qZlNrx06rxQTOr8IfM4
FpOUurDTazgGzRYSespSdcitdrLCnF2YRVxvYXvGLe48E1KGAdlX5jgc3421H5KR
mudKHMxFqHJV8LDmowfs/acbZp4/SItxhHFYyTr6717yW0QrPHTnj7JHwQdqzZq3
DZb3EoEmUVQK7GH29/Xi8orIlQ2NAgMBAAGjgfgwgfUwDgYDVR0PAQH/BAQDAgGG
MB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATASBgNVHRMBAf8ECDAGAQH/
AgEAMB0GA1UdDgQWBBS7vMNHpeS8qcbDpHIMEI2iNeHI6DAfBgNVHSMEGDAWgBR5
tFnme7bl5AFzgAiIyBpY9umbbjAyBggrBgEFBQcBAQQmMCQwIgYIKwYBBQUHMAKG
Fmh0dHA6Ly94MS5pLmxlbmNyLm9yZy8wEwYDVR0gBAwwCjAIBgZngQwBAgEwJwYD
VR0fBCAwHjAcoBqgGIYWaHR0cDovL3gxLmMubGVuY3Iub3JnLzANBgkqhkiG9w0B
AQsFAAOCAgEAkrHnQTfreZ2B5s3iJeE6IOmQRJWjgVzPw139vaBw1bGWKCIL0vIo
zwzn1OZDjCQiHcFCktEJr59L9MhwTyAWsVrdAfYf+B9haxQnsHKNY67u4s5Lzzfd
u6PUzeetUK29v+PsPmI2cJkxp+iN3epi4hKu9ZzUPSwMqtCceb7qPVxEbpYxY1p9
1n5PJKBLBX9eb9LU6l8zSxPWV7bK3lG4XaMJgnT9x3ies7msFtpKK5bDtotij/l0
GaKeA97pb5uwD9KgWvaFXMIEt8jVTjLEvwRdvCn294GPDF08U8lAkIv7tghluaQh
1QnlE4SEN4LOECj8dsIGJXpGUk3aU3KkJz9icKy+aUgA+2cP21uh6NcDIS3XyfaZ
QjmDQ993ChII8SXWupQZVBiIpcWO4RqZk3lr7Bz5MUCwzDIA359e57SSq5CCkY0N
4B6Vulk7LktfwrdGNVI5BsC9qqxSwSKgRJeZ9wygIaehbHFHFhcBaMDKpiZlBHyz
rsnnlFXCb5s8HKn5LsUgGvB24L7sGNZP2CX7dhHov+YhD+jozLW2p9W4959Bz2Ei
RmqDtmiXLnzqTpXbI+suyCsohKRg6Un0RC47+cpiVwHiXZAW+cn8eiNIjqbVgXLx
KPpdzvvtTnOPlC7SQZSYmdunr3Bf9b77AiC/ZidstK36dRILKz7OA54=
-----END CERTIFICATE-----
)EOF";

// ====== Globale Objekte/States ======
SoftwareSerial meterSerial;  // wird in setup() mit allen Parametern geöffnet
std::unique_ptr<BearSSL::WiFiClientSecure> tlsClient(new BearSSL::WiFiClientSecure());
PubSubClient mqtt(*tlsClient);

bool heatingOn = false;
unsigned long lastPollMs = 0;
const unsigned long pollIntervalMs = 120000;

bool   havePrev = false;
double prev_import_kWh = 0.0;
double prev_export_kWh = 0.0;
unsigned long prev_time_ms = 0;

// ====== WLAN/MQTT ======
void wifi_setup() {
  Serial.printf("[WIFI] Verbinde mit %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  for (int i=0; i<60 && WiFi.status()!=WL_CONNECTED; ++i) { delay(500); Serial.print("."); }
  Serial.println();
  if (WiFi.status()==WL_CONNECTED) {
    Serial.printf("[WIFI] OK, IP=%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("[WIFI] Verbindung fehlgeschlagen (offline).");
  }
}

void mqtt_setup() {
  auto *ta = new BearSSL::X509List(ROOT_CA);
  tlsClient->setTrustAnchors(ta);
  tlsClient->setBufferSizes(4096, 1024);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
}

void mqtt_ensure() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (mqtt.connected()) return;

  Serial.printf("[MQTT] Verbinde zu %s:%u (TLS)\n", MQTT_HOST, MQTT_PORT);
  String cid = "ace3000-" + String(ESP.getChipId(), HEX);
  if (mqtt.connect(cid.c_str(), MQTT_USER, MQTT_PASS, T_STATUS, 0, true, "offline")) {
    Serial.println("[MQTT] verbunden.");
    mqtt.publish(T_STATUS, "online", true);
  } else {
    Serial.printf("[MQTT] Fehler rc=%d\n", mqtt.state());
  }
}

void pubFloat(const char* topic, double v, uint8_t dec=3, bool retain=false) {
  char buf[24]; dtostrf(v, 0, dec, buf);
  mqtt.publish(topic, buf, retain);
}
void pubText(const char* topic, const String& s, bool retain=false) {
  mqtt.publish(topic, s.c_str(), retain);
}

// ====== Helpers: Byte-Klassen & Sanitizer ======
static inline bool isPrintable7bit(char c) {
  return c >= 0x20 && c <= 0x7E;
}
String sanitizePrintable(const String& in) {
  String out; out.reserve(in.length());
  for (size_t i=0;i<in.length();++i) {
    char c = in[i];
    if (isPrintable7bit(c) || c=='\t') out += c;
  }
  return out;
}

// CRLF-Zeile lesen, CR/LF entfernen; Timeout in ms (leer bei Timeout)
String readLine(unsigned long timeout_ms) {
  String s; unsigned long start = millis();
  while (millis() - start < timeout_ms) {
    if (meterSerial.available()) {
      char c = meterSerial.read();
      if (c == '\n') break;
      if (c != '\r') s += c;
    } else {
      delay(1);
    }
  }
  return s;
}

// OBIS "x.y.z(value*unit)" -> Zahl extrahieren
bool parseObisValue(const String& line, const char* obis, double& outVal) {
  if (!line.startsWith(obis)) return false;
  int p1 = line.indexOf('(');
  if (p1 < 0) return false;
  int p2 = line.indexOf('*', p1 + 1);
  if (p2 < 0) p2 = line.indexOf(')', p1 + 1);
  if (p2 < 0) return false;
  outVal = line.substring(p1 + 1, p2).toFloat();
  return true;
}

// Auf Ident warten, /?!-Echo überspringen und echte Herstellerzeile akzeptieren
bool waitForIdent(String& ident, unsigned long timeout_ms = 4000) {
  unsigned long deadline = millis() + timeout_ms;
  ident = "";
  while ((long)(deadline - millis()) > 0) {
    String line = readLine(300);
    if (line.length()==0) continue;
    String clean = sanitizePrintable(line);
    if (clean == "/?!") continue;       // Echo von uns selbst verwerfen
    if (clean.length()>1 && clean[0]=='/') { ident = clean; return true; } // echte Herstellerzeile
  }
  return false;
}

// Auf STX (0x02) warten
bool waitForSTX(unsigned long timeout_ms = 4000) {
  unsigned long deadline = millis() + timeout_ms;
  while ((long)(deadline - millis()) > 0) {
    while (meterSerial.available()) {
      uint8_t b = meterSerial.read();
      if (b == 0x02) return true; // STX
    }
    delay(1);
  }
  return false;
}

// ====== Zähler auslesen & veröffentlichen ======
void doPoll() {
  Serial.println("[IEC] Starte Auslesung");
  while (meterSerial.available()) meterSerial.read(); // Puffer leeren

  // Init senden
  meterSerial.print("/?!\r\n");
  Serial.println("[IEC] Init '/?!' gesendet");

  // Auf Hersteller-Ident warten (Echo ignorieren)
  String ident;
  if (!waitForIdent(ident, 5000)) {
    Serial.println("[WARN] Keine Hersteller-Ident erhalten (nur Echo?).");
    return;
  }
  Serial.print("[ID ] "); Serial.println(ident);

  // ACK "000" (Baud bleibt 300)
  delay(300);
  meterSerial.write(0x06);
  meterSerial.print("000\r\n");
  Serial.println("[IEC] ACK '000' gesendet");

  // Auf STX warten
  if (!waitForSTX(5000)) {
    Serial.println("[WARN] Kein STX empfangen – Zähler hat Liste nicht gestartet.");
    return;
  }
  Serial.println("[IEC] STX empfangen, lese Liste...");

  // Datensätze bis '!' einlesen
  double import_kWh = NAN, export_kWh = NAN;
  String serialNo = "", statusFF = "", eventC50 = "";
  bool gotBang = false, haveImport = false, haveExport = false;

  while (true) {
    String raw  = readLine(3000);
    String line = sanitizePrintable(raw);

    if (line.length() == 0) break;

    if (line == "!") {
      gotBang = true;
      Serial.println("[END] Ende des Datensatzes");
      break;
    }

    Serial.print("[DATA] "); Serial.println(line);

    double v;
    if (parseObisValue(line, "1.8.0", v)) { import_kWh = v; haveImport = true; }
    else if (parseObisValue(line, "2.8.0", v)) { export_kWh = v; haveExport = true; }
    else if (line.startsWith("C.1("))  { int p1=line.indexOf('('),p2=line.indexOf(')',p1+1); if(p1>0&&p2>p1) serialNo=line.substring(p1+1,p2); }
    else if (line.startsWith("F.F("))  { int p1=line.indexOf('('),p2=line.indexOf(')',p1+1); if(p1>0&&p2>p1) statusFF=line.substring(p1+1,p2); }
    else if (line.startsWith("C.5.0(")){ int p1=line.indexOf('('),p2=line.indexOf(')',p1+1); if(p1>0&&p2>p1) eventC50=line.substring(p1+1,p2); }
  }

  // Nur vollständige Frames akzeptieren
  if (!(gotBang && haveImport && haveExport)) {
    Serial.println("[WARN] Unvollständiger Frame – Ignoriere diesen Durchlauf.");
    return;
  }

  // Zusammenfassung
  Serial.println("---- Zusammenfassung ----");
  if (serialNo.length())      { Serial.print("Serial     : "); Serial.println(serialNo); }
  if (!isnan(import_kWh))     { Serial.print("Import 1.8.0: "); Serial.print(import_kWh, 1); Serial.println(" kWh"); }
  if (!isnan(export_kWh))     { Serial.print("Export 2.8.0: "); Serial.print(export_kWh, 1); Serial.println(" kWh"); }
  if (statusFF.length())      { Serial.print("Status F.F : "); Serial.println(statusFF); }
  if (eventC50.length())      { Serial.print("Event C.5.0: "); Serial.println(eventC50); }

  // Durchschnitts-Netzleistung aus ΔkWh zwischen Frames
  if (!isnan(import_kWh) && !isnan(export_kWh)) {
    bool importChanged = (import_kWh != prev_import_kWh);
    bool exportChanged = (export_kWh != prev_export_kWh);

    if (havePrev && (importChanged || exportChanged)) {
      unsigned long now = millis();
      unsigned long dt_ms = now - prev_time_ms;
      double d_import = import_kWh - prev_import_kWh;
      double d_export = export_kWh - prev_export_kWh;
      double net_kWh  = d_import - d_export;          // + = Netzbezug, - = Einspeisung
      double avg_W = (dt_ms > 0) ? (net_kWh * 3600000.0 / (double)dt_ms) : NAN;

// ... nach Berechnung von avg_W:
double import_W = NAN;
double export_W = NAN;

if (dt_ms > 0) {
  // d_import und d_export sind in kWh; -> W umrechnen
  import_W = (d_import > 0) ? (d_import * 3600000.0 / (double)dt_ms) : 0.0;
  export_W = (d_export > 0) ? (d_export * 3600000.0 / (double)dt_ms) : 0.0;
}

// Debug
Serial.print("Import_W: ");
if (!isnan(import_W)) { Serial.print(import_W, 1); Serial.print(" W, "); } else Serial.print("n/a, ");
Serial.print("Export_W: ");
if (!isnan(export_W)) { Serial.print(export_W, 1); Serial.println(" W"); } else Serial.println("n/a");

// Hysterese (wie gehabt) nutzt weiterhin avg_W (Netto, +Bezug / -Einspeisung)
if (!isnan(avg_W)) {
  if (!heatingOn && avg_W < RELAY_ON_THRESHOLD) {
    digitalWrite(RELAY_PIN, HIGH);
    heatingOn = true;
    Serial.println("[CTRL] Heater ON (avg export sufficient)");
  } else if (heatingOn && avg_W > RELAY_OFF_THRESHOLD) {
    digitalWrite(RELAY_PIN, LOW);
    heatingOn = false;
    Serial.println("[CTRL] Heater OFF (avg export too low)");
  }
}

// MQTT publish (nur wenn verbunden)
mqtt_ensure();
if (mqtt.connected()) {
  // Energiestände (retain = true)
  pubFloat(T_IMPORT_KWH, import_kWh, 3, true);
  pubFloat(T_EXPORT_KWH, export_kWh, 3, true);

  // Leistungen (retain = false, es sind Momentanwerte)
  if (!isnan(avg_W))    pubFloat(T_NET_W,    avg_W,    1, false);   // Netto (+Bezug / -Einspeisung)
  if (!isnan(import_W)) pubFloat(T_IMPORT_W, import_W, 1, false);   // >= 0
  if (!isnan(export_W)) pubFloat(T_EXPORT_W, export_W, 1, false);   // >= 0

  if (serialNo.length()) pubText(T_SERIAL, serialNo, true);
  pubText(T_HEATER_STATE, heatingOn ? "ON" : "OFF", true);
  mqtt.loop();
}


      Serial.print("ΔImport: "); Serial.print(d_import, 4); Serial.print(" kWh, ");
      Serial.print("ΔExport: "); Serial.print(d_export, 4); Serial.print(" kWh, ");
      Serial.print("Δt: "); Serial.print(dt_ms); Serial.println(" ms");
      Serial.print("Avg Net Power: ");
      if (!isnan(avg_W)) { Serial.print(avg_W, 1); Serial.println(" W"); }
      else { Serial.println("n/a"); }

      // Hysterese (negativ = Einspeisung)
      if (!isnan(avg_W)) {
        if (!heatingOn && avg_W < RELAY_ON_THRESHOLD) {
          digitalWrite(RELAY_PIN, HIGH);
          heatingOn = true;
          Serial.println("[CTRL] Heater ON (avg export sufficient)");
        } else if (heatingOn && avg_W > RELAY_OFF_THRESHOLD) {
          digitalWrite(RELAY_PIN, LOW);
          heatingOn = false;
          Serial.println("[CTRL] Heater OFF (avg export too low)");
        }
      }

      // MQTT publish
      mqtt_ensure();
      if (mqtt.connected()) {
        pubFloat(T_IMPORT_KWH, import_kWh, 3, true);
        pubFloat(T_EXPORT_KWH, export_kWh, 3, true);
        if (!isnan(avg_W)) pubFloat(T_NET_W, avg_W, 1, false);
        if (serialNo.length()) pubText(T_SERIAL, serialNo, true);
        pubText(T_HEATER_STATE, heatingOn ? "ON" : "OFF", true);
        mqtt.loop();
      }
    } else if (!havePrev) {
      Serial.println("[INFO] Erste Messung – noch keine Durchschnittsleistung.");
    } else {
      Serial.println("[INFO] Keine Registeränderung – Durchschnittsleistung nicht berechnet.");
    }

    prev_import_kWh = import_kWh;
    prev_export_kWh = export_kWh;
    prev_time_ms = millis();
    havePrev = true;
  }

  Serial.println("-------------------------");
}


// ====== Setup / Loop ======
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== ACE3000 → MQTT (TLS) ===");
  Serial.printf("Pins: RX=%d (D6), TX=%d (D7), 300 baud, 7E1, invert=%s\n",
                RX_PIN, TX_PIN, INVERT_SERIAL ? "true" : "false");

  // SoftwareSerial robust initialisieren (wie im funktionierenden Test!)
  meterSerial.begin(300, SWSERIAL_7E1, RX_PIN, TX_PIN, INVERT_SERIAL, 256);
  delay(300);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  Serial.println("[RELAY] OFF");

  wifi_setup();
    ArduinoOTA.setHostname("ACE3000"); // frei wählbarer Name
  ArduinoOTA.onStart([]() {
    Serial.println("[OTA] Update gestartet");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\n[OTA] Update fertig");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[OTA] Fortschritt: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Fehler[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth fehlgeschlagen");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin fehlgeschlagen");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Verbindung fehlgeschlagen");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Datenempfang fehlgeschlagen");
    else if (error == OTA_END_ERROR) Serial.println("End fehlgeschlagen");
  });
  ArduinoOTA.begin();
  Serial.println("[OTA] Bereit");

  mqtt_setup();

  // Sofort eine erste Auslesung beim Start
  doPoll();

  Serial.println("[INFO] Setup fertig – wechsle in 15s-Intervall.");
  lastPollMs = millis();
}

void loop() {
  ArduinoOTA.handle();
  if (millis() - lastPollMs >= pollIntervalMs) {
    lastPollMs = millis();
    doPoll();
  }
  if (mqtt.connected()) mqtt.loop();
    

}
