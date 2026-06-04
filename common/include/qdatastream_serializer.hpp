#ifndef QDATASTREAM_SERIALIZER_HPP
#define QDATASTREAM_SERIALIZER_HPP

#include <QDataStream>

#include "iserializer.hpp"
#include "types.hpp"

class QDataStreamSerializer : public ISerializer
{
public:
    QByteArray serializeRrcSetupRequest(
        const RrcSetupRequest& info) const override;

    std::optional<RrcSetupRequest> deserializeRrcSetupRequest(
        const QByteArray& payload) const override;
    QByteArray serializeRachPreamble(const uint16_t& ra_rnti) const override;
    std::optional<RachPreambleInfo> deserializeRachPreamble(
        const QByteArray& payload) const override;

    QByteArray serializeRar(const RarInfo& info) const override;
    std::optional<RarInfo> deserializeRar(
        const QByteArray& payload) const override;

    QByteArray serializeRrcSetup(const RrcSetupInfo& info) const override;
    std::optional<RrcSetupInfo> deserializeRrcSetup(
        const QByteArray& payload) const override;

    QByteArray serializeRrcSetupComplete(
        const RrcSetupCompleteInfo& info) const override;

    std::optional<RrcSetupCompleteInfo> deserializeRrcSetupComplete(
        const QByteArray& payload) const override;

    QByteArray serializeRrcRelease(const RrcReleaseCause& info) const override;
    std::optional<RrcReleaseCause> deserializeRrcRelease(
        const QByteArray& payload) const override;

    QByteArray serializeRegistrationRequest(
        const RegistrationRequestInfo& info) const override;
    std::optional<RegistrationRequestInfo> deserializeRegistrationRequest(
        const QByteArray& payload) const override;

    QByteArray serializeRegistrationAnswer(
        const RegistrationAnswerInfo& info) const override;
    std::optional<RegistrationAnswerInfo> deserializeRegistrationAnswer(
        const QByteArray& payload) const override;

    std::optional<RrcReconfigurationInfo> deserializeRrcReconfiguration(
        const QByteArray& payload) const override;
    QByteArray serializeChatMessage(
        const ChatMessageInfo& message) const override;
    std::optional<ChatMessageInfo> deserializeChatMessage(
        const QByteArray& payload) const override;

    QByteArray serializeSB1Info(const SIB1Info& sib1) const override;
    std::optional<SIB1Info> deserializeSB1Info(
        const QByteArray& payload) const override;

    QByteArray serializeMeasurementReport(
        const MeasurementReportInfo& info) const override;
    std::optional<MeasurementReportInfo> deserializeMeasurementReport(
        const QByteArray& payload) const override;

    QByteArray serializeRegistrationPayload(const double radius) const override;

    QByteArray serializeTriggerHandover(const HandoverInfo info) const override;
    std::optional<HandoverInfo> deserializeTriggerHandover(
        const QByteArray& payload) const override;
};

#endif  // QDATASTREAM_SERIALIZER_HPP
