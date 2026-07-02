#ifndef TYPES_HPP
#define TYPES_HPP

#include <chrono>

#include <QDebug>
#include <QHostAddress>
#include <QPoint>

enum class EntityType : uint8_t {
    UE,
    GNB,
    RadioHub,
    UNKNOWN = 255
};

QDebug operator<<(QDebug stream, EntityType type);

QString typeToString(EntityType type);

enum class RrcEstablishmentCause : uint8_t {
    EMERGENCY = 0,
    HIGH_PRIORITY_ACCESS = 1,
    MT_ACCESS = 2,      // Mobile Terminated
    MO_SIGNALLING = 3,  // Mobile Originated Signalling
    MO_DATA = 4,
    MO_VOICE_CALL = 5,
    MO_VIDEO_CALL = 6,
    MO_SMS = 7,
    MPS_PRIORITY_ACCESS = 8,
    MCS_PRIORITY_ACCESS = 9
};

inline QString toString(RrcEstablishmentCause cause)
{
    switch (cause) {
        case RrcEstablishmentCause::EMERGENCY:
            return "RrcEstablishmentCause::EMERGENCY";
        case RrcEstablishmentCause::HIGH_PRIORITY_ACCESS:
            return "RrcEstablishmentCause::HIGH_PRIORITY_ACCESS";
        case RrcEstablishmentCause::MO_SIGNALLING:
            return "mo-Signalling";
        case RrcEstablishmentCause::MO_DATA:
            return "mo-Data";

        default:
            return "unknown";
    }
}

// 5G states (3GPP TS 38.331)
enum class UeRrcState : uint8_t {
    RRC_IDLE,
    RRC_CONNECTED,
    RRC_INACTIVE
};

// 3GPP TS 38.304
enum class CellSearchStatus {
    /**
     * @value SYNCHRONIZING
     * @brief Hardware initialization and frequency sweeping. RF receiver is
     * unlocked, and the UE is completely blind to incoming radio frames (e.g.,
     * SIB1).
     */
    SYNCHRONIZING,
    /**
     * @value CELL_SELECTION
     * @brief RF hardware is locked to the channel. The receiver is active and
     * actively decoding MIB/SIB1 broadcast bursts.
     */
    CELL_SELECTION,
    /**
     * @value CAMPED
     * @brief Successful cell attachment. The UE has selected a valid cell and
     * is monitoring paging.
     */
    CAMPED
};

QString toString(UeRrcState ue_rrc_state);
QString toString(CellSearchStatus cell_status);

// ProtocolMsgType from gNb to UE and from UE to gNB

enum class ProtocolMsgType : uint8_t {
    // Broadcast
    Sib1 = 0,

    // RACH: Initial Access
    RachPreamble,
    Rar,

    // RRC Connection
    RrcSetup,
    RrcSetupRequest,
    RrcSetupComplete,
    RrcRelease,

    // NAS: Mobility & Connection Management
    RegistrationRequest,
    RegistrationAccept,
    DeregistrationRequest,
    ServiceRequest,
    Paging,

    // Xn/N2 Mobility: Handover
    MeasurementReport,
    RrcReconfiguration,
    RrcReconfigurationComplete,

    // User Plane
    UserPlaneData,

    Unknown = 255
};

enum class RegistrationStatus : uint8_t {
    Rejected = 0,
    Accepted = 1,
    Pending = 2
};

QString toString(RegistrationStatus reg_status);

using rnti_t = uint16_t;
using ta_index_t = uint16_t;

struct HubRegistrationResponce {
    uint8_t status;
};

struct RarInfo {
    rnti_t ra_rnti;
    rnti_t temp_c_rnti;
    ta_index_t timing_advance;
};

struct RrcSetupRequest {
    uint64_t ue_identity;
    uint8_t cause;
};

struct RrcSetupInfo {
    quint64 received_identity;
    uint8_t config_status;
};

struct PlmnIdentity {
    uint32_t mcc = 255;  // Mobile Country Code
    uint32_t mnc = 1;    // Mobile Network Code

