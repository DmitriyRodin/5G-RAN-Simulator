#ifndef ISERIALIZER_HPP
#define ISERIALIZER_HPP

#include <QByteArray>

#include "types.hpp"

class ISerializer
{
public:
    virtual ~ISerializer() = default;

    virtual QByteArray serializeSB1Info(const SIB1Info& sib1) const = 0;
    virtual SIB1Info deserializeSB1Info(const QByteArray& payload) const = 0;

    virtual QByteArray serializeRrcSetupRequest(
        const RrcSetupRequest& info) const = 0;

    virtual RrcSetupRequest deserializeRrcSetupRequest(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRachPreamble(const uint16_t& ra_rnti) const = 0;
    virtual RachPreambleInfo deserializeRachPreamble(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRar(const RarInfo& info) const = 0;
    virtual std::optional<RarInfo> deserializeRar(
        const QByteArray& payload, const uint16_t& last_rach_ra_rnti) const = 0;

    virtual QByteArray serializeRrcSetup(const RrcSetupInfo& info) const = 0;
    virtual RrcSetupInfo deserializeRrcSetup(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRrcSetupComplete(
        const RrcSetupCompleteInfo& info) const = 0;
    virtual RrcSetupCompleteInfo deserializeRrcSetupComplete(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRrcRelease(
        const RrcReleaseCause& info) const = 0;
    virtual RrcReleaseCause deserializeRrcRelease(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRegistrationRequest(
        const RegistrationRequestInfo& info) const = 0;
    virtual RegistrationRequestInfo deserializeRegistrationRequest(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeRegistrationAnswer(
        const RegistrationAnswerInfo& info) const = 0;
    virtual RegistrationAnswerInfo deserializeRegistrationAnswer(
        const QByteArray& payload) const = 0;

    virtual QByteArray serializeMeasurementReport(
        const double& rsrp, const uint32_t& gnb_id) const = 0;
    virtual std::optional<RrcReconfigurationInfo> deserializeRrcReconfiguration(
        const QByteArray& payload) const = 0;
    virtual QByteArray serializeChatMessage(
        const ChatMessageInfo& info) const = 0;
    virtual ChatMessageInfo deserializeChatMessage(
        const QByteArray& payload) const = 0;
};

#endif  // ISERIALIZER_HPP
