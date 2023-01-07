#pragma once

#include <set>
#include <mutex>
#include <thread>
#include <chrono>
#include <string>
#include <fstream>
#include <optional>
#include <algorithm>
#include <functional>
#include <unordered_map>

using LogStream = std::ofstream;
using LogStreamPtr = std::shared_ptr<std::ofstream>;

using StreamNames = std::set<std::string>;
using NameToStreamMap = std::unordered_map<std::string, LogStreamPtr>;
using NameStreamPair = std::pair<std::string, LogStreamPtr>;

using ThreadLock = std::lock_guard<std::mutex>;

using LogFunction = std::function<void(const StreamNames&)>;

using std::ios;
using namespace std::chrono;

struct Loggette {

	Loggette() = default;
	Loggette(const std::string& name, const LogStreamPtr const logStreamPtr) : 
		name(name), streamPtr(logStreamPtr) {}
	Loggette(const NameStreamPair& pair) : Loggette(pair.first, pair.second) {}

	const std::string name = "ERROR";
	LogStreamPtr streamPtr = nullptr;

	void Write(const std::string& message) { *streamPtr << message; }
	void WriteLine(const std::string& message) { Write(message + "\n"); }
};

using NullableLoggette = std::optional<Loggette>;

class Logger {

private:

	static inline NameToStreamMap logStreams;
	static inline StreamNames streamNames;
	static inline std::mutex mutex;

public:

	static bool streamExists(const std::string& name);

	static void startTimerLog(const StreamNames& streams, const minutes& interval);

	static NullableLoggette getLog(const std::string& name);

	static NullableLoggette openStream(const std::string& name, 
		const ios::openmode mode = ios::out | ios::app);

	static NullableLoggette newStream(const std::string& name, 
		const ios::openmode mode = ios::out);

	static bool closeStream(const std::string& name);

	static void Log(const std::string& streamName, const std::string& message,
		const bool checkIfStreamExists = true);

	static void LogLine(const std::string& streamName, const std::string& message,
		const bool checkIfStreamExists = true);

	static void LogToGroup(const StreamNames& streamNames, const std::string& message);
};