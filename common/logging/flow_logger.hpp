#ifndef FLOW_LOGGER_HPP
#define FLOW_LOGGER_HPP

#include <stdint.h>
#include "common/types.hpp"

#include <QMutex>
#include <QMutexLocker>

static QMutex logMutex;

class FlowLogger {
public:
    FlowLogger() = delete;
    static void log(const EntityType type,
                    const uint32_t from,
                    const uint32_t to,
                    const ProtocolMsgType msg_type,
                    const bool isIncoming);

private:
    static QString formatId(uint32_t id);
    static QString msgTypeToString(const ProtocolMsgType msg_type);
};

#endif // FLOW_LOGGER_HPP
