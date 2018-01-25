#include "TelnetServer.h"

TelnetServer::TelnetServer(const NanoESP& nanoesp, bool(*process_command)(const String&, String&))
    : nanoesp((NanoESP*)&nanoesp), process_command(process_command), id(0) {

}

bool TelnetServer::init(int port) {
    if(nanoesp->startTcpServer(port)) {
        Serial.println(F("Telnet server started"));
        return true;
    }

    Serial.println(F("Telnet server failed"));
    return false;
}

bool TelnetServer::print(const String& data) {
    if(nanoesp->sendCom("AT+CIPSEND=" + String(id) + "," + String(data.length()), ">")) {
        nanoesp->print(data);
    }

    if(nanoesp->find("OK\r\n")) {
        return true;
    }
    return false;
}

bool TelnetServer::println(const String& data) {
    return this->print(data + String("\r\n"));
}

void TelnetServer::process_message(const String& message) {
    if(message.startsWith(F("exit")) || message == "\xff\xec" /* CTRL+D */) {
        nanoesp->closeConnection(id);
        on_closed();
        return;
    }

    String reply;
    if(this->process_command(message, reply))
        println(reply);
    prompt();
}

void TelnetServer::on_connected() {
    Serial.println(F("TelnetServer::on_connected"));
    println(F("Welcome to NanoESP_P1!"));
    prompt();
    connected = true;
}

void TelnetServer::on_message(const String& message) {
    Serial.print(F("TelnetServer::on_message "));
    Serial.println(message);

    process_message(message);
}

void TelnetServer::on_closed() {
    Serial.println(F("TelnetServer::on_closed"));
    connected = false;
}

