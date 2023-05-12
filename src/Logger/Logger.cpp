#include "Logger/Logger.h"

NullableLoggette Logger::getLog(const string& name) {
	if (!streamExists(name)) { return std::nullopt; }
	return Loggette(name, logStreams[name]);
}

bool Logger::streamExists(const string& name) {
	return std::find(streamNames.begin(), streamNames.end(), name) != streamNames.end();
}

NullableLoggette Logger::openStream(const string& name, const std::ios::openmode mode) {

	if (streamExists(name)) { return std::nullopt; }

	LogStreamPtr stream = make_shared<LogStream>(name, mode);
	Loggette loggette(name, stream);

	if (stream->good() && streamNames.insert(name).second) {
		logStreams[name] = stream;
	}

	return loggette;
}

NullableLoggette Logger::newStream(const string& name, const std::ios::openmode mode) {
	return openStream(name, mode);
}

bool Logger::closeStream(const string& name) {

	// Check if stream exists
	if (!streamExists(name)) { return false; }

	// Close and delete stream
	logStreams[name]->close();
	logStreams.erase(name);

	return true;

}

void Logger::Log(const string& streamName, const string& message, const bool checkIfStreamExists) {

	ThreadLock lock(mutex);

	// Check if stream exists before writing
	if (checkIfStreamExists) {
		if (!streamExists(streamName)) { return; }
	}

	*logStreams[streamName] << message;

}

void Logger::LogLine(const string& streamName, const string& message, const bool checkIfStreamExists) {
	Log(streamName, message + "\n", checkIfStreamExists);
}

void Logger::LogToGroup(const StreamNames& streamNames, const string& message) {
	for (const string& name : streamNames) { Log(name, message); }
}