#include "../include/utils.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

void logWithTimestamp(const std::string& msg) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "[" << std::put_time(std::localtime(&now_c), "%T") << "] " << msg << std::endl;
}
