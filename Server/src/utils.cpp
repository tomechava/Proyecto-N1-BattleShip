#include "../include/utils.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <fstream>

void logWithTimestamp(const std::string& msg) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "[" << std::put_time(std::localtime(&now_c), "%T") << "] " << msg << std::endl;
}

void logToFile(const std::string& file, const std::string& message) {
    std::ofstream out(file, std::ios::app);
    if (!out) return;

    // Timestamp
    std::time_t now = std::time(nullptr);
    char buf[80];
    std::strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S] ", std::localtime(&now));
    out << buf << message << std::endl;
}