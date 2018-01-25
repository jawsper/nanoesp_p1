#ifndef _INFLUXDB_H_
#define _INFLUXDB_H_

#include <NanoESP.h>

class InfluxDB {
private:
    NanoESP *esp;
    String host;
    int port;
    String database;
    int connection_id;

    String measurement;
    String tag_set;


public:
    String error;
    
    InfluxDB(NanoESP &esp, const String &host, int port, const String &database) 
    : esp(&esp), host(host), port(port), database(database), connection_id(-1), measurement(""), tag_set(""), error("") {}

    inline void setConnection(const String &host, uint16_t port) {
        this->host = host;
        this->port = port;
    }
    inline void setDatabase(const String &database) {
        this->database = database;
    }

    inline void setMeasurement(const String &measurement) {
        this->measurement = measurement;
    }

    inline void setTagSet(const String &tag_set) {
        this->tag_set = tag_set;
    }

    bool connect(int connection_id) {
        if(esp->newConnection(connection_id, TCP, host, port)) {
            this->connection_id = connection_id;
            return true;
        }
        return false;
    }
    void disconnect() {
        if(connection_id < 0) {
            return;
        }
        esp->closeConnection(this->connection_id);
        connection_id = -1;
    }

    // bool writeData(String tag, int value) {
    //  return this->writeData(tag, String(value), false);
    // }

    // bool writeData(String tag, float value) {
    //  return this->writeData(tag, String(value), false);
    // }

    // bool writeValue(String tag, String value, bool quote=true) {
    //  if(quote) {
    //      value = "\"" + value + "\"";
    //  }
    //  return writeData(tag, "value=" + value);
    // }

private:
    bool writeToSocket(const String &data);
public:
    bool writeData(const String &measurement, const String &tag_set, const String &field_set);

    inline bool writeFieldSet(const String &field_set) {
        return this->writeData(this->measurement, this->tag_set, field_set);
    }
};


#endif