    bool operator==(const PlmnIdentity& other) const
    {
        return (mcc == other.mcc) && (mnc == other.mnc);
    }
};

struct RrcSetupCompleteInfo {
    PlmnIdentity plmn;
};

struct RegistrationRequestInfo {
    uint32_t ue_id;
    QString ue_cap;
};

struct RegistrationAnswerInfo {
    RegistrationStatus status;
    std::optional<QString> reject_reason = std::nullopt;
};

struct RrcReconfigurationInfo {
    uint32_t gnb_id;
};

struct ChatMessageInfo {
    uint32_t receiver_ue_id;
    uint32_t sender_ue_id;
    QString text;
};

struct MeasurementReportInfo {
    uint32_t reported_gnb_id;
    double rsrp;
};

struct HandoverInfo {
    uint32_t gnb_id;
};

/**
 * @struct RachConfigCommon
 * @brief Represents the Random Access Channel (RACH) common configuration
 * parameters.
 * * This structure maps directly to the 3GPP TS 38.331 specification
 * (Section 6.3.2, Information Element: RACH-ConfigCommon / RACH-ConfigGeneric).
 * It configures the L1/L2 MAC Random Access procedure (CBRA) for User Equipment
 * (UE) synchronization.
 */
struct RachConfigCommon {
    /**
     * @brief Corresponds to 3GPP IE 'totalNumberOfRA-Preambles'.
     * Total number of preambles available for contention-based and
     * contention-free Random Access occasions per cell. Range: [1..63]. If
     * absent, all 64 preambles are available.
     */
    uint8_t total_number_of_RA_preambles = 64;

    /**
     * @brief Corresponds to 3GPP IE 'preambleTransMax'.
     * Maximum number of Random Access Preamble transmissions performed by the
     * UE MAC layer before declaring a Random Access Procedure failure to upper
     * RRC layer. Maps to 3GPP enum values: n3, n4, n5, n6, n7, n8, n10, n20,
     * etc.
     */
    uint8_t preamble_trans_max = 4;

    /**
     * @brief Corresponds to 3GPP IE 'ra-ResponseWindow'.
     * Duration of the Random Access Response (RAR / Msg2) monitoring window.
     * Expressed in milliseconds for the simulator abstraction layer (derived
     * from 3GPP slots 'sl'). If the UE does not receive a valid RAR matching
     * its RAPID within this window, a preamble retransmission is triggered.
     */
    uint16_t ra_response_window_ms = 20;

    /**
     * @brief Corresponds to 3GPP IE 'preambleReceivedTargetPower'.
     * The initial target power level expected by the gNB receiver for the
     * incoming preamble. Specified in dBm with a 2 dBm step size according to
     * standard. Used by the UE Open-Loop Power Control algorithm to calculate
     * initial TX power.
     */
    int8_t preamble_received_target_power = -100;

    /**
     * @brief Corresponds to 3GPP IE 'powerRampingStep'.
     * Power ramping step size applied for subsequent preamble retransmissions
     * upon collision or RAR timeout. Expressed in dB. Maps to 3GPP enum values:
     * dB0, dB2, dB4, dB6.
     */
    uint8_t power_ramping_step = 2;

    friend QDataStream& operator<<(QDataStream& out,
                                   const RachConfigCommon& config);
    friend QDataStream& operator>>(QDataStream& in, RachConfigCommon& config);
};

QDataStream& operator<<(QDataStream& out, const RachConfigCommon& config);
QDataStream& operator>>(QDataStream& in, RachConfigCommon& config);

/**
 * @enum SibType
 * @brief Categorizes the 5G System Information Blocks (SIBs) broadcasted by the
 * gNB.
 * * Maps directly to 3GPP TS 38.331 specifications for System Information
 * classification.
 */
