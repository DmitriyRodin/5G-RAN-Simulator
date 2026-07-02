#include "sys_info_service.hpp"

SIB1Info SysInfoService::buildSib1(const GnbCellConfig& cfg,
                                   const uint32_t gnb_id) const
{
    SIB1Info sib;

    sib.cell_identity =
        (static_cast<uint64_t>(gnb_id) << 8) | cfg.local_cell_id;
    sib.tac = cfg.tac;

    sib.qRx_lev_min = cfg.qRx_lev_min;
    sib.intra_freq_reselection = cfg.intra_freq_reselection;

    sib.cell_barred = cfg.cell_barred;
    sib.reserved_for_operator_use = cfg.reserved_for_operator_use;

    sib.rach_config = cfg.rach_config;
    sib.si_scheduling = cfg.si_scheduling;

    sib.plmn_identity_info_list = cfg.plmns;

    qDebug() << "[SysInfo] Built SIB1 master configuration for NCI:"
             << QString::number(sib.cell_identity, 16).toUpper()
             << "| PLMN count:" << sib.plmn_identity_info_list.size();

    return sib;
}
