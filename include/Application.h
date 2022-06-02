#pragma once
#include "Server.h"
#include "Client.h"
#include "Window.h"
#include <functional>

class Application {

private:

	Application() = delete;

	static bool _isClient;
	static bool _init;
	static std::atomic<bool> _exit;

	static GenericWindow* _window;
	static NetAgent* _netAgent;

	template <typename T>
	static MsgWriterPtr<T> _writer;

	template <typename T>
	static MsgReaderPtr<T> _reader;

	static void Cleanup();

	static void RunClient(Client&);
	static void RunServer(Server&);

	template <typename T>
	static void ConnectMsgHandlers(MsgWriterPtr<T>& writer, MsgReaderPtr<T>& reader) {
		writer = new MessageWriter<T>;
		reader = new MessageReader<T>(writer);
	}

public:
	static bool Init();
	static void Run();
};
