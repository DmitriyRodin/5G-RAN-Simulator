#ifndef ISERIALIZER_HPP
#define ISERIALIZER_HPP

#include <QByteArray>

#include "types.hpp"

class ISerializer
{
public:
    virtual ~ISerializer() = default;

    virtual QByteArray serializeSB1Info(const SIB1Info& sib1) const = 0;
    virtual std::optional<SIB1Info> deserializeSB1Info(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRrcSetupRequest(
        const RrcSetupRequest& info) const = 0;

    virtual std::optional<RrcSetupRequest> deserializeRrcSetupRequest(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRachPreamble(const uint16_t& ra_rnti) const = 0;
    virtual std::optional<RachPreambleInfo> deserializeRachPreamble(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRar(const RarInfo& info) const = 0;
    virtual std::optional<RarInfo> deserializeRar(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRrcSetup(const RrcSetupInfo& info) const = 0;
    virtual std::optional<RrcSetupInfo> deserializeRrcSetup(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRrcSetupComplete(
        const RrcSetupCompleteInfo& info) const = 0;
    virtual std::optional<RrcSetupCompleteInfo> deserializeRrcSetupComplete(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRrcRelease(
        const RrcReleaseCause& info) const = 0;
    virtual std::optional<RrcReleaseCause> deserializeRrcRelease(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRegistrationRequest(
        const RegistrationRequestInfo& info) const = 0;
    virtual std::optional<RegistrationRequestInfo>
    deserializeRegistrationRequest(const QByteArray& payload) const = 0;

    virtual QByteArray serializeRegistrationAnswer(
        const RegistrationAnswerInfo& info) const = 0;
    virtual std::optional<RegistrationAnswerInfo> deserializeRegistrationAnswer(
        const QByteArray& payload) const = 0;

    virtual std::optional<RrcReconfigurationInfo> deserializeRrcReconfiguration(
        const QByteArray& payload) const = 0;
    virtual QByteArray serializeChatMessage(
        const ChatMessageInfo& info) const = 0;
    virtual std::optional<ChatMessageInfo> deserializeChatMessage(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeMeasurementReport(
        const MeasurementReportInfo& info) const = 0;
    virtual std::optional<MeasurementReportInfo> deserializeMeasurementReport(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRegistrationPayload(
        const double radius) const = 0;

    virtual QByteArray serializeTriggerHandover(
        const HandoverInfo info) const = 0;
    virtual std::optional<HandoverInfo> deserializeTriggerHandover(
        const QByteArray& payload) const = 0;
};

#endif  // ISERIALIZER_HPP
