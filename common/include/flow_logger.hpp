#ifndef FLOW_LOGGER_HPP
#define FLOW_LOGGER_HPP

#include <stdint.h>

#include <QMutex>
#include <QMutexLocker>

#include "types.hpp"

static QMutex logMutex;

class FlowLogger
{
public:
    FlowLogger() = delete;

    static void setup(uint32_t hub_id, uint32_t broadcast_id);

    static void log(const EntityType type, const uint32_t from,
                    const uint32_t to, const ProtocolMsgType msg_type,
                    const bool isIncoming);
    static QString formatId(const uint32_t id);

private:
    static QString msgTypeToString(const ProtocolMsgType msg_type);
    static EntityType getOppositeType(EntityType senderType);

    inline static uint32_t hub_id_ = 0;
    inline static uint32_t broadcast_id_ = 0;
    inline static bool initialized_ = false;
};

#endif  // FLOW_LOGGER_HPP
