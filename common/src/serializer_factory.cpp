#include "serializer_factory.hpp"

std::unique_ptr<ISerializer> SerializerFactory::create(
    const SerializerType& type)
{
    auto& registry = getRegistry();
    auto it = registry.find(type);
    if (it == registry.end()) {
        return nullptr;
    }
    return it->second();
}

bool SerializerFactory::registerSerializer(const SerializerType& type,
                                           CreatorFunc creator)
{
    getRegistry()[type] = creator;
    return true;
}

std::map<SerializerType, SerializerFactory::CreatorFunc>&
SerializerFactory::getRegistry()
{
    static std::map<SerializerType, CreatorFunc> registry;
    return registry;
}
