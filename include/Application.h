#pragma once
#include "Server.h"
#include "Client.h"

class Application {
private:

	Application() = delete;

	static bool _isClient;
	static bool _init;
	static std::atomic<bool> _exit;

	static GenericWindow* _window;
	static NetAgent* _netAgent;
	
	static std::thread* _networkThr;

	static void Cleanup();

	static void RunClient();
	static void RunServer();

	template <typename T>
	static void ConnectMsgHandlers(MessageWriter<T>*& writer, MessageReader<T>*& reader) {
		writer = new MessageWriter<T>;
		reader = new MessageReader<T>(writer);
	}

public:
	static void Init(const bool isClient);
	static void Run();
};
