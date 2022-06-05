#pragma once
#include "Server.h"
#include "Client.h"
#include "Window.h"

class Application {

private:

	Application() = delete;

	static bool _isClient;
	static bool _init;
	static std::atomic<bool> _exit;

	static std::unique_ptr<NetAgent> _netAgent;
	static std::unique_ptr<GenericWindow> _window;

	template <typename Message>
	static Messageable<Message> _msgHandler;

	template <typename Message>
	static void ConnectMessageHandlers(Messageable<Message>* m1, Messageable<Message>* m2) {
		m1->msgWriter = new MessageWriter<Message>;
		m2->msgReader = new MessageReader<Message>(m1->msgWriter);

		m2->msgWriter = new MessageWriter<Message>;
		m1->msgReader = new MessageReader<Message>(m2->msgWriter);
	}

	static void Cleanup();

	static void RunClient(Client& client);
	static void RunServer(Server& server);


public:
	static bool Init();
	static void Run();
};



