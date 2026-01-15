#ifndef BASE_ENTITY_HPP
#define BASE_ENTITY_HPP

#include <atomic>
#include <string>

#include <types.hpp>

class BaseEntity {
public:
    BaseEntity(int id, const EntityType& type);
    virtual ~BaseEntity();

    void run();
    void stop();

protected:

    void setup_network();
    void send_message(const std::string& payload);

    int id_;
    EntityType type_;
    std::atomic<bool> running_{true};
};

#endif // BASE_ENTITY_HPP
