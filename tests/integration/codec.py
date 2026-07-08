import struct
import os
import sys
import subprocess

os.environ['PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION'] = 'python'

def compile_protobuf():
    """
    Compiles ran_messages.proto using protoc if it exists and imports it.
    """
    current_dir = os.path.dirname(os.path.abspath(__file__))
    proto_dir = os.path.abspath(os.path.join(current_dir, '../../common/protobuf/proto'))
    proto_file = os.path.join(proto_dir, 'ran_messages.proto')
    
    if not os.path.exists(proto_file):
        raise FileNotFoundError(f"ran_messages.proto not found at {proto_file}")
        
    cmd = [
        'protoc',
        '--experimental_allow_proto3_optional',
        f'-I={proto_dir}',
        f'--python_out={current_dir}',
        proto_file
    ]
    subprocess.run(cmd, check=True)
    
    if current_dir not in sys.path:
        sys.path.insert(0, current_dir)

try:
    compile_protobuf()
    import ran_messages_pb2
except Exception as e:
    print(f"Warning: Oops -> Failed to compile or import protobuf ran_messages_pb2: {e}")
    ran_messages_pb2 = None

def pack_qstring(s):
    if s is None:
        return struct.pack('>I', 0xffffffff)
    encoded = s.encode('utf-16-be')
    return struct.pack('>I', len(encoded)) + encoded

def unpack_qstring(data, offset):
    length, = struct.unpack_from('>I', data, offset)
    offset += 4
    if length == 0xffffffff:
        return None, offset
    val = data[offset:offset+length].decode('utf-16-be')
    offset += length
    return val, offset

# EntityTypes
UE_TYPE = 0
GNB_TYPE = 1
HUB_TYPE = 2

# SimMessageTypes
SIM_MSG_REGISTRATION = 0
SIM_MSG_REGISTRATION_RESPONSE = 1
SIM_MSG_DEREGISTRATION = 2
SIM_MSG_DATA = 3

# ProtocolMsgTypes
PROTO_MSG_SIB1 = 0
PROTO_MSG_RACH_PREAMBLE = 1
PROTO_MSG_RAR = 2
PROTO_MSG_RRC_SETUP = 3
PROTO_MSG_RRC_SETUP_REQUEST = 4
PROTO_MSG_RRC_SETUP_COMPLETE = 5
PROTO_MSG_RRC_RELEASE = 6
PROTO_MSG_REGISTRATION_REQUEST = 7
PROTO_MSG_REGISTRATION_ACCEPT = 8
PROTO_MSG_DEREGISTRATION_REQUEST = 9
PROTO_MSG_SERVICE_REQUEST = 10
PROTO_MSG_PAGING = 11
PROTO_MSG_MEASUREMENT_REPORT = 12
PROTO_MSG_RRC_RECONFIGURATION = 13
PROTO_MSG_RRC_RECONFIGURATION_COMPLETE = 14
PROTO_MSG_USER_PLANE_DATA = 15

class SimPacket:
    def __init__(self, src_id, node_type, dst_id, msg_type, pos_x=0.0, pos_y=0.0, payload=b''):
        self.src_id = src_id
        self.node_type = node_type
        self.dst_id = dst_id
        self.msg_type = msg_type
        self.pos_x = pos_x
        self.pos_y = pos_y
        self.payload = payload

    def pack(self):
        # Remember header format: >IBIBdd (Big-Endian byte order. struct size: 26 bytes)
        header = struct.pack('>IBIBdd', self.src_id, self.node_type, self.dst_id, self.msg_type, self.pos_x, self.pos_y)
        return header + self.payload

    @classmethod
    def unpack(cls, data):
        if len(data) < 26:
            raise ValueError("Packet too short")
        src_id, node_type, dst_id, msg_type, pos_x, pos_y = struct.unpack_from('>IBIBdd', data, 0)
        payload = data[26:]
        return cls(src_id, node_type, dst_id, msg_type, pos_x, pos_y, payload)

