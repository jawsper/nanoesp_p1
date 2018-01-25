#include "InfluxDB.h"

bool InfluxDB::writeToSocket(const String& data) {
    // Serial.println(data.length());
    if(this->esp->sendCom("AT+CIPSEND=" + String(this->connection_id) + "," + String(data.length()), ">")) {
        this->esp->print(data);
        String response = esp->readString();
        Serial.println(response);
        return true;
    }
    return false;
}

bool InfluxDB::writeData(const String &measurement, const String &tag_set, const String &field_set) {
    if(!this->connect(1)) {
        Serial.println(F("Error connecting to influx!"));
        error = "CIPSTART failed!";
        return false;
    }


    int content_length = measurement.length() + 1 + tag_set.length() + 1 + field_set.length();
    int header_length = 15 + database.length() + 11 
        + 6 + host.length() + 1 + String(port).length() + 2 
        + 16 + String(content_length).length() + 23;

    Serial.println(header_length + content_length);

    esp->print(F("AT+CIPSEND="));
    esp->print(connection_id);
    esp->print(F(","));
    esp->println(header_length + content_length);
    if(!esp->findUntil(">", "ERROR")) {
        error = "CIPSEND failed!";
        return false;
    }
    esp->print(F("POST /write?db="));
    esp->print(database);
    esp->print(F(" HTTP/1.1\r\nHost: "));
    esp->print(host);
    esp->print(":");
    esp->print(port);
    esp->print(F("\r\nContent-Length: "));
    esp->print(content_length);
    esp->print(F("\r\nConnection: close\r\n\r\n"));
    esp->print(measurement + "," + tag_set + " " + field_set);

    // String data = "POST /write?db=" + this->database + " HTTP/1.1\r\nHost: " + this->host + ":" + this->port + "\r\n";
    // data += "Content-Length: " + String(content_length) + "\r\nConnection: close\r\n\r\n";
    // data += measurement + "," + tag_set + " " + field_set;
    // Serial.println(data.length());

    // bool error = !this->writeToSocket(data);
    // if(error) {
    //     Serial.println(F("Error sending data to influx!"));
    // }

    int length;
    if(esp->recvData(this->connection_id, length)) {
        char buffer[length];
        esp->find(":");
        esp->readBytes(buffer, length);
        Serial.println(String(buffer));
    }
    this->disconnect();
    return true;
}