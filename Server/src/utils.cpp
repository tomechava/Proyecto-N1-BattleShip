#include "../include/utils.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <set>


void logWithTimestamp(const std::string& msg) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "[" << std::put_time(std::localtime(&now_c), "%T") << "] " << msg << std::endl;
}

// Versión por defecto: log general (poco usada con macros)
void logToFile(const std::string& message) {
    const std::string defaultLogFile = "logs/server.log";
    logToFile(defaultLogFile, message);
}

// Versión con nombre de archivo
void logToFile(const std::string& file, const std::string& message) {
    std::ofstream out(file, std::ios::app);
    if (!out) return;

    std::time_t now = std::time(nullptr);
    char buf[80];
    std::strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S] ", std::localtime(&now));
    out << buf << message << std::endl;
}

std::string vectorToString(const std::vector<std::string>& vec) {
    std::string result = "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        result += vec[i];
        if (i != vec.size() - 1) result += ", ";
    }
    result += "]";
    return result;
}

std::string vectorOfVectorsToString(const std::vector<std::vector<std::string>>& vec) {
    std::string result = "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        result += vectorToString(vec[i]);
        if (i != vec.size() - 1) result += ", ";
    }
    result += "]";
    return result;
}

//Imprimimos barco
std::string setToString(const std::set<std::string>& s) {
    std::string result = "{";
    size_t i = 0;
    for (const auto& item : s) {
        result += item;
        if (i != s.size() - 1) result += ", ";
        ++i;
    }
    result += "}";
    return result;
}

//Imprimimos listado de barcos
std::string vectorOfSetsToString(const std::vector<std::set<std::string>>& vec) {
    std::string result = "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        result += setToString(vec[i]);
        if (i != vec.size() - 1) result += ", ";
    }
    result += "]";
    return result;
}