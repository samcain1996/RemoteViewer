#pragma once
#include "Messageable.h"


/*---------------------------------------*/
/*		Allows for threads to send		 */
/*	    messages between eachother       */
/*---------------------------------------*/

template <typename Message>
class MessageHandler {

private:
	const bool _ownsQueue;				// Flag to check whether the MessageHandler "owns" the queue
	bool moved = false;
	std::mutex*	_mutex;					// Mutex to ensure only 1 thread access queue at one time

protected:
	std::queue<Message>* _queuePtr;	    // Pointer to queue holding messages

	// If constructed by itself, create a new queue
	MessageHandler() : _ownsQueue(true), _mutex(new std::mutex), _queuePtr(new std::queue<Message>) {};

	// If constructed from another MessageHandler, share its queue
	MessageHandler(const MessageHandler<Message>& msgHandler) : _ownsQueue(false), _mutex(msgHandler._mutex), _queuePtr(msgHandler._queuePtr) {};
	MessageHandler(MessageHandler<Message>* const msgHandler) : _ownsQueue(false), _mutex(msgHandler->_mutex), _queuePtr(msgHandler->_queuePtr) {};
	//MessageHandler(MessageHandler<Message>&& other) noexcept :
	//	_ownsQueue(std::move(other._ownsQueue)), _mutex(std::move(other._mutex)),
	//	_queuePtr(std::move(other._queuePtr)) { other.moved = true; };

	~MessageHandler() { 
		if (_ownsQueue && !moved) { 
			delete _mutex;
			delete _queuePtr;
		} 
	};

	MessageHandler& operator=(const MessageHandler&) = delete;
	MessageHandler& operator=(MessageHandler&&) = delete;

	// Put message in back of queue
	bool Push(Message message) {
		ThreadLock lock(*_mutex);

		_queuePtr->push(message);

		return true;
	}

	// Remove message from the front of the queue
	Message Pop() {
		ThreadLock lock(*_mutex);

		Message msg = _queuePtr->front();
		_queuePtr->pop();

		return msg;
	};

public:
	int Size() const { return _queuePtr->size(); }
	// Return if the queue is empty
	bool Empty() const { return _queuePtr->empty(); }
	void Clear() { 
		ThreadLock lock(*_mutex);

		while (!_queuePtr->empty()) { _queuePtr->pop(); }

	}
};

// Read-only end of MessageHandler
template <typename Message>
class MessageReader : public MessageHandler<Message> {
public:
	// Create new queue
	MessageReader() : MessageHandler<Message>() {};

	// Create from existing queue
	MessageReader(const MessageWriter<Message>& writer) : MessageHandler<Message>(writer) {}
	MessageReader(MessageWriter<Message>* const writer) : MessageHandler<Message>(writer) {}
	// Do not allow copying
	MessageReader(const MessageReader<Message>& reader) = delete;
	MessageReader(MessageReader<Message>&& other) noexcept : 
		MessageHandler<Message>(std::forward<MessageReader<Message>>(other)) {};

	MessageReader& operator=(const MessageReader&) = delete;
	MessageReader& operator=(MessageReader&&) = delete;

	// Read message from queue
	Message ReadMessage() {
		return this->Pop();
	}
};

// Write-only end of MessageHandler
template <typename Message>
class MessageWriter : public MessageHandler<Message> {
public:
	// Create new queue
	MessageWriter() : MessageHandler<Message>() {};

	// Create from existing queue
	MessageWriter(const MessageReader<Message>& reader) : MessageHandler<Message>(reader) {}
	MessageWriter(MessageReader<Message>* const reader) : MessageHandler<Message>(reader) {}

	MessageWriter(const MessageWriter<Message>& writer) = delete;
	MessageWriter(MessageWriter<Message>&& other) noexcept : 
		MessageHandler<Message>(std::forward<MessageWriter<Message>>(other)) {};

	MessageWriter& operator=(const MessageWriter&) = delete;
	MessageWriter& operator=(MessageWriter&&) = delete;

	// Write message to queue
	bool WriteMessage(Message&& message) {
		return this->Push(std::forward<Message>(message));
	}
	//bool WriteMessage(Message message) {
	//	return this->Push(message);
	//}
};

