#include "sim_protocol.hpp"

#include <gtest/gtest.h>

#include <QByteArray>
#include <QPointF>

#include "types.hpp"

using namespace SimProtocol;

class SimProtocolTest : public ::testing::Test
{
protected:
    const uint32_t TEST_UE_ID = 544;
    const uint32_t TEST_GNB_ID = 7;
    const EntityType TEST_UE_TYPE = EntityType::UE;
    const SimMessageType TEST_MESSAGE_TYPE = SimMessageType::Registration;
    const QPointF TEST_POS = QPointF(77.7, -444.44);
    const QByteArray TEST_PAYLOAD =
        "Protocol_Verification_Payload_5G-RAN-Simulator";
    const uint32_t HUB_ID = 0;
    const uint32_t BROADCAST_ID = 4294967295;
    const QPointF HUB_VIRTUAL_POS{0.0, 0.0};
};

TEST_F(SimProtocolTest, SerializationSymmetry)
{
    QByteArray raw_data =
        buildPacket(TEST_UE_ID, TEST_UE_TYPE, TEST_GNB_ID, TEST_MESSAGE_TYPE,
                    TEST_POS, TEST_PAYLOAD);
    DecodedPacket decoded = parse(raw_data);

    ASSERT_TRUE(decoded.isValid);
    EXPECT_EQ(decoded.srcId, TEST_UE_ID);
    EXPECT_EQ(decoded.dstId, TEST_GNB_ID);
    EXPECT_EQ(decoded.nodeType, TEST_UE_TYPE);
    EXPECT_EQ(decoded.type, TEST_MESSAGE_TYPE);

    EXPECT_DOUBLE_EQ(decoded.position.x(), TEST_POS.x());
    EXPECT_DOUBLE_EQ(decoded.position.y(), TEST_POS.y());

    EXPECT_EQ(decoded.payload, TEST_PAYLOAD);
}

TEST_F(SimProtocolTest, RoutingLogicBroadcast)
{
    QByteArray raw_data =
        buildPacket(TEST_GNB_ID, TEST_UE_TYPE, BROADCAST_ID, TEST_MESSAGE_TYPE,
                    TEST_POS, TEST_PAYLOAD);
    DecodedPacket decoded = parse(raw_data);

    EXPECT_TRUE(decoded.isBroadcast(BROADCAST_ID));
    EXPECT_FALSE(decoded.isForHub(HUB_ID));
}

TEST_F(SimProtocolTest, RoutingLogicTargetMe)
{
    QByteArray raw_data =
        buildPacket(TEST_GNB_ID, EntityType::GNB, TEST_UE_ID,
                    SimMessageType::RegistrationResponse, TEST_POS);
    DecodedPacket decoded = parse(raw_data);

    EXPECT_TRUE(decoded.isForMe(TEST_UE_ID, BROADCAST_ID));
    EXPECT_FALSE(decoded.isForMe(123321, BROADCAST_ID));
}

TEST_F(SimProtocolTest, HandlesEmptyPayload)
{
    QByteArray raw_data =
        buildPacket(TEST_UE_ID, TEST_UE_TYPE, TEST_GNB_ID, TEST_MESSAGE_TYPE,
                    TEST_POS, QByteArray());
    DecodedPacket decoded = parse(raw_data);

    ASSERT_TRUE(decoded.isValid);
    EXPECT_TRUE(decoded.payload.isEmpty());
}

TEST_F(SimProtocolTest, RejectsIncompleteHeader)
{
    QByteArray data_less_than_min_header = "too_short";
    DecodedPacket decoded = parse(data_less_than_min_header);

    EXPECT_FALSE(decoded.isValid);
}

TEST_F(SimProtocolTest, IdentifiesHubMessages)
{
    QByteArray raw_data =
        buildPacket(HUB_ID, EntityType::RadioHub, TEST_UE_ID,
                    SimMessageType::RegistrationResponse, HUB_VIRTUAL_POS);
    DecodedPacket decoded = parse(raw_data);

    EXPECT_TRUE(decoded.isFromHub(HUB_ID));
    EXPECT_EQ(decoded.srcId, HUB_ID);
}
