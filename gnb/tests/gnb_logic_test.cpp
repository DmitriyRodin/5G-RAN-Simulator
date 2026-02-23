#include "gnb_logic_test.hpp"

class GnbLogicTest : public Test
{
protected:
    void SetUp() override
    {
        gnb = new StrictMock<MockGnbLogic>(101);

        config.tac = 123;
        config.mcc = 255;
        config.mnc = 1;
        gnb->setCellConfig(config);
    }

    void TearDown() override
    {
        delete gnb;
    }

    StrictMock<MockGnbLogic>* gnb;
    GnbCellConfig config;
};

TEST_F(GnbLogicTest, SIB1_Broadcast_Validation)
{
    EXPECT_CALL(*gnb, sendSimData(ProtocolMsgType::Sib1,
                                  HasSib1Data(101, 123, 255, 1), 0xFFFFFFFF))
        .Times(1);

    gnb->sendBroadcastInfo();
}

TEST_F(GnbLogicTest, RACH_Procedure_Msg1_To_Msg2)
{
    uint32_t ue_id = 777;
    uint16_t ra_rnti = 5;

    QByteArray msg1;
    QDataStream out(&msg1, QIODevice::WriteOnly);
    out << ra_rnti;

    uint16_t expected_t_crnti = static_cast<uint16_t>(1000 + (ue_id % 9000));

    EXPECT_CALL(*gnb, sendSimData(ProtocolMsgType::Rar,
                                  HasRarData(ra_rnti, expected_t_crnti), ue_id))
        .Times(1);

    gnb->onProtocolMessageReceived(ue_id, ProtocolMsgType::RachPreamble, msg1);

    ASSERT_TRUE(gnb->ue_contexts_.contains(ue_id));
    EXPECT_EQ(gnb->ue_contexts_[ue_id].crnti, expected_t_crnti);
}

TEST_F(GnbLogicTest, RRC_Connection_Setup_Success)
{
    uint32_t ue_id = 777;
    uint16_t crnti = 1777;

    UeContext ctx;
    ctx.id = ue_id;
    ctx.crnti = crnti;
    ctx.last_activity = QDateTime::currentDateTime();
    ctx.is_attached = false;
    ctx.state = UeRrcState::DETACHED;

    gnb->ue_contexts_[ue_id] = ctx;

    QByteArray msg3;
    QDataStream out(&msg3, QIODevice::WriteOnly);
    uint64_t ue_identity = 987654321;

    out << static_cast<quint64>(ue_identity)
        << static_cast<uint8_t>(0);  // Identity + EstablishmentCause

    EXPECT_CALL(*gnb, sendSimData(ProtocolMsgType::RrcSetup, _, ue_id))
        .Times(1);

    gnb->onProtocolMessageReceived(ue_id, ProtocolMsgType::RrcSetupRequest,
                                   msg3);
}

TEST_F(GnbLogicTest, Handover_Trigger_On_MeasurementReport)
{
    uint32_t ue_id = 777;

    UeContext ctx;
    ctx.id = ue_id;
    ctx.last_activity = QDateTime::currentDateTime();
    ctx.is_attached = false;
    ctx.state = UeRrcState::DETACHED;

    gnb->ue_contexts_[ue_id] = ctx;
    gnb->ue_contexts_[ue_id].last_rssi = -90.0;

    QByteArray report;
    QDataStream out(&report, QIODevice::WriteOnly);
    out << (uint32_t)102 << (double)-80.0;

    EXPECT_CALL(*gnb,
                sendSimData(ProtocolMsgType::RrcReconfiguration, _, ue_id))
        .Times(1);

    gnb->onProtocolMessageReceived(ue_id, ProtocolMsgType::MeasurementReport,
                                   report);
}

TEST_F(GnbLogicTest, Inactivity_Timeout_Release)
{
    uint32_t ue_id = 888;
    UeContext ctx;
    ctx.id = ue_id;
    ctx.state = UeRrcState::RRC_CONNECTED;
    ctx.last_activity = QDateTime::currentDateTime().addSecs(-40);
    gnb->ue_contexts_[ue_id] = ctx;

    EXPECT_CALL(*gnb, sendSimData(ProtocolMsgType::RrcRelease, _, ue_id))
        .Times(1);

    gnb->onTick();
}

TEST_F(GnbLogicTest, Handle_Registration_Request_Success)
{
    uint32_t ue_id = 999;

    QByteArray emmpty_payload;

    EXPECT_CALL(*gnb, sendSimData(ProtocolMsgType::RrcSetup,
                                  HasRegistrationResponse(
                                      RegistrationStatus::Accepted, 42),
                                  ue_id))
        .Times(1);

    gnb->onProtocolMessageReceived(ue_id, ProtocolMsgType::RegistrationRequest,
                                   emmpty_payload);
}