enum class SibType : uint8_t {
    SIB2_RADIO_RESOURCE_COMMON = 2,  // Core radio resource configurations.
    SIB3_CELL_RESELECTION = 3,       // Neighbor cell serving frequencies for
                                     // intra-frequency reselection.
    SIB4_UTRA_NEIGHBORS = 4,         // Information about 3G/4G neighbor cells.
    SIB5_INTER_FREQ_HANDOVER =
        5  // Inter-frequency neighbor cell reselection criteria.
};

/**
 * @struct SibMapping
 * @brief Defines the broadcasting schedule for a specific System Information
 * Block.
 * * Maps to 3GPP 'SchedulingInfo' structure inside 'SI-SchedulingInfo' IE.
 */
struct SibMapping {
    /**
     * @brief Corresponds to 3GPP IE 'sib-Type'.
     * The type of the SIB mapped to this specific periodic schedule container.
     */
    SibType sib_type;

    /**
     * @brief Corresponds to 3GPP IE 'si-Periodicity'.
     * Periodic interval for broadcasting this SI message.
     * Expressed in milliseconds within the simulator (derived from 3GPP Radio
     * Frames 'rf'). Common network values: 80ms (rf8), 160ms (rf16), 320ms
     * (rf32), 640ms (rf64).
     */
    uint32_t periodicity_ms;

    friend QDataStream& operator<<(QDataStream& out, const SibMapping& sib_map);
    friend QDataStream& operator>>(QDataStream& in, SibMapping& sib_map);
};

/**
 * @struct SiSchedulingInfo
 * @brief Represents the System Information (SI) scheduling configuration.
 * * This structure maps directly to the 3GPP TS 38.331 specification
 * (Section 6.3.2, Information Element: SI-SchedulingInfo). It commands the UE
 * when and how long it must listen to the air interface to receive non-SIB1
 * system messages.
 */
struct SiSchedulingInfo {
    /**
     * @brief Corresponds to 3GPP IE 'si-WindowLength'.
     * The duration of the time window in milliseconds during which a specific
     * SI message is transmitted by the gNB. The UE activates its receiver only
     * during this window. Maps to 3GPP slot configurations (e.g., s5 = 5 slots,
     * s10 = 10 slots).
     */
    uint8_t si_window_length_ms = 10;

    /**
     * @brief Corresponds to 3GPP 'systemInfoValueTag' located inside SIB1.
     * A configuration version tag used to indicate modifications in the
     * broadcasted SI. Range: [0..31]. When the network modifies any SIB
     * configuration, this tag is incremented.
     */
    uint8_t system_info_value_tag = 1;

    /**
     * @brief Corresponds to 3GPP structure 'schedulingInfoList'.
     * Dynamic list of all scheduled SI messages and their respective SIB
     * mappings transmitted by this specific cell.
     */
    std::vector<SibMapping> scheduled_sibs;

    friend QDataStream& operator<<(QDataStream& out,
                                   const SiSchedulingInfo& info);
    friend QDataStream& operator>>(QDataStream& in, SiSchedulingInfo& info);
};

/**
 * @struct SIB1Info
 * @brief Represents the parsed contents of System Information Block Type 1
 * (SIB1).
 * * This structure maps directly to the 3GPP TS 38.331 specification
 * (Section 6.2.2, Information Element: SIB1). SIB1 contains system information
 * relevant when evaluating if a UE is allowed to access a cell and defines the
 * scheduling of other system information blocks.
 * * @see 3GPP TS 38.331 Version 18.5.0, Section 6.2.2, Page 381 (SIB1 message
 * definition).
 */
