#pragma once
#include <fstream>
#include <iostream>
#include <vector>

class Logger {

private:

	std::vector<std::ostream*> outStreams;

public:

	Logger(const std::string& name) { 
		outStreams.emplace_back(new std::ofstream(name, std::ios_base::out)); 
	
	}

	~Logger() {

		for (size_t index = 0; index < outStreams.size(); index++) {

			std::ofstream* of = dynamic_cast<std::ofstream*>(outStreams[index]);
			of->close();
			delete outStreams[index];

		}

	}

	void Log(const std::string& message) {

		for (auto& out : outStreams) { *out << message; }

	}


	void LogLine(const std::string& message) {
	
		Log(message + "\n");
	
	}
};