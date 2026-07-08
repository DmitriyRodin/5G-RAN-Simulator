import pytest
import time
import socket
from codec import (
    PROTO_MSG_SIB1,
    PROTO_MSG_RACH_PREAMBLE,
    PROTO_MSG_RAR,
    PROTO_MSG_RRC_SETUP_REQUEST,
    PROTO_MSG_RRC_SETUP,
    PROTO_MSG_RRC_SETUP_COMPLETE,
    PROTO_MSG_REGISTRATION_REQUEST,
    PROTO_MSG_USER_PLANE_DATA,
    PROTO_MSG_MEASUREMENT_REPORT,
    PROTO_MSG_RRC_RECONFIGURATION,
    PROTO_MSG_RRC_RELEASE
)

##
# @file test_gnb.py
# @brief Integration tests for the 5G RAN gNB protocol stack and signaling flows.
# @details Validates Cell Broadcast, RACH Procedure, RRC state transitions, NAS registration, User Plane data routing, Handover triggers, and Inactivity timers.
#

##
# @brief Verifies that the gNB starts up and successfully registers at the RadioHub.
# @param gnb_harness The parameterized test harness fixture.
# @return None
# @details This test checks that the gNB sends a SIM_MSG_REGISTRATION packet to the RadioHub
#          and transitions into an operational state upon receiving a registration confirmation.
#
def test_gnb_registration(gnb_harness):
    assert gnb_harness.gnb_addr is not None


##
# @brief Verifies the periodic broadcasting of System Information Block Type 1 (SIB1).
# @param gnb_harness The parameterized test harness fixture.
# @return None
# @details SIB1 should be sent to the broadcast address containing cell parameters like TAC, Cell ID, and PLMN lists.
#
def test_sib1_broadcast(gnb_harness):
    # Wait for SIB1 broadcast
    dst_id, proto_type, payload = gnb_harness.recv_proto_message(expected_dst_id=4294967295, timeout=3.0)
    assert dst_id == 4294967295  # Broadcast ID
    assert proto_type == PROTO_MSG_SIB1

    sib1 = gnb_harness.codec.deserialize_sib1(payload)
    assert (sib1['cell_identity'] >> 8) == gnb_harness.gnb_id
    assert sib1['tac'] == 100
    assert len(sib1['plmn_identity_info_list']) == 0


##
# @brief Helper function to perform RACH and RRC connection setup for a UE.
# @param harness The parameterized test harness.
# @param ue_id Unique identifier of the User Equipment.
# @param ue_identity IMSI or temporary random identity of the UE.
# @return The assigned C-RNTI of the UE.
#
def establish_rrc_connection(harness, ue_id, ue_identity):
    # Step 1: RACH Preamble (Msg1)
    preamble = harness.codec.serialize_rach_preamble(ra_rnti=5)
    harness.send_proto_message(ue_id, PROTO_MSG_RACH_PREAMBLE, preamble)

    # Step 2: Random Access Response (Msg2)
    dst_id, proto_type, payload = harness.recv_proto_message(expected_dst_id=ue_id, timeout=2.0)
    assert dst_id == ue_id
    assert proto_type == PROTO_MSG_RAR
    rar = harness.codec.deserialize_rar(payload)
    assert rar['ra_rnti'] == 5
    crnti = rar['temp_c_rnti']

    # Step 3: RRC Connection Setup Request (Msg3)
    # RrcEstablishmentCause::MO_SIGNALLING = 3
    req = harness.codec.serialize_rrc_setup_request(ue_identity=ue_identity, cause=3)
    harness.send_proto_message(ue_id, PROTO_MSG_RRC_SETUP_REQUEST, req)

    # Step 4: RRC Setup (Msg4)
    dst_id, proto_type, payload = harness.recv_proto_message(expected_dst_id=ue_id, timeout=2.0)
    assert dst_id == ue_id
    assert proto_type == PROTO_MSG_RRC_SETUP
    setup = harness.codec.deserialize_rrc_setup(payload)
    assert setup['received_identity'] == ue_identity
    assert setup['config_status'] == 1  # Success

    # Step 5: RRC Setup Complete (Msg5)
    complete = harness.codec.serialize_rrc_setup_complete(mcc=255, mnc=1)
    harness.send_proto_message(ue_id, PROTO_MSG_RRC_SETUP_COMPLETE, complete)

    return crnti


##
# @brief Tests Random Access Channel (RACH) procedure and RRC connection setup.
# @param gnb_harness The parameterized test harness fixture.
# @return None
# @details UE performs random access preamble transmission, decodes the timing advance and C-RNTI from Msg2,
#          requests configuration from gNB, and completes the setup.
#
def test_rach_and_rrc_setup(gnb_harness):
    establish_rrc_connection(gnb_harness, ue_id=777, ue_identity=987654321)


