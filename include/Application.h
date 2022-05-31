#pragma once
#include "Server.h"
#include "Client.h"

#define DEBUG 1

class Application {
private:

	Application() = delete;

	static bool _isClient;
	static bool _init;
	static std::atomic<bool> _exit;

	static GenericWindow* _window;
	static NetAgent* _netAgent;

	static MessageReader<ByteArray>* eventReader;
	static MessageReader<SDL_Event>* sigh;

	static void Cleanup();

	static void RunClient(Client&);
	static void RunServer(Server&);

	template <typename T>
	static void ConnectMsgHandlers(MsgWriterPtr<T>& writer, MsgReaderPtr<T>& reader) {
		writer = new MessageWriter<T>;
		reader = new MessageReader<T>(writer);
	}

public:
	static bool Init(const bool isClient);
	static void Run();
};
