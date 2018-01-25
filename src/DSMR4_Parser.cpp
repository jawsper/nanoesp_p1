#include <Arduino.h>
#include "DSMR4_Parser.h"
#include "CRC16.h"

void DSMR4_Parser::addBuffer(const String& data) {
    this->current_crc = CRC16(this->current_crc, data);
}

bool DSMR4_Parser::parseLine(const String& line) {
    // Serial.print("parsing: \"");
    // Serial.print(line);
    // Serial.println("\"");
    // Huidige tarief (int)
    if(line.startsWith(F("0-0:96.14.0"))) {
        return false;
    // Meterstand tarief 1 (kW)
    } else if(line.startsWith(F("1-0:1.8.1"))) {
        values.power_total_tariff_1 = getValue(line);
        return true;
    // Meterstand tarief 2 (kW)
    } else if(line.startsWith(F("1-0:1.8.2"))) {
        values.power_total_tariff_2 = getValue(line);
        return true;
    // Huidige verbruik (W)
    } else if(line.startsWith(F("1-0:1.7.0"))) {
        values.power_actual = getValue(line);
        return true;
    // Gas meterstand (m3)
    } else if(line.startsWith(F("0-1:24.2.1"))) {
        values.gas_total = getValue(line);
        return true;
    }
    return false;
}


/*
 * Returns:
 * -1: not ready
 *  0: crc failed
 *  1: crc passed
 */
int DSMR4_Parser::addLine(const String& line) {
    if(this->parsing) {
        if(line.startsWith("!")) {
            this->parsing = false;
            this->addBuffer("!");
            // crc check returns 1 if ok; 0 if fail
            // bool success = checkCRC(buffer) == line.substring(1);
            // Serial.print(F("CRC: "));
            Serial.println(this->current_crc, HEX);
            return 1;
        }
    } else {
        if(line.startsWith("/")) {
            this->parsing = true;
            // this->buffer = "";
            this->current_crc = 0x0000;
        } else {
            return -1;
        }
    }
    this->addBuffer(line + "\n");
    if(this->parseLine(line)) {
        // Serial.println("!!!PARSED!!!");
    }
    return -1;
}