#include <QString>
#include <QJsonObject>

#include "types.hpp"

MessageType parseMessageType(const QJsonObject& obj)
{
    const QString type = obj["type"].toString();

    if (type == "ATTACH_REQUEST")      return MessageType::AttachRequest;
    if (type == "ATTACH_ACCEPT")       return MessageType::AttachAccept;
    if (type == "MEASUREMENT_REPORT")  return MessageType::MeasurementReport;
    if (type == "DATA_TRANSFER")       return MessageType::DataTransfer;

    return MessageType::Unknown;
}
