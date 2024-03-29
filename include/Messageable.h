#pragma once

#include "Types.h"

template <typename Message>
class MessageReader;

template <typename Message>
class MessageWriter;

template <typename Message>
class Messageable {
    friend void ConnectMessageables(Messageable<Message>& m1, Messageable<Message>& m2) {
        m1.msgWriter = new MessageWriter<Message>;
        m2.msgReader = new MessageReader<Message>(m1.msgWriter);

        m2.msgWriter = new MessageWriter<Message>;
        m1.msgReader = new MessageReader<Message>(m2.msgWriter);
    }
    friend void Connect(Messageable<Message>& m1, Messageable<Message>& m2) {
        m2.msgReader = new MessageReader<Message>(m1.msgWriter);
    }
public:
    MessageWriter<Message>* msgWriter = nullptr;
    MessageReader<Message>* msgReader = nullptr;

    virtual ~Messageable() {
        if (msgWriter != nullptr) {
            delete msgWriter;
            delete msgReader;
        }
    }
};