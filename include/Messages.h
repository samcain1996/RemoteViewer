#pragma once
#include "Types.h"

/*---------------------------------------*/
/*		Allows for threads to send		 */
/*	    messages between eachother       */
/*---------------------------------------*/

// Aliases for classes
template <typename Message>
class MessageHandler;
template <typename Message>
using MsgHandlerPtr = MessageHandler<Message>*;

template <typename Message>
class MessageReader;
template <typename Message>
using MsgReaderPtr = MessageReader<Message>*;

template <typename Message>
class MessageWriter;
template <typename Message>
using MsgWriterPtr = MessageWriter<Message>*;

template <typename Message>
using MessageWriterList = std::vector<MessageWriter<Message> >;

template <typename Message>
using MessageReaderList = std::vector<MessageReader<Message> >;

template <typename Message>
class MessageHandler {

private:
	const bool _ownsQueue;				// Flag to check whether the MessageHandler "owns" the queue

protected:
	Message _msg{};						// Last message read from queue
	std::mutex*	_mutex;					// Mutex to ensure only 1 thread access queue at one time
	std::queue<Message>* _queuePtr;	    // Pointer to queue holding messages

	// If constructed by itself, create a new queue
	MessageHandler() : _queuePtr(new std::queue<Message>), _mutex(new std::mutex), _ownsQueue(true) {};

	// If constructed from another MessageHandler, share its queue
	MessageHandler(MsgHandlerPtr<Message> const msgHandler) : _queuePtr(msgHandler->_queuePtr), _mutex(msgHandler->_mutex), _ownsQueue(false) {};

	MessageHandler(const MessageHandler&) = delete;
	MessageHandler(MessageHandler&&) = delete;

	~MessageHandler() { 
		if (_ownsQueue) { 
			delete _mutex;
			delete _queuePtr;
		} 
	};

	MessageHandler& operator=(const MessageHandler&) = delete;
	MessageHandler& operator=(MessageHandler&&) = delete;

	// Put message in back of queue
	const bool Push(const Message message) {
		ThreadLock lock(*_mutex);

		_queuePtr->push(message);

		return true;
	}

	// Remove message from the front of the queue
	const Message* const Pop() {
		ThreadLock lock(*_mutex);

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
class MessageReader : public MessageHandler<Message> {
public:
	// Create new queue
	MessageReader() : MessageHandler<Message>() {};

	// Create from existing queue
	MessageReader(MsgWriterPtr<Message> const writer) : MessageHandler<Message>(writer) {}

	// Do not allow copying
	MessageReader(const MessageReader&) = delete;
	MessageReader(MessageReader&&) = delete;

	MessageReader& operator=(const MessageReader&) = delete;
	MessageReader& operator=(MessageReader&&) = delete;

	// Read message from queue
	const Message ReadMessage() {
		const Message* const msgPtr = this->Pop();

		if (msgPtr != nullptr) {
			return *msgPtr;
		}

		return Message();
	}
};

// Write-only end of MessageHandler
template <typename Message>
class MessageWriter : public MessageHandler<Message> {
public:
	// Create new queue
	MessageWriter() : MessageHandler<Message>() {};

	// Create from existing queue
	MessageWriter(MsgReaderPtr<Message> const reader) : MessageHandler<Message>(reader) {}

	// Do not allow copying
	MessageWriter(const MessageWriter&) = delete;
	MessageWriter(MessageWriter&&) = delete;

	MessageWriter& operator=(const MessageWriter&) = delete;
	MessageWriter& operator=(MessageWriter&&) = delete;

	// Write message to queue
	const bool WriteMessage(const Message& message) {
		return this->Push(message);
	}
};