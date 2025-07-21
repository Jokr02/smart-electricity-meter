String line;
while (smlSerial.available()) {
  char c = smlSerial.read();
  if (c == '\n') {
    if (line.startsWith("1-0:1.8.0")) {
      float value = line.substring(10).toFloat();
      totalEnergy = value;
    }
    if (line.startsWith("1-0:16.7.0")) {
      currentPower = line.substring(10).toFloat();
    }
    line = "";
  } else {
    line += c;
  }
}
