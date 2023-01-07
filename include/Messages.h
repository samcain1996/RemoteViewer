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

protected:
	std::mutex*	_mutex;					// Mutex to ensure only 1 thread access queue at one time
	std::queue<Message>* _queuePtr;	    // Pointer to queue holding messages

	// If constructed by itself, create a new queue
	MessageHandler() : _ownsQueue(true), _mutex(new std::mutex), _queuePtr(new std::queue<Message>) {};

	// If constructed from another MessageHandler, share its queue
	MessageHandler(MessageHandler<Message>* const msgHandler) : _ownsQueue(false), _mutex(msgHandler->_mutex), _queuePtr(msgHandler->_queuePtr) {};

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
	bool Push(Message&& message) {
		ThreadLock lock(*_mutex);

		_queuePtr->push(std::forward<Message>(message));

		return true;
	}

	// Remove message from the front of the queue
	Message Pop() {
		ThreadLock lock(*_mutex);

		if (_queuePtr->empty()) { return nullptr; }

		Message msg = _queuePtr->front();
		_queuePtr->pop();

		return msg;
	};

public:
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
	MessageReader(MessageWriter<Message>* const writer) : MessageHandler<Message>(writer) {}

	// Do not allow copying
	MessageReader(const MessageReader&) = delete;
	MessageReader(MessageReader&&) = delete;

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
	MessageWriter(MessageReader<Message>* const reader) : MessageHandler<Message>(reader) {}

	// Do not allow copying
	MessageWriter(const MessageWriter&) = delete;
	MessageWriter(MessageWriter&&) = delete;

	MessageWriter& operator=(const MessageWriter&) = delete;
	MessageWriter& operator=(MessageWriter&&) = delete;

	// Write message to queue
	bool WriteMessage(Message&& message) {
		return this->Push(std::forward<Message>(message));
	}
};

//template <class T, typename Message>
//concept IsMsgHandler = std::same_as<T, MessageHandler<Message>> || std::derived_from<T, MessageHandler<Message>>;
//
//template <class T, typename Message>
//concept IsDerivedMsgHandler = IsMsgHandler<T, Message> && (!std::same_as<T, MessageHandler<Message>>);
//
//template <class T, class U, typename Message>
//concept IsOppositeMsgHandler = IsDerivedMsgHandler<T, Message> && IsDerivedMsgHandler<U, Message> &&
//(!std::same_as<T, U>);
