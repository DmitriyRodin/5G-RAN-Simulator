#include "protobuf_serializer.hpp"

#include <QDebug>

#include "ran_messages.pb.h"
#include "serializer_factory.hpp"
#include "serializer_type.hpp"

enum class SerializerType : uint8_t;

namespace {
QByteArray toQByteArray(const std::string& str)
{
    return QByteArray(str.data(), static_cast<int>(str.size()));
}
}  // namespace

QByteArray ProtobufSerializer::serializeRrcSetupRequest(
    const RrcSetupRequest& info) const
{
    ran::protocol::RrcSetupRequest proto_msg;
    proto_msg.set_ue_identity(info.ue_identity);
    proto_msg.set_cause(static_cast<uint32_t>(info.cause));

    return toQByteArray(proto_msg.SerializeAsString());
}

std::optional<RrcSetupRequest> ProtobufSerializer::deserializeRrcSetupRequest(
    const QByteArray& payload) const
{
    ran::protocol::RrcSetupRequest proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }
    return RrcSetupRequest{proto_msg.ue_identity(),
                           static_cast<uint8_t>(proto_msg.cause())};
}

QByteArray ProtobufSerializer::serializeRachPreamble(
    const uint16_t& ra_rnti) const
{
    ran::protocol::RachPreambleInfo proto_msg;
    proto_msg.set_ra_rnti(ra_rnti);
    return toQByteArray(proto_msg.SerializeAsString());
}

std::optional<RachPreambleInfo> ProtobufSerializer::deserializeRachPreamble(
    const QByteArray& payload) const
{
    ran::protocol::RachPreambleInfo proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }
    return RachPreambleInfo{static_cast<uint16_t>(proto_msg.ra_rnti())};
}

QByteArray ProtobufSerializer::serializeRar(const RarInfo& info) const
{
    ran::protocol::RarInfo proto_msg;
    proto_msg.set_ra_rnti(info.ra_rnti);
    proto_msg.set_temp_c_rnti(info.temp_c_rnti);
    proto_msg.set_timing_advance(info.timing_advance);
    return toQByteArray(proto_msg.SerializeAsString());
}

std::optional<RarInfo> ProtobufSerializer::deserializeRar(
    const QByteArray& payload) const
{
    ran::protocol::RarInfo proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }
    RarInfo info;
    info.ra_rnti = static_cast<uint16_t>(proto_msg.ra_rnti());
    info.temp_c_rnti = static_cast<uint16_t>(proto_msg.temp_c_rnti());
    info.timing_advance = proto_msg.timing_advance();
    return info;
}

QByteArray ProtobufSerializer::serializeRrcSetup(const RrcSetupInfo& info) const
{
    ran::protocol::RrcSetupInfo proto_msg;
    proto_msg.set_received_identity(info.received_identity);
    proto_msg.set_config_status(info.config_status);
    return toQByteArray(proto_msg.SerializeAsString());
}

std::optional<RrcSetupInfo> ProtobufSerializer::deserializeRrcSetup(
    const QByteArray& payload) const
{
    ran::protocol::RrcSetupInfo proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }
    RrcSetupInfo info;
    info.received_identity = proto_msg.received_identity();
    info.config_status = static_cast<uint8_t>(proto_msg.config_status());
    return info;
}

QByteArray ProtobufSerializer::serializeRrcSetupComplete(
    const RrcSetupCompleteInfo& info) const
{
    ran::protocol::RrcSetupCompleteInfo proto_msg;
    auto* plmn = proto_msg.mutable_plmn();
    plmn->set_mcc(info.plmn.mcc);
    plmn->set_mnc(info.plmn.mnc);
    return toQByteArray(proto_msg.SerializeAsString());
}

std::optional<RrcSetupCompleteInfo>
ProtobufSerializer::deserializeRrcSetupComplete(const QByteArray& payload) const
{
    ran::protocol::RrcSetupCompleteInfo proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }
    RrcSetupCompleteInfo info;
    info.plmn.mcc = proto_msg.plmn().mcc();
    info.plmn.mnc = proto_msg.plmn().mnc();
    return info;
}

QByteArray ProtobufSerializer::serializeRrcRelease(
    const RrcReleaseCause& cause) const
{
    ran::protocol::RrcReconfigurationInfo proto_msg;
    proto_msg.set_target_gnb_id(static_cast<uint32_t>(cause));
    return toQByteArray(proto_msg.SerializeAsString());
}

std::optional<RrcReleaseCause> ProtobufSerializer::deserializeRrcRelease(
    const QByteArray& payload) const
{
    ran::protocol::RrcReconfigurationInfo proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }
    return static_cast<RrcReleaseCause>(proto_msg.target_gnb_id());
}

QByteArray ProtobufSerializer::serializeRegistrationRequest(
    const RegistrationRequestInfo& info) const
{
    ran::protocol::RegistrationRequestInfo proto_msg;
    proto_msg.set_ue_id(info.ue_id);
    proto_msg.set_ue_cap(info.ue_cap.toStdString());
    return toQByteArray(proto_msg.SerializeAsString());
}

std::optional<RegistrationRequestInfo>
ProtobufSerializer::deserializeRegistrationRequest(
    const QByteArray& payload) const
{
    ran::protocol::RegistrationRequestInfo proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }
    RegistrationRequestInfo info;
    info.ue_id = proto_msg.ue_id();
    info.ue_cap = QString::fromStdString(proto_msg.ue_cap());
    return info;
}

