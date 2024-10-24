#include "Simulation.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>

std::string GetCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime = *std::localtime(&currentTime);
    std::stringstream timeStream;
    timeStream << '[' 
               << std::setw(2) << std::setfill('0') << localTime.tm_hour << ':'
               << std::setw(2) << std::setfill('0') << localTime.tm_min << ':'
               << std::setw(2) << std::setfill('0') << localTime.tm_sec << ']';
    return timeStream.str();
}

void Simulation::Log(std::string log) {
    m_Logs.push_back(GetCurrentTimeString() + " " + log); 
    if (m_Logs.size() > 10000) 
        m_Logs.erase(m_Logs.begin());
}

bool Simulation::Started() { 
    return m_Started; 
}

bool Simulation::Paused() { 
    return m_Paused; 
}

void Simulation::Start() { 
    m_Started = true; m_Paused = false; 
}

void Simulation::Pause() { 
    m_Paused = true; 
}

void Simulation::Resume() { 
    m_Paused = false; 
}

void Simulation::Abort() { 
    m_Started = false; 
    m_Paused = false; 
}
