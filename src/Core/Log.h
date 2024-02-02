#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/ringbuffer_sink.h"
#include "spdlog/fmt/ostr.h"

class Log{
public:
	static void Init();
	inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }
	inline static std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt>& GetRingBuffer() { return s_Ringbuffer; }
private:
	static std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> s_Ringbuffer;
	static std::shared_ptr<spdlog::logger> s_Logger;
};

#define ERROR(...) ::Log::GetLogger()->error(__VA_ARGS__)
#define WARN(...)  ::Log::GetLogger()->warn(__VA_ARGS__)
#define INFO(...)  ::Log::GetLogger()->info(__VA_ARGS__)
#define TRACE(...) ::Log::GetLogger()->trace(__VA_ARGS__)
