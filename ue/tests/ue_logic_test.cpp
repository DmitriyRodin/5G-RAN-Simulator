#include "ue_logic_test.hpp"

class UeLogicTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ue = new UeLogicTestWrapper(101);
    }

    void TearDown() override
    {
        delete ue;
    }

    UeLogicTestWrapper* ue;
};

TEST_F(UeLogicTest, InitialStateIsDetached)
{
    EXPECT_EQ(ue->getCurrentState(), UeRrcState::DETACHED);
}

TEST_F(UeLogicTest, HandleSib1Transition)
{
    emit ue->registrationAtRadioHubConfirmed();

    QTest::qWait(3100);

    ue->onProtocolMessageReceived(50, ProtocolMsgType::Sib1, QByteArray());

    EXPECT_EQ(ue->getCurrentState(), UeRrcState::RRC_CONNECTING);

    ASSERT_FALSE(ue->sent_messages.isEmpty());
    EXPECT_EQ(ue->sent_messages.last().type, ProtocolMsgType::RachPreamble);
}

TEST_F(UeLogicTest, SearchingForCellDelayVerification)
{
    emit ue->registrationAtRadioHubConfirmed();

    EXPECT_EQ(ue->getCurrentState(), UeRrcState::DETACHED);

    QTest::qWait(1500);

    EXPECT_EQ(ue->getCurrentState(), UeRrcState::DETACHED);
    qDebug() << "Checked at 1500ms: State is still DETACHED. Good.";

    QTest::qWait(800);

    EXPECT_EQ(ue->getCurrentState(), UeRrcState::SEARCHING_FOR_CELL);
    qDebug() << "Checked at 2300ms: State changed to SEARCHING_FOR_CELL.";
}

TEST_F(UeLogicTest, HandleRarFailure)
{
    ue->setState(UeRrcState::DETACHED);

    QByteArray rar_payload;
    ue->onProtocolMessageReceived(50, ProtocolMsgType::Rar, rar_payload);
    EXPECT_NE(UeRrcState::RRC_CONNECTING, ue->getCurrentState());
}

TEST_F(UeLogicTest, HandleRarSuccess)
{
    QByteArray rar_payload;
    QDataStream ds(&rar_payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    uint16_t ra_rnti = 101 % 65535;
    uint16_t t_crnti = 0x1234;
    ds << ra_rnti << t_crnti << (uint16_t)0;
    ue->last_rach_ra_rnti_ = 101;

    ue->setState(UeRrcState::RRC_CONNECTING);
    ue->onProtocolMessageReceived(50, ProtocolMsgType::Rar, rar_payload);

    ASSERT_FALSE(ue->sent_messages.isEmpty());
    EXPECT_EQ(ue->sent_messages.last().type, ProtocolMsgType::RrcSetupRequest);
}

TEST_F(UeLogicTest, ChatMessageValidation)
{
    ue->sendChatMessage(202, "Hello");

    for (const auto& msg : ue->sent_messages) {
        EXPECT_NE(msg.type, ProtocolMsgType::UserPlaneData);
    }
}

TEST_F(UeLogicTest, MeasurementReportTimer)
{
    ue->run();

    QTest::qWait(600);

    bool found_report = false;
    for (const auto& msg : ue->sent_messages) {
        if (msg.type == ProtocolMsgType::MeasurementReport) {
            found_report = true;
        }
    }
    EXPECT_FALSE(found_report);
}

TEST_F(UeLogicTest, HandleRrcSetupSuccess)
{
    ue->state_ = UeRrcState::RRC_CONNECTING;
    ue->target_gnb_id_ = 50;
    ue->sent_msg3_identity_ = 101;

    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << (quint64)101 << (uint8_t)1;

    ue->onProtocolMessageReceived(50, ProtocolMsgType::RrcSetup, payload);

    EXPECT_EQ(ue->state_, UeRrcState::RRC_CONNECTED);

    ASSERT_FALSE(ue->sent_messages.isEmpty());
    EXPECT_EQ(ue->sent_messages.last().type, ProtocolMsgType::RrcSetupComplete);
}

TEST_F(UeLogicTest, HandleRrcSetupContentionFailure)
{
    ue->state_ = UeRrcState::RRC_CONNECTING;
    ue->target_gnb_id_ = 50;
    ue->sent_msg3_identity_ = 101;

    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << (quint64)999 << (uint8_t)1;  // Чужой ID!

    ue->onProtocolMessageReceived(50, ProtocolMsgType::RrcSetup, payload);

    EXPECT_EQ(ue->state_, UeRrcState::RRC_IDLE);
    EXPECT_EQ(ue->crnti_, 0);
}

TEST_F(UeLogicTest, SendRegistrationRequestPayload)
{
    ue->state_ = UeRrcState::RRC_CONNECTED;
    ue->target_gnb_id_ = 50;

    ue->sendRegistrationRequest();

    ASSERT_FALSE(ue->sent_messages.isEmpty());
    auto msg = ue->sent_messages.last();
    EXPECT_EQ(msg.type, ProtocolMsgType::RegistrationRequest);

    QDataStream in(msg.payload);
    in.setByteOrder(QDataStream::BigEndian);
    QString model;
    uint16_t id;
    in >> model >> id;

    EXPECT_STREQ(model.toStdString().c_str(), "UE-Capabilities-Model-X");
    EXPECT_EQ(id, 101);
}

TEST_F(UeLogicTest, HandleRrcReleaseAndRestart)
{
    ue->state_ = UeRrcState::RRC_CONNECTED;
    ue->target_gnb_id_ = 50;

    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds << (uint8_t)RrcReleaseCause::Other;

    ue->onProtocolMessageReceived(50, ProtocolMsgType::RrcRelease, payload);

    EXPECT_EQ(ue->state_, UeRrcState::DETACHED);
    EXPECT_EQ(ue->target_gnb_id_, 0);
    QTest::qWait(2500);
    EXPECT_EQ(ue->state_, UeRrcState::SEARCHING_FOR_CELL);
}

TEST_F(UeLogicTest, HandleHandoverReconfiguration)
{
    ue->state_ = UeRrcState::RRC_CONNECTED;
    const uint32_t CURRENT_GNB{50};
    ue->target_gnb_id_ = CURRENT_GNB;

    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    const uint32_t TARGET_GNB{60};
    ds << TARGET_GNB;

    ue->onProtocolMessageReceived(CURRENT_GNB,
                                  ProtocolMsgType::RrcReconfiguration, payload);

    EXPECT_EQ(ue->target_gnb_id_, TARGET_GNB);
    EXPECT_EQ(ue->state_, UeRrcState::RRC_CONNECTING);

    ASSERT_FALSE(ue->sent_messages.isEmpty());
    EXPECT_EQ(ue->sent_messages.last().type, ProtocolMsgType::RachPreamble);
    EXPECT_EQ(ue->sent_messages.last().dest, TARGET_GNB);
}

TEST_F(UeLogicTest, HandleRegistrationAcceptSuccess)
{
    ue->state_ = UeRrcState::RRC_CONNECTED;
    ue->is_registered_ = false;

    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    uint32_t expected_tmsi = 0xABCDE123;
    ds << expected_tmsi;

    ue->onProtocolMessageReceived(50, ProtocolMsgType::RegistrationAccept,
                                  payload);

    EXPECT_TRUE(ue->is_registered_);

    qDebug() << "Registration Accept processed. UE is now registered.";
}
