#ifndef SERIALIZER_TYPE_HPP
#define SERIALIZER_TYPE_HPP

#include <cstdint>

enum class SerializerType : uint8_t {
    QDataStream = 0,
    Protobuf = 1
};

#endif  // SERIALIZER_TYPE_HPP
