#ifndef UE_LOGIC_TEST_HPP
#define UE_LOGIC_TEST_HPP

#include <gtest/gtest.h>

#include <QDataStream>
#include <QSignalSpy>
#include <QtTest/QTest>

#include "ue_logic.hpp"

namespace TestData {
constexpr uint32_t UE_ID = 101;
constexpr double RADIUS = 1200.0;
constexpr int RADIO_FRAME_DURATION = 10;
constexpr double TXP = 5.0;
constexpr uint32_t HUB_ID = 0;
constexpr uint16_t HUB_PORT = 5555;
constexpr uint32_t BROADCAST_ID = 0xFFFFFFFF;
constexpr Point2D VIRT_POS{0.0, 0.0};
const std::string ADDRESS("127.0.0.1");
const RadioSettings RADIO{RADIO_FRAME_DURATION, TXP};
constexpr Cell CELL{100};
const HubSettings HUB_SET{HUB_PORT, HUB_ID, BROADCAST_ID, VIRT_POS, ADDRESS};
const UeSettings UE_SETTINGS(HUB_SET, RADIO, CELL);
};  // namespace TestData

class UeLogicTestWrapper : public UeLogic
{
public:
    UeLogicTestWrapper(uint32_t id, UeSettings set)
        : UeLogic(id, set)
    {
    }
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
    using UeLogic::serializer_;
    using UeLogic::state_;
    using UeLogic::target_gnb_id_;

    using UeLogic::sendRegistrationRequest;
};

#endif  // UE_LOGIC_TEST_HPP
