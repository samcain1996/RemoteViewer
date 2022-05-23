#pragma once
#include <queue>
#include "Types.h"

using ApplicationMessage = std::pair<ByteArray, uint32>;
using MessageQueue = std::queue<ApplicationMessage>;

class ApplicationMessageHandler {

protected:
	std::mutex	_mutex;
	ApplicationMessage _msg;
	MessageQueue* _queuePtr;

	ApplicationMessageHandler(const ApplicationMessageHandler&) = delete;
	ApplicationMessageHandler(ApplicationMessageHandler&&) = delete;

	ApplicationMessageHandler() : _queuePtr(new MessageQueue) {};

	void SetQueuePtr(ApplicationMessageHandler* amh) {
		amh->_queuePtr = _queuePtr;
	}

	bool Push(const ApplicationMessage message) {
		ThreadLock lock(_mutex);

		_queuePtr->push(message);

		return true;
	}
	ApplicationMessage* Pop() {
		ThreadLock lock(_mutex);

		if (_queuePtr->empty()) { return nullptr; }

		_msg = _queuePtr->front();
		_queuePtr->pop();

		return &_msg;
	};

public:
	bool Empty() const { return _queuePtr->empty(); }
};

class ApplicationMessageReader : public ApplicationMessageHandler {
public:
	ApplicationMessageReader() : ApplicationMessageHandler() {};

	ApplicationMessageReader(ApplicationMessageHandler* writer) {
		SetQueuePtr(writer);
	};

	ApplicationMessageReader(const ApplicationMessageReader&) = delete;
	ApplicationMessageReader(ApplicationMessageReader&&) = delete;

	const ApplicationMessage ReadMessage() {
		ApplicationMessage* msgPtr = Pop();

		if (msgPtr != nullptr) {
			return *msgPtr;
		}
	}
};

class ApplicationMessageWriter : public ApplicationMessageHandler {
public:
	ApplicationMessageWriter() : ApplicationMessageHandler() {};

	ApplicationMessageWriter(ApplicationMessageHandler* reader) {
		SetQueuePtr(reader);
	};

	ApplicationMessageWriter(const ApplicationMessageWriter&) = delete;
	ApplicationMessageWriter(ApplicationMessageWriter&&) = delete;

	bool WriteMessage(const ApplicationMessage& message) {
		return Push(message);
	}
};