#include "qdatastream_serializer.hpp"

#include <QIODevice>

QByteArray QDataStreamSerializer::serializeRrcSetupRequest(
    const RrcSetupRequest& info) const
{
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << static_cast<quint64>(info.ue_identity);
    ds << static_cast<uint8_t>(info.cause);
    return data;
}

RrcSetupRequest QDataStreamSerializer::deserializeRrcSetupRequest(
    const QByteArray& payload) const
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    quint64 id;
    uint8_t cause;
    ds >> id >> cause;

    return RrcSetupRequest{id, cause};
}

QByteArray QDataStreamSerializer::serializeRachPreamble(
    const uint16_t& ra_rnti) const
{
    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << ra_rnti;
    return payload;
}

RachPreambleInfo QDataStreamSerializer::deserializeRachPreamble(
    const QByteArray& payload) const
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);
    uint16_t ra_rnti;
    ds >> ra_rnti;
    return RachPreambleInfo{ra_rnti};
}

QByteArray QDataStreamSerializer::serializeRar(const RarInfo& info) const
{
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);

    out << info.ra_rnti;
    out << info.temp_c_rnti;
    out << info.timing_advance;
    return payload;
}

RarInfo QDataStreamSerializer::deserializeRar(const QByteArray& payload) const
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    RarInfo info;
    ds >> info.ra_rnti >> info.temp_c_rnti >> info.timing_advance;

    return info;
}

QByteArray QDataStreamSerializer::serializeRrcSetup(
    const RrcSetupInfo& info) const
{
    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << info.received_identity << info.config_status;
    return payload;
}

RrcSetupInfo QDataStreamSerializer::deserializeRrcSetup(
    const QByteArray& payload) const
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    quint64 received_identity;
    uint8_t config_status;

    ds >> received_identity;
    ds >> config_status;

    return {received_identity, config_status};
}

QByteArray QDataStreamSerializer::serializeRrcSetupComplete(
    const RrcSetupCompleteInfo& info) const
{
    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << info.plmn.mcc << info.plmn.mnc;
    return payload;
}

RrcSetupCompleteInfo QDataStreamSerializer::deserializeRrcSetupComplete(
    const QByteArray& payload) const
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);
    RrcSetupCompleteInfo info;
    ds >> info.plmn.mcc >> info.plmn.mnc;
    return info;
}

QByteArray QDataStreamSerializer::serializeRrcRelease(
    const RrcReleaseCause& cause) const
{
    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << static_cast<uint8_t>(cause);
    return payload;
}

RrcReleaseCause QDataStreamSerializer::deserializeRrcRelease(
    const QByteArray& payload) const
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);
    uint8_t cause_raw;
    ds >> cause_raw;
    RrcReleaseCause cause = static_cast<RrcReleaseCause>(cause_raw);
    return cause;
}

QByteArray QDataStreamSerializer::serializeRegistrationRequest(
    const RegistrationRequestInfo& info) const
{
    QByteArray nas_data;
    QDataStream ds(&nas_data, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << info.ue_id << info.ue_cap;
    return nas_data;
}

RegistrationRequestInfo QDataStreamSerializer::deserializeRegistrationRequest(
    const QByteArray& payload) const
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);
    RegistrationRequestInfo info;

    ds >> info.ue_id >> info.ue_cap;
    return info;
}

QByteArray QDataStreamSerializer::serializeRegistrationAnswer(
    const RegistrationAnswerInfo& info) const
{
    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << info.status;
    if (info.reject_reason.has_value()) {
        ds << info.reject_reason.value();
    }
    return payload;
}

RegistrationAnswerInfo QDataStreamSerializer::deserializeRegistrationAnswer(
    const QByteArray& payload) const
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);
    RegistrationAnswerInfo result;
    uint8_t raw_status;
    ds >> raw_status;
    result.status = static_cast<RegistrationStatus>(raw_status);

    if (result.status != RegistrationStatus::Accepted) {
        QString message;
        ds >> message;
        result.reject_reason = message;
    }
    return result;
}

QByteArray QDataStreamSerializer::serializeMeasurementReport(
    const MeasurementReportInfo& info) const
{
    QByteArray report;
    QDataStream ds(&report, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << info.reported_gnb_id << info.rsrp;

    return report;
}

MeasurementReportInfo QDataStreamSerializer::deserializeMeasurementReport(
    const QByteArray& payload) const
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    MeasurementReportInfo info;
    ds >> info.reported_gnb_id;
    ds >> info.rsrp;

    return info;
}

std::optional<RrcReconfigurationInfo>
QDataStreamSerializer::deserializeRrcReconfiguration(
    const QByteArray& payload) const
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    uint32_t target_gnb_id;

    if (ds.atEnd()) {
        return std::nullopt;
    }
    ds >> target_gnb_id;
    return RrcReconfigurationInfo{target_gnb_id};
}

ChatMessageInfo QDataStreamSerializer::deserializeChatMessage(
    const QByteArray& payload) const
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    ChatMessageInfo message;
    ds >> message.receiver_ue_id >> message.sender_ue_id >> message.text;
    return message;
}

QByteArray QDataStreamSerializer::serializeChatMessage(
    const ChatMessageInfo& message) const
{
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << message.receiver_ue_id << message.sender_ue_id << message.text;

    return data;
}
QByteArray QDataStreamSerializer::serializeSB1Info(const SIB1Info& sib1) const
{
    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << sib1.gnb_id;
    ds << sib1.cell_config.tac;
    ds << sib1.cell_config.minRxLevel;
    ds << sib1.cell_config.plmns_size;
    for (const auto [mcc, mnc] : sib1.cell_config.plmns) {
        ds << mcc;
        ds << mnc;
    }

    return payload;
}

SIB1Info QDataStreamSerializer::deserializeSB1Info(
    const QByteArray& payload) const
{
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    SIB1Info sib1;
    const uint8_t plmns_size{};
    ds >> sib1.gnb_id >> sib1.cell_config.tac >> sib1.cell_config.minRxLevel >>
        sib1.cell_config.plmns_size;

    sib1.cell_config.plmns.reserve(sib1.cell_config.plmns_size);
    for (size_t i = 0; i < sib1.cell_config.plmns_size; ++i) {
        uint32_t mcc;
        uint32_t mnc;
        ds >> mcc >> mnc;
        qDebug() << "mcc: " << mcc << ", mnc: " << mnc;
        sib1.cell_config.plmns.push_back({mcc, mnc});
    }

    return sib1;
}

QByteArray QDataStreamSerializer::serializeRegistrationPayload(
    double radius) const
{
    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << radius;
    return payload;
}

QByteArray QDataStreamSerializer::serializeTriggerHandover(
    const HandoverInfo info) const
{
    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << static_cast<uint32_t>(info.gnb_id);

    return payload;
}

HandoverInfo QDataStreamSerializer::deserializeTriggerHandover(
    const QByteArray& payload) const
{
    HandoverInfo info;
    QDataStream ds(payload);
    ds.setByteOrder(QDataStream::BigEndian);

    ChatMessageInfo message;
    ds >> info.gnb_id;
    return info;
}
