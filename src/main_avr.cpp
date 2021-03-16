#ifdef ARDUINO_ARCH_AVR

#include <Arduino.h>
#include <NanoESP.h>

#include <SoftwareSerial.h>
#include <EEPROM.h>

#include "TelnetServer.h"
#include "DSMR4_Parser.h"
#include "InfluxDB.h"

#include "CRC16.h"
// #include "MemoryFree.h"

#include "secret.h"

#define PIN_ENABLE_P1 2
#define LED_WLAN 13


struct config_t {
    char influx_ip_address[16];
    uint16_t influx_port;
    char influx_database[16];

    char ssid[32];
    char psk[32];
} config;

bool parse_command(const String&, String&);

NanoESP nanoesp;
InfluxDB influx(nanoesp, INFLUXDB_IP, INFLUXDB_PORT, INFLUXDB_DATABASE);
DSMR4_Parser parser;
TelnetServer telnetServer(nanoesp, parse_command);

bool error = false;
String last_status = "";

bool load_config() {
    uint16_t correct_eeprom_crc;
    EEPROM.get(0, correct_eeprom_crc);
    EEPROM.get(2, config);
    uint16_t config_crc = CRC16_t(0, config);
    // return false;
    return config_crc == correct_eeprom_crc;
}

void save_config() {
    uint16_t config_crc = CRC16_t(0, config);
    EEPROM.put(0, config_crc);
    EEPROM.put(2, config);
}

void reset() {
    digitalWrite(PIN_ENABLE_P1, LOW);
    digitalWrite(LED_WLAN, LOW);
    if(!nanoesp.init()) {
        Serial.println(F("nanoesp init failed..."));
    }
    // Connect with WiFi
    Serial.print(F("Connecting to SSID \""));
    Serial.print(config.ssid);
    Serial.println(F("\""));
    while(!nanoesp.wifiConnected()) {
        if(nanoesp.configWifi(STATION, config.ssid, config.psk)) {
            break;
        } else {
            Serial.println(F("Wifi connection failed!"));
            return;
        }
    }
    Serial.println(F("Wifi connected!"));


    String ip, mac;
    nanoesp.getIpMac(ip, mac);
    Serial.print(F("IP: "));
    Serial.println(ip);
    Serial.print(F("MAC: "));
    Serial.println(mac);

    telnetServer.init();

    error = false;

    digitalWrite(PIN_ENABLE_P1, HIGH);
    digitalWrite(LED_WLAN, HIGH);

}

void setup() {
    pinMode(PIN_ENABLE_P1, OUTPUT);
    digitalWrite(PIN_ENABLE_P1, LOW);
    Serial.begin(115200);
    Serial.println(F("NanoESP_P1 init"));

    if(!load_config()) {
        Serial.println(F("EEPROM invalid!"));

        memset(&config, 0, sizeof(config));

        strcpy(config.influx_ip_address, INFLUXDB_IP);
        config.influx_port = INFLUXDB_PORT;
        strcpy(config.influx_database, INFLUXDB_DATABASE);

        strcpy(config.ssid, SSID);
        strcpy(config.psk, PASSWORD);


        save_config();
    } else {
        Serial.println(F("EEPROM valid!"));
    }

    Serial.println(F("====CONFIG===="));
    Serial.print(F("Influx IP: "));
    Serial.println(config.influx_ip_address);
    Serial.print(F("Influx Port: "));
    Serial.println(config.influx_port);
    Serial.print(F("Influx Database: "));
    Serial.println(config.influx_database);
    Serial.print(F("SSID: "));
    Serial.println(config.ssid);
    Serial.print(F("PSK: "));
    Serial.println(config.psk);
    Serial.println(F("=============="));

    influx.setConnection(String(config.influx_ip_address), config.influx_port);
    influx.setDatabase(String(config.influx_database));
    influx.setMeasurement("power");
    influx.setTagSet("host=power_esp");

    reset();
}

