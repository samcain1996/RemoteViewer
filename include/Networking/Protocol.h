#pragma once

#include <unordered_set>
#include "Types.h"

using Data = Byte;
using ProtocolSubtype = Byte;
using ProtocolFunction = std::function<void(Data* data, size_t size)>;
using ProtocolMap = std::unordered_map<Data, std::unordered_set<ProtocolSubtype> >;
using SubtypeSet = std::unordered_map<ProtocolSubtype, ProtocolFunction>;


class Protocol {

protected:

    ProtocolSubtype _nextSubtype = 'S';

    const ProtocolFunction DEFAULT_PROTOCOL_FUNCTION = [](Data* data, size_t size) -> void {};

    ProtocolMap _protocolMap;
    SubtypeSet _serializer;

	Protocol(const ProtocolMap& _protocolMap) : _protocolMap(_protocolMap) {

        // Add default subtype mapping
        _serializer[ProtocolSubtype{}] = DEFAULT_PROTOCOL_FUNCTION;
    }


    //virtual const ProtocolSubtype getSubtype(Data* data, size_t size) const = 0;

public:

    virtual const bool addSubtype(const ProtocolSubtype& subtype, const ProtocolFunction& deserializeFunction) {

        if (this->_protocolMap.find(subtype) == this->_protocolMap.end()) { return false; }

        _serializer[subtype] = deserializeFunction;
        return true;
    }
};


class ImageProtocol : public Protocol {
public:

    //const Data getSubtype(Data* data, size_t size) const override {

    //    if (size <= 0) { return char{}; }

    //    return data[0];
    //}

public:
    ImageProtocol() : Protocol(
        ProtocolMap { 
        { 'S', { 'I', 'E' } },
        { 'I', { 'I', 'E' } },
        { 'E', { 'S' } }
        }) {
    
        _serializer['S'] = [](Data* data, size_t size) {};
        _serializer['E'] = [](Data* data, size_t size) {};
    }

};

