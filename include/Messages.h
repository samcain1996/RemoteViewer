#pragma once
#include "Types.h"


// Aliases for classes
template <typename Message>
class MessageHandler;
template <typename Message>
using MsgHandlerPtr = MessageHandler<Message>*;
template <typename Message>
using MsgHandlerRef = MessageHandler<Message>&;

template <typename Message>
class MessageReader;
template <typename Message>
using MsgReaderPtr = MessageReader<Message>*;
template <typename Message>
using MsgReaderRef = MessageReader<Message>&;

template <typename Message>
class MessageWriter;
template <typename Message>
using MsgWriterPtr = MessageWriter<Message>*;
template <typename Message>
using MsgWriterRef = MessageWriter<Message>&;

template <typename Message>
class MessageHandler {

private:
	const bool _ownsQueue;				// Flag to check whether the MessageHandler "owns" the queue

protected:
	Message _msg{};						// Last message read from queue
	std::mutex	_mutex;					// Mutex to ensure only 1 thread access queue at one time
	std::queue<Message>* _queuePtr;	    // Pointer to queue holding messages

	// If constructed by itself, create a new queue
	MessageHandler() : _queuePtr(new std::queue<Message>), _ownsQueue(true) {};

	// If constructed from another MessageHandler, share its queue
	MessageHandler(MsgHandlerPtr<Message> msgHandler) : _queuePtr(msgHandler->_queuePtr), _ownsQueue(false) {};

	MessageHandler(const MessageHandler&) = delete;
	MessageHandler(MessageHandler&&) = delete;

	~MessageHandler() { if (_ownsQueue) { delete _queuePtr; } };

	MessageHandler& operator=(const MessageHandler&) = delete;
	MessageHandler& operator=(MessageHandler&&) = delete;

	// Put message in back of queue
	bool Push(const Message message) {
		ThreadLock lock(_mutex);

		_queuePtr->push(message);

		return true;
	}

	// Remove message from the front of the queue
	const Message* const Pop() {
		ThreadLock lock(_mutex);

		if (_queuePtr->empty()) { return nullptr; }

		_msg = _queuePtr->front();
		_queuePtr->pop();

		return &_msg;
	};

public:
	// Return if the queue is empty
	bool Empty() const { return _queuePtr->empty(); }
};

// Read-only end of MessageHandler
template <typename Message>
class MessageReader : public MessageHandler<Message>{
public:
	// Create new queue
	MessageReader() : MessageHandler<Message>() {};

	// Create from existing queue
	MessageReader(MsgWriterPtr<Message> writer) : MessageHandler<Message>(writer) {}

	// Do not allow copying
	MessageReader(const MessageReader&) = delete;
	MessageReader(MessageReader&&) = delete;

	MessageReader& operator=(const MessageReader&) = delete;
	MessageReader& operator=(MessageReader&&) = delete;

	// Read message from queue
	const Message ReadMessage() {
		const Message* msgPtr = this->Pop();

		if (msgPtr != nullptr) {
			return *msgPtr;
		}

		return Message();
	}
};

// Write-only end of MessageHandler
template <typename Message>
class MessageWriter : public MessageHandler<Message>{
public:
	// Create new queue
	MessageWriter() : MessageHandler<Message>() {};

	// Create from existing queue
	MessageWriter(MsgReaderPtr<Message> reader) : MessageHandler<Message>(reader) {}

	// Do not allow copying
	MessageWriter(const MessageWriter&) = delete;
	MessageWriter(MessageWriter&&) = delete;

	MessageWriter& operator=(const MessageWriter&) = delete;
	MessageWriter& operator=(MessageWriter&&) = delete;

	// Write message to queue
	bool WriteMessage(const Message& message) {
		return this->Push(message);
	}
};