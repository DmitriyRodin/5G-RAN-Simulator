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

    RrcSetupRequest deserializeRrcSetupRequest(
        const QByteArray& payload) const override;
    QByteArray serializeRachPreamble(const uint16_t& ra_rnti) const override;
    RachPreambleInfo deserializeRachPreamble(
        const QByteArray& payload) const override;

    QByteArray serializeRar(const RarInfo& info) const override;
    RarInfo deserializeRar(const QByteArray& payload) const override;

    QByteArray serializeRrcSetup(const RrcSetupInfo& info) const override;
    RrcSetupInfo deserializeRrcSetup(const QByteArray& payload) const override;

    QByteArray serializeRrcSetupComplete(
        const RrcSetupCompleteInfo& info) const override;

    RrcSetupCompleteInfo deserializeRrcSetupComplete(
        const QByteArray& payload) const override;

    QByteArray serializeRrcRelease(const RrcReleaseCause& info) const override;
    RrcReleaseCause deserializeRrcRelease(
        const QByteArray& payload) const override;

    QByteArray serializeRegistrationRequest(
        const RegistrationRequestInfo& info) const override;
    RegistrationRequestInfo deserializeRegistrationRequest(
        const QByteArray& payload) const override;

    QByteArray serializeRegistrationAnswer(
        const RegistrationAnswerInfo& info) const override;
    RegistrationAnswerInfo deserializeRegistrationAnswer(
        const QByteArray& payload) const override;

    std::optional<RrcReconfigurationInfo> deserializeRrcReconfiguration(
        const QByteArray& payload) const override;
    QByteArray serializeChatMessage(
        const ChatMessageInfo& message) const override;
    ChatMessageInfo deserializeChatMessage(
        const QByteArray& payload) const override;

    QByteArray serializeSB1Info(const SIB1Info& sib1) const override;
    SIB1Info deserializeSB1Info(const QByteArray& payload) const override;

    QByteArray serializeMeasurementReport(
        const MeasurementReportInfo& info) const override;
    MeasurementReportInfo deserializeMeasurementReport(
        const QByteArray& payload) const override;

    QByteArray serializeRegistrationPayload(const double radius) const override;

    QByteArray serializeTriggerHandover(const HandoverInfo info) const override;
    HandoverInfo deserializeTriggerHandover(
        const QByteArray& payload) const override;
};

#endif  // QDATASTREAM_SERIALIZER_HPP