bool parse_command(const String& command, String& reply) {
    Serial.println("Command: \"" + command + "\"");

    if(command.startsWith(F("config"))) {
        reply = "Config:" \
            "\r\nSSID: " + String(config.ssid) +
            "\r\nPSK: " + String(config.psk) +
            "\r\nInflux config:" +
            "\r\nIP: " + String(config.influx_ip_address) +
            "\r\nPort: " + String(config.influx_port) +
            "\r\nDatabase: " + String(config.influx_database);
            
        return true;
    }/* else if(command.startsWith(F("AT"))) {
        nanoesp.println(command);
        reply = nanoesp.readString();
        return true;
    }*/ else if(command.startsWith(F("ip="))) {
        String ip = command.substring(3);
        memset(config.influx_ip_address, 0, sizeof(config.influx_ip_address));
        strcpy(config.influx_ip_address, ip.c_str());
        save_config();
        influx.setConnection(String(config.influx_ip_address), config.influx_port);
        reply = F("Config saved!");
        return true;
    } else if(command.startsWith(F("port="))) {
        int port = command.substring(5).toInt();
        config.influx_port = port;
        save_config();
        influx.setConnection(String(config.influx_ip_address), config.influx_port);
        reply = F("Config saved!");
        return true;
    } else if(command.startsWith(F("database="))) {
        String database = command.substring(9);
        memset(config.influx_database, 0, sizeof(config.influx_database));
        strcpy(config.influx_database, database.c_str());
        save_config();
        influx.setDatabase(String(config.influx_database));
        reply = F("Config saved!");
        return true;
    } else if(command.startsWith(F("ssid="))) {
        String ssid = command.substring(5);
        memset(config.ssid, 0, sizeof(config.ssid));
        strcpy(config.ssid, ssid.c_str());
        save_config();
        reply = F("Config saved!");
        return true;
    } else if(command.startsWith(F("psk="))) {
        String psk = command.substring(4);
        memset(config.psk, 0, sizeof(config.psk));
        strcpy(config.psk, psk.c_str());
        save_config();
        reply = F("Config saved!");
        return true;
    } else if(command.startsWith(F("test"))) {
        if(influx.writeData("test_measurement", "host=poweresp", "testing=\"time\"")) {
            reply = F("Data written successfully!");
        } else {
            reply = F("Writing to influx failed!");
        }
        return true;
    } else if(command.startsWith(F("status"))) {
        if(last_status.length() > 0) {
            reply = last_status;
        } else {
            reply = F("No status");
        }
        return true;
    } else if(command.startsWith(F("reset"))) {
        nanoesp.disconnectWifi();
        reset();
        return true;
    } else if(command.startsWith(F("help"))) {
        reply = F("help : this help function\r\nconfig : get current config\r\nip=<address> : sets influx ip\r\nport=<port> : sets influx port\r\ndatabase=<name> : sets influx database name\r\nstatus : get last status\r\nreset : reset Wifi connection\r\nssid=<ssid> : set SSID\r\npsk=<psk> : set PSK");
        return true;
    }

    return false;
}

void loop() {
    // Check for Wifi connection
    if(error) {
        if(!nanoesp.wifiConnected()) {
            Serial.println(F("Wifi not connected!"));
            reset();
            return;
        } else {
            error = false;
        }
    }

    if(nanoesp.available()) {
        String buffer = nanoesp.readStringUntil('\n');
        if(buffer.endsWith("\r")) buffer.remove(buffer.length() - 1);
        if(buffer.length() > 0) {
            Serial.print("RECV: ");
            Serial.println(buffer);
            // NOTE: I'm going with the assumption that the server will have connection ID 0.
            if(buffer == "0,CONNECT") {
                telnetServer.on_connected();
            } else if(buffer == "0,CLOSED") {
                telnetServer.on_closed();
            } else if(buffer.startsWith(F("+IPD,0,"))) {
                // int length = buffer.substring(7).parseInt();
                String message = buffer.substring(buffer.indexOf(F(":")) + 1);
                telnetServer.on_message(message);
            }
        }
    }

    if(Serial.available()) {
        while(Serial.available()) {
            String line = Serial.readStringUntil('\n');
            if(line.endsWith(F("\r"))) {
                line.remove(line.length() - 1);
            }

            if(line.startsWith(F("==!!"))) {
                String command = line.substring(4);

                String reply;
                if(parse_command(command, reply)) {
                    Serial.println(reply);
                }
                break;
            }

            int status = parser.addLine(line);
            if(status < 0) continue;
            if(status == 1) {
                Serial.println(F("====Data===="));
                Serial.println(parser.values.power_total_tariff_1);
                Serial.println(parser.values.power_total_tariff_2);
                Serial.println(parser.values.power_actual);
                Serial.println(parser.values.gas_total);

                if(parser.values.power_total_tariff_1.length() > 0 &&
                    parser.values.power_total_tariff_2.length() > 0 && 
                    parser.values.power_actual.length() > 0 &&
                    parser.values.gas_total.length() > 0) {
                    String field_set = 
                         "power_total_tariff_1=" + parser.values.power_total_tariff_1 + 
                        ",power_total_tariff_2=" + parser.values.power_total_tariff_2 +
                        ",power_actual=" + parser.values.power_actual + 
                        ",gas_total=" + parser.values.gas_total;
                    if(influx.writeFieldSet(field_set)) {
                        last_status = F("Writing data success!");
                        Serial.println(last_status);
                    } else {
                        last_status = influx.error;
                        Serial.println(last_status);
                        error = true;
                    }
                } else {
                    last_status = F("Invalid data!");
                    Serial.println(last_status);
                }
                break;
            }
        }
    }
}

#endif // ARDUINO_ARCH_AVR