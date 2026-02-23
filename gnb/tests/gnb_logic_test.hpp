#ifndef GNB_LOGIC_TEST_HPP
#define GNB_LOGIC_TEST_HPP

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDataStream>
#include <QDateTime>

#include "gnb_logic.hpp"
#include "types.hpp"

using namespace ::testing;

MATCHER_P4(HasSib1Data, gnb_id, tac, mcc, mnc, "")
{
    QDataStream ds(arg);
    ds.setByteOrder(QDataStream::BigEndian);
    uint32_t r_id;
    uint16_t r_tac;
    int16_t r_minRx, r_mcc, r_mnc;

    ds >> r_id >> r_tac >> r_minRx >> r_mcc >> r_mnc;

    return r_id == gnb_id && r_tac == tac && r_mcc == mcc && r_mnc == mnc;
}

MATCHER_P2(HasRarData, expected_ra_rnti, expected_t_crnti, "")
{
    QDataStream ds(arg);
    ds.setByteOrder(QDataStream::BigEndian);
    uint16_t r_ra_rnti, r_t_crnti, r_ta;

    ds >> r_ra_rnti >> r_t_crnti >> r_ta;

    return r_ra_rnti == expected_ra_rnti && r_t_crnti == expected_t_crnti;
}

MATCHER_P2(HasRegistrationResponse, expected_status, expected_crnti, "")
{
    QDataStream ds(arg);
    ds.setByteOrder(QDataStream::BigEndian);
    uint8_t r_status;
    uint16_t r_crnti;

    ds >> r_status >> r_crnti;

    return r_status == static_cast<uint8_t>(expected_status) &&
           r_crnti == expected_crnti;
}

class MockGnbLogic : public GnbLogic
{
public:
    MockGnbLogic(uint32_t id)
        : GnbLogic(id)
    {
    }

    MOCK_METHOD(void, sendSimData,
                (ProtocolMsgType type, const QByteArray& payload,
                 uint32_t receiver_id),
                (override));

    using GnbLogic::cellConfig_;
    using GnbLogic::handleRegistrationRequest;
    using GnbLogic::onProtocolMessageReceived;
    using GnbLogic::onTick;
    using GnbLogic::sendBroadcastInfo;
    using GnbLogic::ue_contexts_;
};

#endif  // GNB_LOGIC_TEST_HPP
