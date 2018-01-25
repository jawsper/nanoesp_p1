#ifndef _TELNETSERVER_H_
#define _TELNETSERVER_H_

#include <NanoESP.h>

class TelnetServer {
private:
    NanoESP* nanoesp;
    bool(*process_command)(const String&, String&);
    int id;

    bool connected = false;
    
    void process_message(const String&);

    void prompt() { print("> "); }
public:

    TelnetServer(const NanoESP&, bool(*)(const String&, String&));

    bool init(int port = 23);
    bool print(const String&);
    bool println(const String&);

    bool has_connection() { return connected; }

    void on_connected();
    void on_message(const String&);
    void on_closed();
};

#endif