class QDataStreamCodec:
    @staticmethod
    def serialize_registration_payload(radius):
        return struct.pack('>d', radius)

    @staticmethod
    def deserialize_hub_registration_response(payload):
        status, = struct.unpack('>B', payload)
        return {'status': status}

    @staticmethod
    def deserialize_sib1(payload):
        cell_id, tac, q_rx, barred, reserved, reselection = struct.unpack_from('>Ihh???', payload, 0)
        offset = 11
        
        # RachConfigCommon
        total_p, trans_max, win, target_p, ramping_step = struct.unpack_from('>BBHbB', payload, offset)
        offset += 6
        
        # SiSchedulingInfo
        win_len, tag, sibs_size = struct.unpack_from('>BBI', payload, offset)
        offset += 6
        sibs = []
        for _ in range(sibs_size):
            sib_type, periodicity = struct.unpack_from('>BI', payload, offset)
            sibs.append({'sib_type': sib_type, 'periodicity_ms': periodicity})
            offset += 5
            
        # PLMN list
        plmns_size, = struct.unpack_from('>B', payload, offset)
        offset += 1
        plmns = []
        for _ in range(plmns_size):
            mcc, mnc = struct.unpack_from('>II', payload, offset)
            plmns.append({'mcc': mcc, 'mnc': mnc})
            offset += 8
            
        return {
            'cell_identity': cell_id,
            'tac': tac,
            'qRx_lev_min': q_rx,
            'cell_barred': barred,
            'reserved_for_operator_use': reserved,
            'intra_freq_reselection': reselection,
            'rach_config': {
                'total_number_of_RA_preambles': total_p,
                'preamble_trans_max': trans_max,
                'ra_response_window_ms': win,
                'preamble_received_target_power': target_p,
                'power_ramping_step': ramping_step
            },
            'si_scheduling': {
                'si_window_length_ms': win_len,
                'system_info_value_tag': tag,
                'scheduled_sibs': sibs
            },
            'plmn_identity_info_list': plmns
        }

    @staticmethod
    def serialize_rach_preamble(ra_rnti):
        return struct.pack('>H', ra_rnti)

    @staticmethod
    def deserialize_rar(payload):
        ra_rnti, temp_c_rnti, timing_advance = struct.unpack('>HHH', payload)
        return {'ra_rnti': ra_rnti, 'temp_c_rnti': temp_c_rnti, 'timing_advance': timing_advance}

    @staticmethod
    def serialize_rrc_setup_request(ue_identity, cause):
        return struct.pack('>QB', ue_identity, cause)

    @staticmethod
    def deserialize_rrc_setup(payload):
        received_identity, config_status = struct.unpack('>QB', payload)
        return {'received_identity': received_identity, 'config_status': config_status}

    @staticmethod
    def serialize_rrc_setup_complete(mcc, mnc):
        return struct.pack('>II', mcc, mnc)

    @staticmethod
    def serialize_registration_request(ue_id, ue_cap):
        return struct.pack('>I', ue_id) + pack_qstring(ue_cap)

    @staticmethod
    def deserialize_registration_answer(payload):
        status, = struct.unpack_from('>B', payload, 0)
        offset = 1
        reject_reason = None
        if status != 1 and len(payload) > 1:
            reject_reason, _ = unpack_qstring(payload, offset)
        return {'status': status, 'reject_reason': reject_reason}

    @staticmethod
    def serialize_chat_message(receiver_ue_id, sender_ue_id, text):
        return struct.pack('>II', receiver_ue_id, sender_ue_id) + pack_qstring(text)

    @staticmethod
    def deserialize_chat_message(payload):
        receiver, sender = struct.unpack_from('>II', payload, 0)
        text, _ = unpack_qstring(payload, 8)
        return {'receiver_ue_id': receiver, 'sender_ue_id': sender, 'text': text}

    @staticmethod
    def serialize_measurement_report(reported_gnb_id, rsrp):
        return struct.pack('>Id', reported_gnb_id, rsrp)

    @staticmethod
    def deserialize_rrc_reconfiguration(payload):
        target_gnb_id, = struct.unpack('>I', payload)
        return {'target_gnb_id': target_gnb_id}

    @staticmethod
    def deserialize_rrc_release(payload):
        cause, = struct.unpack('>B', payload)
        return {'cause': cause}

