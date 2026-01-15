#include <QDebug>

#include <base_entity.hpp>

BaseEntity::BaseEntity(int id, const EntityType& type)
    : id_(id)
    , type_(type)
{
}

BaseEntity::~BaseEntity() {
    running_ = false;
}

void BaseEntity::run()
{
    if (type_ == EntityType::GNB) {
        qDebug() << "GNB # " << id_ << ", starts";
    } else if (type_ == EntityType::UE) {
        qDebug() << "UE # " << id_ << ", starts and is searching for network";
    }
}

void BaseEntity::stop() {
    running_ = false;
}

void BaseEntity::setup_network()
{

}

void BaseEntity::send_message(const std::string &payload)
{

}
