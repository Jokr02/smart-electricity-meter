#include <SoftwareSerial.h>
#include <sml/sml_file.h>
#include <sml/sml_parser.h>

SoftwareSerial smlSerial(D7, D6); // RX, TX
SmlParser parser;

uint8_t buffer[512]; // SML-Datenpuffer
size_t pos = 0;

float totalEnergy = 0.0;
float currentPower = 0.0;
float feedIn = 0.0;

void parseSml() {
  while (smlSerial.available()) {
    byte b = smlSerial.read();
    buffer[pos++] = b;
    if (pos >= sizeof(buffer)) pos = 0;

    if (parser.parse(buffer, pos)) {
      for (int i = 0; i < parser.getListSize(); i++) {
        SmlListEntry entry = parser.getListEntry(i);
        if (entry.obis == "1-0:1.8.0") {
          totalEnergy = entry.value / 10000.0;
        }
        if (entry.obis == "1-0:16.7.0") {
          currentPower = entry.value;
        }
        if (entry.obis == "1-0:2.8.0") {
          feedIn = -entry.value / 10000.0;
        }
      }
      pos = 0; // reset buffer
    }
  }
}