class ProtobufCodec:
    @staticmethod
    def serialize_registration_payload(radius):
        msg = ran_messages_pb2.HubRegistrationPayload()
        msg.radius = radius
        return msg.SerializeToString()

    @staticmethod
    def deserialize_hub_registration_response(payload):
        msg = ran_messages_pb2.HubRegistrationResponse()
        msg.ParseFromString(payload)
        return {'status': msg.status}

    @staticmethod
    def deserialize_sib1(payload):
        msg = ran_messages_pb2.SIB1Info()
        msg.ParseFromString(payload)
        sibs = []
        for s in msg.si_scheduling.scheduled_sibs:
            sibs.append({'sib_type': s.sib_type, 'periodicity_ms': s.periodicity_ms})
            
        plmns = []
        for p in msg.plmn_identity_info_list:
            plmns.append({'mcc': p.mcc, 'mnc': p.mnc})
            
        return {
            'cell_identity': msg.cell_identity,
            'tac': msg.tac,
            'qRx_lev_min': msg.qRx_lev_min,
            'cell_barred': msg.cell_barred,
            'reserved_for_operator_use': msg.reserved_for_operator_use,
            'intra_freq_reselection': msg.intra_freq_reselection,
            'rach_config': {
                'total_number_of_RA_preambles': msg.rach_config.total_number_of_RA_preambles,
                'preamble_trans_max': msg.rach_config.preamble_trans_max,
                'ra_response_window_ms': msg.rach_config.ra_response_window_ms,
                'preamble_received_target_power': -100,  # Not present in proto
                'power_ramping_step': 2  # Not present in proto
            },
            'si_scheduling': {
                'si_window_length_ms': msg.si_scheduling.si_window_length_ms,
                'system_info_value_tag': msg.si_scheduling.system_info_value_tag,
                'scheduled_sibs': sibs
            },
            'plmn_identity_info_list': plmns
        }

    @staticmethod
    def serialize_rach_preamble(ra_rnti):
        msg = ran_messages_pb2.RachPreambleInfo()
        msg.ra_rnti = ra_rnti
        return msg.SerializeToString()

    @staticmethod
    def deserialize_rar(payload):
        msg = ran_messages_pb2.RarInfo()
        msg.ParseFromString(payload)
        return {'ra_rnti': msg.ra_rnti, 'temp_c_rnti': msg.temp_c_rnti, 'timing_advance': msg.timing_advance}

    @staticmethod
    def serialize_rrc_setup_request(ue_identity, cause):
        msg = ran_messages_pb2.RrcSetupRequest()
        msg.ue_identity = ue_identity
        msg.cause = cause
        return msg.SerializeToString()

    @staticmethod
    def deserialize_rrc_setup(payload):
        msg = ran_messages_pb2.RrcSetupInfo()
        msg.ParseFromString(payload)
        return {'received_identity': msg.received_identity, 'config_status': msg.config_status}

    @staticmethod
    def serialize_rrc_setup_complete(mcc, mnc):
        msg = ran_messages_pb2.RrcSetupCompleteInfo()
        msg.plmn.mcc = mcc
        msg.plmn.mnc = mnc
        return msg.SerializeToString()

    @staticmethod
    def serialize_registration_request(ue_id, ue_cap):
        msg = ran_messages_pb2.RegistrationRequestInfo()
        msg.ue_id = ue_id
        msg.ue_cap = ue_cap
        return msg.SerializeToString()

    @staticmethod
    def deserialize_registration_answer(payload):
        msg = ran_messages_pb2.RegistrationAnswerInfo()
        msg.ParseFromString(payload)
        reject = msg.reject_reason if msg.HasField('reject_reason') else None
        return {'status': msg.status, 'reject_reason': reject}

    @staticmethod
    def serialize_chat_message(receiver_ue_id, sender_ue_id, text):
        msg = ran_messages_pb2.ChatMessageInfo()
        msg.receiver_ue_id = receiver_ue_id
        msg.sender_ue_id = sender_ue_id
        msg.text = text
        return msg.SerializeToString()

    @staticmethod
    def deserialize_chat_message(payload):
        msg = ran_messages_pb2.ChatMessageInfo()
        msg.ParseFromString(payload)
        return {'receiver_ue_id': msg.receiver_ue_id, 'sender_ue_id': msg.sender_ue_id, 'text': msg.text}

    @staticmethod
    def serialize_measurement_report(reported_gnb_id, rsrp):
        msg = ran_messages_pb2.MeasurementReportInfo()
        msg.reported_gnb_id = reported_gnb_id
        msg.rsrp = rsrp
        return msg.SerializeToString()

    @staticmethod
    def deserialize_rrc_reconfiguration(payload):
        msg = ran_messages_pb2.RrcReconfigurationInfo()
        msg.ParseFromString(payload)
        return {'target_gnb_id': msg.target_gnb_id}

    @staticmethod
    def deserialize_rrc_release(payload):
        msg = ran_messages_pb2.RrcReconfigurationInfo()
        msg.ParseFromString(payload)
        return {'cause': msg.target_gnb_id}
