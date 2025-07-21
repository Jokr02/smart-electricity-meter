#include <SmlReader.h>
#include <SoftwareSerial.h>

SoftwareSerial smlSerial(D7, D6); // RX, TX
SmlReader sml(smlSerial);

float totalEnergy = 0.0;
float currentPower = 0.0;
float feedIn = 0.0;

void parseSml() {
  if (sml.available()) {
    SmlDataEntry* entry;
    while ((entry = sml.readEntry()) != nullptr) {
      if (entry->obis == 0x0100010800FF) { // 1-0:1.8.0 -> Total energy (kWh)
        totalEnergy = entry->value / 10000.0; // usually in Wh
      }
      if (entry->obis == 0x0100100700FF) { // 1-0:16.7.0 -> current power (W)
        currentPower = entry->value;
      }
      if (entry->obis == 0x0100020800FF) { // 1-0:2.8.0 -> Feed-in (kWh)
        feedIn = -entry->value / 10000.0;
      }
    }
  }
}