struct SIB1Info {
    /**
     * @brief Corresponds to 3GPP field 'cellIdentity' inside
     * 'CellAccessRelatedInfo'.
     * * Globally unique 5G Cell Identity (NCI - NR Cell Identity). It is a
     * 36-bit bitstring used to identify the specific cell within the PLMN
     * network topology.
     * * @see 3GPP TS 38.331, Section 6.3.2, Page 575 (CellIdentity IE).
     */
    uint32_t cell_identity;
    /**
     * @brief Corresponds to 3GPP field 'trackingAreaCode' inside
     * 'CellAccessRelatedInfo'.
     * * Tracking Area Code (TAC) identifies the tracking area to which the cell
     * belongs. Used by the 5G Core (AMF) for paging and mobility management. In
     * real 5G, it is a 24-bit bitstring (max value 16777215).
     * * @see 3GPP TS 38.331, Section 6.3.2, Page 1121 (TrackingAreaCode IE).
     */
    uint16_t tac;
    /**
     * @brief Corresponds to 3GPP field 'q-RxLevMin' inside 'CellSelectionInfo'.
     * * Specifies the minimum required Rx received signal level in the cell for
     * cell selection/reselection. Expressed in dBm.
     * Actual value = field value * 2 [dBm]. Range: [-70..-22] (maps to -140 dBm
     * to -44 dBm).
     * * @see 3GPP TS 38.331, Section 6.3.2, Page 578 (CellSelectionInfo-IEs).
     */
    int16_t qRx_lev_min = -70;
    /**
     * @brief Corresponds to 3GPP field 'cellBarred' inside
     * 'CellAccessRelatedInfo'.
     * * Indicates whether the cell is barred (prohibited) for access by any UE.
     * Maps to 3GPP ASN.1 ENUMERATED {barred, notBarred}. If set to true
     * (barred), the UE is not allowed to select this cell.
     * * @see 3GPP TS 38.331, Section 6.3.2, Page 576
     * (CellAccessRelatedInfo-IEs).
     */
    bool cell_barred = false;
    /**
     * @brief Corresponds to 3GPP field 'cellReservedForOperatorUse' inside
     * 'PLMN-IdentityInfo'.
     * * Indicates whether the cell is reserved for the operator's
     * private/testing use. Maps to 3GPP ASN.1 ENUMERATED {reserved,
     * notReserved}. UEs with normal USIMs ignore cells where this flag is set
     * to true.
     * * @see 3GPP TS 38.331, Section 6.3.2, Page 924 (PLMN-IdentityInfo-IEs).
     */
    bool reserved_for_operator_use = false;
    /**
     * @brief Corresponds to 3GPP field 'intraFreqReselection' inside
     * 'CellAccessRelatedInfo'.
     * * Controls cell reselection to intra-frequency cells when the highest
     * ranked cell is barred. Maps to 3GPP ASN.1 ENUMERATED {allowed,
     * notAllowed}. Tells the UE if it can look for sister cells on the exact
     * same frequency channel.
     * * @see 3GPP TS 38.331, Section 6.3.2, Page 576
     * (CellAccessRelatedInfo-IEs).
     */
    bool intra_freq_reselection = true;

    /**
     * @brief Corresponds to 3GPP field 'rach-ConfigCommon' inside
     * 'BWP-UplinkCommon'.
     * * Embedded structure containing cell-specific random access parameters
     * (L1/L2 MAC preamble parameters, response windows, and initial power
     * targets).
     * * @see 3GPP TS 38.331, Section 6.3.2, Page 544 (BWP-UplinkCommon-IEs).
     */
    RachConfigCommon rach_config;

    /**
     * @brief Corresponds to 3GPP field 'si-SchedulingInfo' inside SIB1 message
     * root.
     * * Embedded structure defining the asynchronous time-window lengths,
     * periodicities, and mapping list for all other broadcasted SIBs (SIB2,
     * SIB3, SIB5, etc.).
     * * @see 3GPP TS 38.331, Section 6.3.2, Page 1042 (SI-SchedulingInfo-IEs).
     */
    SiSchedulingInfo si_scheduling;
    /**
     * @brief Corresponds to 3GPP structure 'plmn-IdentityInfoList' inside SIB1.
     * * Dynamic vector containing the list of PLMN identities (MCC/MNC
     * combinations) broadcasted by this specific gNB cell. A 5G cell can be
     * shared between multiple network operators simultaneously.
     * * @see 3GPP TS 38.331, Section 6.2.2, Page 382 (SIB1-IEs definition).
     */
    std::vector<PlmnIdentity> plmn_identity_info_list;
};