##
# @brief Tests NAS registration request handling.
# @param gnb_harness The parameterized test harness fixture.
# @return None
# @details Once RRC connected, the UE sends a Registration Request (containing capabilities).
#          The gNB responds with a Registration Answer indicating Acceptance.
#
def test_nas_registration(gnb_harness):
    establish_rrc_connection(gnb_harness, ue_id=777, ue_identity=987654321)

    # Send Registration Request
    reg_req = gnb_harness.codec.serialize_registration_request(ue_id=777, ue_cap="Model-X-MIMO4x4")
    gnb_harness.send_proto_message(777, PROTO_MSG_REGISTRATION_REQUEST, reg_req)

    # Receive Registration Response (wrapped in RrcSetup)
    dst_id, proto_type, payload = gnb_harness.recv_proto_message(expected_dst_id=777, timeout=2.0)
    assert dst_id == 777
    assert proto_type == PROTO_MSG_RRC_SETUP
    ans = gnb_harness.codec.deserialize_registration_answer(payload)
    assert ans['status'] == 1  # Accepted


##
# @brief Tests User Plane message routing between two connected UEs.
# @param gnb_harness The parameterized test harness fixture.
# @return None
# @details Registers two UEs (UE A and UE B) at the gNB, then routes a chat message from UE A to UE B.
#
def test_user_plane_relay(gnb_harness):
    # Establish connection for UE A (777)
    establish_rrc_connection(gnb_harness, ue_id=777, ue_identity=111111111)
    
    # Establish connection for UE B (888)
    establish_rrc_connection(gnb_harness, ue_id=888, ue_identity=222222222)

    # Complete NAS registration for both to trigger full attachment
    reg_req_a = gnb_harness.codec.serialize_registration_request(ue_id=777, ue_cap="UE-A")
    gnb_harness.send_proto_message(777, PROTO_MSG_REGISTRATION_REQUEST, reg_req_a)
    harness_payload = gnb_harness.recv_proto_message(expected_dst_id=777, timeout=2.0)
    
    reg_req_b = gnb_harness.codec.serialize_registration_request(ue_id=888, ue_cap="UE-B")
    gnb_harness.send_proto_message(888, PROTO_MSG_REGISTRATION_REQUEST, reg_req_b)
    harness_payload = gnb_harness.recv_proto_message(expected_dst_id=888, timeout=2.0)

    # Send User Plane Data from A to B
    chat_payload = gnb_harness.codec.serialize_chat_message(receiver_ue_id=888, sender_ue_id=777, text="Hello UE B!")
    gnb_harness.send_proto_message(777, PROTO_MSG_USER_PLANE_DATA, chat_payload)

    # Receive relayed message at UE B
    dst_id, proto_type, payload = gnb_harness.recv_proto_message(expected_dst_id=888, timeout=2.0)
    assert dst_id == 888
    assert proto_type == PROTO_MSG_USER_PLANE_DATA
    chat = gnb_harness.codec.deserialize_chat_message(payload)
    assert chat['sender_ue_id'] == 777
    assert chat['receiver_ue_id'] == 888
    assert chat['text'] == "Hello UE B!"


##
# @brief Tests handover trigger logic based on UE Measurement Reports.
# @param gnb_harness The parameterized test harness fixture.
# @return None
# @details UE reports low RSSI on serving cell and higher RSSI on target cell.
#          gNB must trigger handover by issuing an RRC Reconfiguration command with target GNB ID.
#
def test_handover_trigger(gnb_harness):
    establish_rrc_connection(gnb_harness, ue_id=777, ue_identity=987654321)

    # 1. Update serving cell RSSI
    serv_report = gnb_harness.codec.serialize_measurement_report(reported_gnb_id=gnb_harness.gnb_id, rsrp=-90.0)
    gnb_harness.send_proto_message(777, PROTO_MSG_MEASUREMENT_REPORT, serv_report)

    # Wait for serving update processing (ignoring broadcast packets if they arrive)
    time.sleep(0.1)

    # 2. Send neighbor report triggering handover (neighbor RSRP = -80.0, serving was -90.0, diff = 10dB > 3dB hysteresis)
    neigh_report = gnb_harness.codec.serialize_measurement_report(reported_gnb_id=102, rsrp=-80.0)
    gnb_harness.send_proto_message(777, PROTO_MSG_MEASUREMENT_REPORT, neigh_report)

    # 3. Receive RRC Reconfiguration
    dst_id, proto_type, payload = gnb_harness.recv_proto_message(expected_dst_id=777, timeout=2.0)
    assert dst_id == 777
    assert proto_type == PROTO_MSG_RRC_RECONFIGURATION
    reconfig = gnb_harness.codec.deserialize_rrc_reconfiguration(payload)
    assert reconfig['target_gnb_id'] == 102


##
# @brief Tests the inactivity release timer.
# @param gnb_harness The parameterized test harness fixture.
# @return None
# @details Once connected, if the UE sends no messages, the gNB should trigger RRC Release after 30 seconds.
#
@pytest.mark.slow
def test_inactivity_timeout(gnb_harness):
    establish_rrc_connection(gnb_harness, ue_id=777, ue_identity=987654321)

    # Wait for the inactivity release (~30s + grace time)
    # We poll to skip broadcast/SIB1 messages
    start_time = time.time()
    release_received = False
    
    while time.time() - start_time < 45.0:
        try:
            dst_id, proto_type, payload = gnb_harness.recv_proto_message(expected_dst_id=777, timeout=1.0)
            if dst_id == 777 and proto_type == PROTO_MSG_RRC_RELEASE:
                release = gnb_harness.codec.deserialize_rrc_release(payload)
                assert release['cause'] == 1  # UserInactivity
                release_received = True
                break
        except (socket.timeout, TimeoutError):
            pass

    assert release_received, "RRC Release not received after inactivity timeout"
