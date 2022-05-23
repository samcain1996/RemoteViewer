#pragma once
#include <queue>
#include "Types.h"

template <typename Message>
class MessageHandler {

private:

	//~MessageHandler() { delete _queuePtr; };  // Leak ??

protected:
	Message _msg;
	std::mutex	_mutex;
	std::queue<Message>* _queuePtr;

	MessageHandler(const MessageHandler&) = delete;
	MessageHandler(MessageHandler&&) = delete;

	MessageHandler() : _queuePtr(new std::queue<Message>) {};

	void SetQueuePtr(MessageHandler* msgHandler) {
		msgHandler->_queuePtr = _queuePtr;
	}

	bool Push(const Message message) {
		ThreadLock lock(_mutex);

		_queuePtr->push(message);

		return true;
	}

	Message* Pop() {
		ThreadLock lock(_mutex);

		if (_queuePtr->empty()) { return nullptr; }

		_msg = _queuePtr->front();
		_queuePtr->pop();

		return &_msg;
	};

public:
	bool Empty() const { return _queuePtr->empty(); }
};

template <typename Message>
class MessageReader : public MessageHandler<Message>{
public:
	MessageReader() : MessageHandler<Message>() {};

	MessageReader(MessageHandler<Message>* writer) {
		this->SetQueuePtr(writer);
	};

	MessageReader(const MessageReader&) = delete;
	MessageReader(MessageReader&&) = delete;

	const Message ReadMessage() {
		Message* msgPtr = this->Pop();

		if (msgPtr != nullptr) {
			return *msgPtr;
		}
	}
};

template <typename Message>
class MessageWriter : public MessageHandler<Message>{
public:
	MessageWriter() : MessageHandler<Message>() {};

	MessageWriter(MessageHandler<Message>* reader) {
		SetQueuePtr(reader);
	};

	MessageWriter(const MessageWriter&) = delete;
	MessageWriter(MessageWriter&&) = delete;

	bool WriteMessage(const Message& message) {
		return this->Push(message);
	}
};