struct GnbCellConfig {
    uint16_t local_cell_id = 1;
    uint16_t tac = 100;  // Tracking Area Code
    int16_t qRx_lev_min = -115;
    double tx_power_db = 43.0;

    bool cell_barred = false;
    bool reserved_for_operator_use = false;
    bool intra_freq_reselection = true;

    RachConfigCommon rach_config;
    SiSchedulingInfo si_scheduling;

    std::vector<PlmnIdentity> plmns;

    GnbCellConfig(std::vector<PlmnIdentity> plmns_ident, const uint8_t size)
        : plmns(plmns_ident)
    {
        for (const auto& [mcc, mnc] : plmns) {
            qDebug() << "mcc: " << mcc << ", mnc: " << mnc;
        }
    }
    GnbCellConfig()
    {
    }
};

struct RachPreambleInfo {
    rnti_t ra_rnti;
};

QDebug operator<<(QDebug stream, ChatMessageInfo chat_info);

struct UeContext {
    uint32_t id;
    rnti_t crnti;
    PlmnIdentity selected_plmn;

    UeRrcState state;
    RrcEstablishmentCause establishmentCause;
    bool is_attached;

    QHostAddress ip_address;
    quint16 port;

    double last_rssi;
    std::chrono::steady_clock::time_point last_activity;

    UeContext(uint32_t ue_id, rnti_t new_crnti, const QHostAddress& addr,
              quint16 ip_port)
        : id(ue_id)
        , crnti(new_crnti)
        , selected_plmn(PlmnIdentity{})
        , state(UeRrcState::RRC_IDLE)
        , establishmentCause(RrcEstablishmentCause::MO_SIGNALLING)
        , is_attached(false)
        , ip_address(addr)
        , port(ip_port)
        , last_rssi(0.0)
        , last_activity(std::chrono::steady_clock::now())
    {
    }

    UeContext()
        : id(0)
        , crnti(0)
        , selected_plmn(PlmnIdentity{})
        , state(UeRrcState::RRC_IDLE)
        , establishmentCause(RrcEstablishmentCause::MO_SIGNALLING)
        , is_attached(false)
        , port(0)
        , last_rssi(0.0)
        , last_activity(std::chrono::steady_clock::now())
    {
    }
};

// header and message type from gNb and UE to RaioHub(UE and gNB connection
// simulation)

enum class SimMessageType : uint8_t {
    Registration = 0,
    RegistrationResponse,
    Deregistration,
    Data,
    Unknown = 255
};

struct SimHeader {
    uint32_t source_id;
    uint32_t target_id;
    SimMessageType type;
};

namespace HubResponse {
const uint8_t REG_DENIED = 0;
const uint8_t REG_ACCEPTED = 1;
}  // namespace HubResponse

namespace RrcConfig {
enum class Status : uint8_t {
    Failure = 0,
    Success = 1,
    ReconfigurationWithSync = 2
};
}  // namespace RrcConfig

enum class RrcReleaseCause : uint8_t {
    Other = 0,
    UserInactivity = 1,
    RrcConnectionFailure = 2,
    LoadBalancing = 3,
    CellReselection = 4,
    VoiceFallback = 5
};

inline QString toString(RrcReleaseCause cause)
{
    switch (cause) {
        case RrcReleaseCause::Other:
            return "Other";
        case RrcReleaseCause::UserInactivity:
            return "UserInactivity";
        case RrcReleaseCause::RrcConnectionFailure:
            return "RrcConnectionFailure";
        case RrcReleaseCause::LoadBalancing:
            return "LoadBalancing";
        case RrcReleaseCause::CellReselection:
            return "CellReselection";
        case RrcReleaseCause::VoiceFallback:
            return "VoiceFallback";
        default:
            return "Unknown";
    }
}

namespace NetworkParam {
inline constexpr quint16 EPHEMERAL_PORT = 0;
};

#endif  // TYPES_HPP
