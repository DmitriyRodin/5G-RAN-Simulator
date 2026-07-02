#ifndef SYS_INFO_SERVICE_HPP
#define SYS_INFO_SERVICE_HPP

#include "types.hpp"

class SysInfoService
{
public:
    SIB1Info buildSib1(const GnbCellConfig& cfg, const uint32_t gnb_id) const;
};

#endif  // SYS_INFO_SERVICE_HPP
