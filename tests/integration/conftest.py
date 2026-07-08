import os
import tempfile
import yaml
import socket
import subprocess
import time
import pytest

from codec import (
    SimPacket,
    QDataStreamCodec,
    ProtobufCodec,
    GNB_TYPE,
    UE_TYPE,
    HUB_TYPE,
    SIM_MSG_REGISTRATION,
    SIM_MSG_REGISTRATION_RESPONSE,
    SIM_MSG_DATA
)

class GnbTestHarness:
    def __init__(self, serializer_type):
        self.serializer_type = serializer_type  # 0 for QDataStream, 1 for Protobuf
        self.codec = QDataStreamCodec if serializer_type == 0 else ProtobufCodec
        self.sock = None
        self.proc = None
        self.gnb_addr = None  # ip and port
        self.temp_config_path = None
        self.gnb_id = 101
        self.hub_id = 0

    def setup(self):
        # 1. Read base integration config
        current_dir = os.path.dirname(os.path.abspath(__file__))
        config_path = os.path.join(current_dir, 'config.yaml')
        with open(config_path, 'r') as f:
            config_data = yaml.safe_load(f)

        # Update serializer type
        config_data['simulation']['serializer_type'] = self.serializer_type

        # 2. Write to temp config file
        fd, self.temp_config_path = tempfile.mkstemp(suffix='.yaml', prefix='gnb_test_config_')
        os.close(fd)
        with open(self.temp_config_path, 'w') as f:
            yaml.safe_dump(config_data, f)

        # 3. Bind UDP socket to RadioHub port (6000)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        if hasattr(socket, 'SO_REUSEPORT'):
            try:
                self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
            except Exception:
                pass
        self.sock.bind(('127.0.0.1', 6000))
        self.sock.settimeout(3.0)

        # Let's drain any leftover packets in the OS buffer
        self.sock.setblocking(False)
        while True:
            try:
                self.sock.recvfrom(65536)
            except Exception:
                break
        self.sock.setblocking(True)

        # 4. Launch gnb_app subprocess
        gnb_bin = os.path.abspath(os.path.join(current_dir, '../../build_test/gnb/gnb_app'))
        if not os.path.exists(gnb_bin):
            gnb_bin = os.path.abspath(os.path.join(current_dir, '../../build/gnb/gnb_app'))
            if not os.path.exists(gnb_bin):
                raise FileNotFoundError("gnb_app binary not found in build_test or build directories")
        self.proc = subprocess.Popen(
            [gnb_bin, '-i', str(self.gnb_id), '-c', self.temp_config_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        # 5. Wait for Registration packet from gNB
        try:
            raw_data, addr = self.sock.recvfrom(4096)
            self.gnb_addr = addr
            packet = SimPacket.unpack(raw_data)
            
            assert packet.src_id == self.gnb_id
            assert packet.node_type == GNB_TYPE
            assert packet.dst_id == self.hub_id
            assert packet.msg_type == SIM_MSG_REGISTRATION

            # Respond with Registration Response (ACCEPTED = 1)
            reg_resp_payload = self.codec.serialize_registration_payload(1200.0) # Dummy radius/status representation
            # Wait, C++ BaseEntity expects deserialized status from registration response
            # Let's serialize the HubRegistrationResponse status=1
            if self.serializer_type == 1:
                # Protobuf Registration response status
                reg_resp_payload = self.codec.serialize_registration_payload(1.0) # Actually we have status=1
                import ran_messages_pb2
                proto_resp = ran_messages_pb2.HubRegistrationResponse()
                proto_resp.status = 1
                reg_resp_payload = proto_resp.SerializeToString()
            else:
                import struct
                reg_resp_payload = struct.pack('>B', 1)

            resp_packet = SimPacket(
                src_id = self.hub_id,
                node_type = HUB_TYPE,
                dst_id = self.gnb_id,
                msg_type = SIM_MSG_REGISTRATION_RESPONSE,
                payload = reg_resp_payload
            )
            self.sock.sendto(resp_packet.pack(), self.gnb_addr)
            
        except socket.timeout:
            self.cleanup()
            raise TimeoutError("gNB registration timed out")

    def send_proto_message(self, ue_id, proto_msg_type, payload):
        """
        Sends a simulated protocol message "from the UE" to the gNB.
        """
        # Prefix the protocol message type byte
        full_payload = bytes([proto_msg_type]) + payload
        packet = SimPacket(
            src_id = ue_id,
            node_type = UE_TYPE,
            dst_id = self.gnb_id,
            msg_type=SIM_MSG_DATA,
            payload=full_payload
        )
        self.sock.sendto(packet.pack(), self.gnb_addr)

    def recv_packet(self, timeout=2.0):
        """
        Let's receive a SimPacket from the socket.
        """
        self.sock.settimeout(timeout)
        raw_data, _ = self.sock.recvfrom(4096)
        return SimPacket.unpack(raw_data)

    def recv_proto_message(self, expected_dst_id=None, timeout=2.0):
        """
        Now we receive a protocol message from the gNB, returning
        (target_id, proto_type, payload).
        Filters out broadcast packets if expected_dst_id is specified.
        """
        #import time
        start = time.time()
        while time.time() - start < timeout:
            try:
                packet = self.recv_packet(timeout=max(0.1, timeout - (time.time() - start)))
                if packet.msg_type == SIM_MSG_DATA:
                    proto_type = packet.payload[0]
                    payload = packet.payload[1:]
                    if expected_dst_id is None or packet.dst_id == expected_dst_id:
                        return packet.dst_id, proto_type, payload
            except socket.timeout:
                break
        raise TimeoutError(f"Timed out waiting for message targeting {expected_dst_id}")

    def cleanup(self):
        if self.proc:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=1.0)
            except subprocess.TimeoutExpired:
                self.proc.kill()
            self.proc = None
        if self.sock:
            self.sock.close()
            self.sock = None
        if self.temp_config_path and os.path.exists(self.temp_config_path):
            os.remove(self.temp_config_path)

@pytest.fixture(params=[0, 1])
def gnb_harness(request):
    """
    Pytest fixture parameterized to run tests under QDataStream (0) and Protobuf (1) modes.
    """
    # Skip Protobuf tests if protobuf package is not installed in the environment
    # import ran_messages_pb2 checks if compilation succeeded.
    try:
        import ran_messages_pb2
    except ImportError:
        ran_messages_pb2 = None
    if request.param == 1 and ran_messages_pb2 is None:
        pytest.skip("Oops: protobuf Python package is not installed."
        "We should skip Protobuf tests")
    harness = GnbTestHarness(request.param)
    harness.setup()
    yield harness
    harness.cleanup()
