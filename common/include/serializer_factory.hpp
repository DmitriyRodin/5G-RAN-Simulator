#ifndef SERIALIZER_FACTORY_HPP
#define SERIALIZER_FACTORY_HPP

#include <functional>
#include <map>
#include <memory>

#include <QString>

#include "iserializer.hpp"

enum class SerializerType : uint8_t;

class SerializerFactory
{
public:
    using CreatorFunc = std::function<std::unique_ptr<ISerializer>()>;

    static std::unique_ptr<ISerializer> create(const SerializerType& type);

    static bool registerSerializer(const SerializerType& type,
                                   CreatorFunc creator);

private:
    static std::map<SerializerType, CreatorFunc>& getRegistry();
};

#endif  // SERIALIZER_FACTORY_HPP
