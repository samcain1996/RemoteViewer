#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

class Logger {

private:

	inline static std::unordered_map< std::string, std::ofstream* > logStreams;

public:

	static void createNewLogStream(const std::string& name) { 
		logStreams[name] = new std::ofstream(name, std::ios::out); 
		
	}

	static bool closeStream(const std::string& name) {
		
		for (const auto& streamName : getStreamNames()) {
			
			if (name == streamName) { 

				std::ofstream* const stream = logStreams[streamName];

				stream->close();
			
				logStreams.erase(streamName); 

				delete stream;
				
				return true;
			}
		}

		return false;
	
	}
	
	static const std::vector<std::string> getStreamNames() { 
		
		
		std::vector< std::string > streamNames;
		
		for (const auto& [streamName, dummy] : logStreams) { streamNames.push_back(streamName); }
	
		return streamNames;
	}
	

	static void Log(const std::string& streamName, const std::string& message,
		const bool addNewLine = false, const bool checkIfStreamExists = true) {

		if (checkIfStreamExists) {

			bool exists = false;

			for (const auto& name : getStreamNames()) {
				if (name == streamName) { exists = true; break; }
			}

			if (!exists) { return; }

		}

		*logStreams[streamName] << message;

		if (addNewLine) { *logStreams[streamName] << "\n"; }

	}

	static void LogToGroup(const std::vector<std::string>& streamNames,
		const std::string& message) {
		
		for (const auto& name : streamNames) { Log(name, message); }
		
	}
};