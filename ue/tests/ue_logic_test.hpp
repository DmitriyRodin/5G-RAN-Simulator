#ifndef UE_LOGIC_TEST_HPP
#define UE_LOGIC_TEST_HPP

#include <gtest/gtest.h>

#include <QDataStream>
#include <QSignalSpy>
#include <QtTest/QTest>

#include "ue_logic.hpp"

class UeLogicTestWrapper : public UeLogic
{
public:
    using UeLogic::onProtocolMessageReceived;
    using UeLogic::UeLogic;

    struct OutgoingMsg {
        ProtocolMsgType type;
        QByteArray payload;
        uint32_t dest;
    };
    QList<OutgoingMsg> sent_messages;

    void sendSimData(ProtocolMsgType type, const QByteArray& data,
                     uint32_t dest_id) override
    {
        qDebug() << "sendSimData: type =" << (uint8_t)type;
        sent_messages.append({type, data, dest_id});
    }

    UeRrcState getCurrentState()
    {
        return state_;
    }

    void setState(UeRrcState state)
    {
        state_ = state;
    }

    using UeLogic::crnti_;
    using UeLogic::is_registered_;
    using UeLogic::last_rach_ra_rnti_;
    using UeLogic::sent_msg3_identity_;
    using UeLogic::state_;
    using UeLogic::target_gnb_id_;

    using UeLogic::sendRegistrationRequest;
};

#endif  // UE_LOGIC_TEST_HPP