QByteArray ProtobufSerializer::serializeRegistrationAnswer(
    const RegistrationAnswerInfo& info) const
{
    ran::protocol::RegistrationAnswerInfo proto_msg;
    proto_msg.set_status(static_cast<uint32_t>(info.status));
    if (info.reject_reason.has_value()) {
        proto_msg.set_reject_reason(info.reject_reason.value().toStdString());
    }
    return toQByteArray(proto_msg.SerializeAsString());
}

std::optional<RegistrationAnswerInfo>
ProtobufSerializer::deserializeRegistrationAnswer(
    const QByteArray& payload) const
{
    ran::protocol::RegistrationAnswerInfo proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }
    RegistrationAnswerInfo info;
    info.status = static_cast<RegistrationStatus>(proto_msg.status());
    if (proto_msg.has_reject_reason()) {
        info.reject_reason = QString::fromStdString(proto_msg.reject_reason());
    }
    return info;
}

QByteArray ProtobufSerializer::serializeMeasurementReport(
    const MeasurementReportInfo& info) const
{
    ran::protocol::MeasurementReportInfo proto_msg;
    proto_msg.set_reported_gnb_id(info.reported_gnb_id);
    proto_msg.set_rsrp(info.rsrp);
    return toQByteArray(proto_msg.SerializeAsString());
}

std::optional<MeasurementReportInfo>
ProtobufSerializer::deserializeMeasurementReport(
    const QByteArray& payload) const
{
    ran::protocol::MeasurementReportInfo proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }
    MeasurementReportInfo info;
    info.reported_gnb_id = proto_msg.reported_gnb_id();
    info.rsrp = proto_msg.rsrp();
    return info;
}

std::optional<RrcReconfigurationInfo>
ProtobufSerializer::deserializeRrcReconfiguration(
    const QByteArray& payload) const
{
    ran::protocol::RrcReconfigurationInfo proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }
    return RrcReconfigurationInfo{proto_msg.target_gnb_id()};
}

QByteArray ProtobufSerializer::serializeChatMessage(
    const ChatMessageInfo& message) const
{
    ran::protocol::ChatMessageInfo proto_msg;
    proto_msg.set_receiver_ue_id(message.receiver_ue_id);
    proto_msg.set_sender_ue_id(message.sender_ue_id);
    proto_msg.set_text(message.text.toStdString());
    return toQByteArray(proto_msg.SerializeAsString());
}

std::optional<ChatMessageInfo> ProtobufSerializer::deserializeChatMessage(
    const QByteArray& payload) const
{
    ran::protocol::ChatMessageInfo proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }
    ChatMessageInfo message;
    message.receiver_ue_id = proto_msg.receiver_ue_id();
    message.sender_ue_id = proto_msg.sender_ue_id();
    message.text = QString::fromStdString(proto_msg.text());
    return message;
}

QByteArray ProtobufSerializer::serializeSB1Info(const SIB1Info& sib1) const
{
    ran::protocol::SIB1Info proto_msg;
    proto_msg.set_gnb_id(sib1.gnb_id);

    auto* config = proto_msg.mutable_cell_config();
    config->set_tac(sib1.cell_config.tac);
    config->set_min_rx_level(sib1.cell_config.minRxLevel);

    for (const auto& [mcc, mnc] : sib1.cell_config.plmns) {
        auto* item = config->add_plmns();
        item->set_mcc(mcc);
        item->set_mnc(mnc);
    }
    return toQByteArray(proto_msg.SerializeAsString());
}

std::optional<SIB1Info> ProtobufSerializer::deserializeSB1Info(
    const QByteArray& payload) const
{
    ran::protocol::SIB1Info proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }

    SIB1Info sib1;
    sib1.gnb_id = proto_msg.gnb_id();
    sib1.cell_config.tac = proto_msg.cell_config().tac();
    sib1.cell_config.minRxLevel = proto_msg.cell_config().min_rx_level();

    sib1.cell_config.plmns_size = proto_msg.cell_config().plmns_size();
    sib1.cell_config.plmns.reserve(sib1.cell_config.plmns_size);

    for (int i = 0; i < proto_msg.cell_config().plmns_size(); ++i) {
        const auto& item = proto_msg.cell_config().plmns(i);
        sib1.cell_config.plmns.push_back({item.mcc(), item.mnc()});
    }
    return sib1;
}

QByteArray ProtobufSerializer::serializeRegistrationPayload(
    const double radius) const
{
    ran::protocol::HubRegistrationPayload proto_msg;
    proto_msg.set_radius(radius);
    return toQByteArray(proto_msg.SerializeAsString());
}

QByteArray ProtobufSerializer::serializeTriggerHandover(
    const HandoverInfo info) const
{
    ran::protocol::HandoverInfo proto_msg;
    proto_msg.set_gnb_id(info.gnb_id);
    return toQByteArray(proto_msg.SerializeAsString());
}

std::optional<HandoverInfo> ProtobufSerializer::deserializeTriggerHandover(
    const QByteArray& payload) const
{
    ran::protocol::HandoverInfo proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }
    HandoverInfo info;
    info.gnb_id = proto_msg.gnb_id();
    return info;
}

std::optional<HubRegistrationResponce>
ProtobufSerializer::deserializeHubRegistrationResponce(
    const QByteArray& payload) const
{
    ran::protocol::HubRegistrationResponse proto_msg;
    if (!proto_msg.ParseFromArray(payload.constData(), payload.size())) {
        return std::nullopt;
    }
    HubRegistrationResponce responce;
    responce.status = proto_msg.status();
    return responce;
}

static bool protobuf_registered = SerializerFactory::registerSerializer(
    SerializerType::Protobuf,
    []() { return std::make_unique<ProtobufSerializer>(); });
