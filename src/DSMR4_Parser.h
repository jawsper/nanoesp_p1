#ifndef _DSMR4_PARSER_H_
#define _DSMR4_PARSER_H_

struct values_t
{
    String power_total_tariff_1;
    String power_total_tariff_2;
    String power_actual;
    String gas_total;
};

class DSMR4_Parser
{
private:
    // String buffer;
    unsigned int current_crc;
public:
    values_t values;
    bool parsing = false;
private:
    inline String getValue(const String &s) {
        String val = getString(s);
        val = val.substring(0, val.indexOf('*'));
        // Serial.print(F("VAL: "));
        // Serial.println(val);
        return val;
    }
    // inline int getInt(const String &s) {
    //     String val = getString(s);
    // }

    inline String getString(const String &s) {
        int start = s.lastIndexOf('(') + 1;
        int end = s.lastIndexOf(')');
        return s.substring(start, end);
    }

    void addBuffer(const String &data);

public:
    bool parseLine(const String &line);

    int addLine(const String &line);
};

#endif