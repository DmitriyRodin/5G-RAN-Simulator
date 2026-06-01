#ifndef GNB_LOGIC_TEST_HPP
#define GNB_LOGIC_TEST_HPP

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDataStream>
#include <QDateTime>

#include "gnb_logic.hpp"
#include "settings.hpp"
#include "types.hpp"

using namespace ::testing;

namespace TestData {
constexpr uint32_t GNB_ID = 1;
constexpr double RADIUS = 1200.0;
constexpr int RADIO_FRAME_DURATION = 10;
constexpr double TXP = 43.0;
constexpr uint32_t HUB_ID = 0;
constexpr uint16_t HUB_PORT = 5555;
constexpr uint32_t BROADCAST_ID = 0xFFFFFFFF;
constexpr Point2D VIRT_POS{0.0, 0.0};
const std::string ADDRESS("127.0.0.1");
const RadioSettings RADIO{RADIO_FRAME_DURATION, TXP};
constexpr Cell CELL{100};
const HubSettings HUB_SET{HUB_PORT, HUB_ID, BROADCAST_ID, VIRT_POS, ADDRESS};
const GnbSettings GNB_SETTINGS(HUB_SET, RADIO, CELL, RADIUS);
};  // namespace TestData

MATCHER_P6(HasSib1Data, gnb_id, tac, rx_level, plmns_size, mcc, mnc, "")
{
    QDataStream ds(arg);
    ds.setByteOrder(QDataStream::BigEndian);
    uint32_t r_id;
    uint16_t r_tac;
    int16_t r_minRx;
    uint8_t r_size;
    int32_t r_mcc, r_mnc;

    ds >> r_id >> r_tac >> r_minRx >> r_size >> r_mcc >> r_mnc;

    return r_id == gnb_id && r_tac == tac && r_mcc == mcc && r_mnc == mnc &&
           r_size == plmns_size && rx_level == r_minRx;
}

MATCHER_P2(HasRarData, expected_ra_rnti, expected_t_crnti, "")
{
    QDataStream ds(arg);
    ds.setByteOrder(QDataStream::BigEndian);
    uint16_t r_ra_rnti, r_t_crnti, r_ta;

    ds >> r_ra_rnti >> r_t_crnti >> r_ta;

    return r_ra_rnti == expected_ra_rnti && r_t_crnti == expected_t_crnti;
}

MATCHER_P2(HasRegistrationResponse, expected_status, expected_reasone, "")
{
    QDataStream ds(arg);
    ds.setByteOrder(QDataStream::BigEndian);
    uint8_t r_status;
    QString r_reasone;

    if (!ds.atEnd()) {
        ds >> r_reasone;
        return r_status == static_cast<uint8_t>(expected_status) &&
               r_reasone == expected_reasone;
    }

    return r_status == static_cast<uint8_t>(expected_status);
}

class MockGnbLogic : public GnbLogic
{
public:
    MockGnbLogic(uint32_t id, GnbSettings set)
        : GnbLogic(id, set)
    {
    }

    MOCK_METHOD(void, sendSimData,
                (ProtocolMsgType type, const QByteArray& payload,
                 uint32_t receiver_id),
                (override));

    using BaseEntity::serializer_;
    using GnbLogic::cellConfig_;
    using GnbLogic::handleRegistrationRequest;
    using GnbLogic::onProtocolMessageReceived;
    using GnbLogic::onTick;
    using GnbLogic::sendBroadcastInfo;
    using GnbLogic::ue_contexts_;
};

#endif  // GNB_LOGIC_TEST_HPP
