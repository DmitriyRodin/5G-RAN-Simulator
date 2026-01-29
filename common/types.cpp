#include <QString>
#include <QJsonObject>

#include "types.hpp"

QDebug operator<<(QDebug stream, EntityType type)
{
    switch(type) {
        case EntityType::GNB:     stream.nospace() << "GNB"; break;
        case EntityType::UE:      stream.nospace() << "UE"; break;
        case EntityType::UNKNOWN: stream.nospace() << "Unknown"; break;
        default:                  stream.nospace() << "ErrorType"; break;
    }
    return stream;
}

QString typeToString(EntityType type)
{
    switch(type) {
        case EntityType::UE: return "UE";
        case EntityType::GNB: return "gNB";
        default: return "Unknown";
    }
}
