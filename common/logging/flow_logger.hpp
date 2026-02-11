#ifndef FLOW_LOGGER_HPP
#define FLOW_LOGGER_HPP

#include <stdint.h>

#include <QMutex>
#include <QMutexLocker>

#include "common/types.hpp"

static QMutex logMutex;

class FlowLogger
{
public:
    FlowLogger() = delete;
    static void log(const EntityType type, const uint32_t from,
                    const uint32_t to, const ProtocolMsgType msg_type,
                    const bool isIncoming);
    static QString formatId(uint32_t id);

private:
    static QString msgTypeToString(const ProtocolMsgType msg_type);
    static EntityType getOppositeType(EntityType senderType);
};

#endif  // FLOW_LOGGER_HPP
