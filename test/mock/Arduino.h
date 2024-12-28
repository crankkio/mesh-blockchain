#pragma once
#include <cstdint>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstdio>
#include <cstdarg>

// Constants
#define WL_CONNECTED 3
#define WL_DISCONNECTED 6
#define HTTP_CODE_NO_CONTENT 204
#define HEX 16

// Arduino timing functions
inline unsigned long millis() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    ).count();
}

inline unsigned long micros() {
    using namespace std::chrono;
    return duration_cast<microseconds>(
        system_clock::now().time_since_epoch()
    ).count();
}

// Arduino String class needs more functionality than std::string
class String : public std::string {
public:
    String() : std::string() {}
    String(const char* str) : std::string(str) {}
    String(const std::string& str) : std::string(str) {}
    String(unsigned long num, int base = 10) {
        std::stringstream ss;
        if (base == HEX)
            ss << std::hex;
        ss << num;
        assign(ss.str());
    }
    
    // Arduino-specific String methods
    String substring(size_t from, size_t to) const {
        return substr(from, to - from);
    }
    
    String& replace(const char* from, const char* to) {
        size_t start_pos = 0;
        while((start_pos = find(from, start_pos)) != std::string::npos) {
            std::string::replace(start_pos, strlen(from), to);
            start_pos += strlen(to);
        }
        return *this;
    }

    int indexOf(const String& str) const {
        size_t pos = find(str);
        return pos == npos ? -1 : pos;
    }

    bool startsWith(const String& prefix) const {
        return rfind(prefix, 0) == 0;
    }

    // ArduinoJson compatibility
    size_t write(uint8_t c) {
        push_back(c);
        return 1;
    }

    size_t write(const uint8_t* s, size_t n) {
        append(reinterpret_cast<const char*>(s), n);
        return n;
    }
};

// Serial interface
class SerialClass {
public:
    void print(const char* str) {
        printf("%s", str);
    }
    
    void println(const char* str) {
        printf("%s\n", str);
    }
    
    void printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
};

// WiFi interface
class WiFiClass {
public:
    uint8_t status() { return WL_DISCONNECTED; }
};

// HTTPClient interface
class HTTPClient {
public:
    bool begin(const String& url) { return true; }
    void addHeader(const char* name, const char* value) {}
    void setTimeout(uint32_t timeout) {}
    int POST(const String& payload) { return HTTP_CODE_NO_CONTENT; }
    String getString() { return ""; }
    void end() {}
};

// Global instances
extern SerialClass Serial;
extern WiFiClass WiFi;
extern HTTPClient http;