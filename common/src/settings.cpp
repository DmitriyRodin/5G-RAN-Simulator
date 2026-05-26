#include "settings.hpp"

#include "flow_logger.hpp"

SettingsPack::SettingsPack(HubSettings h, UeSettings u, GnbSettings g,
                           SimulationSettings s, Positions pos, Paths p)
    : hub(std::move(h))
    , ue(std::move(u))
    , gnb(std::move(g))
    , sim(std::move(s))
    , positions(std::move(pos))
    , paths(std::move(p))
{
}

DeployMode SettingsPack::getMode() const
{
    return sim.deploy_mode;
};

FlowLoggerSetupInfo SettingsPack::getFlowLoggerInfo() const
{
    return FlowLoggerSetupInfo(hub.id, hub.broadcast_id);
}

uint32_t SettingsPack::getBroadcast() const
{
    return hub.id;
}

HubSettings::HubSettings(uint16_t p, uint32_t hid, uint32_t bid, Point2D vhp,
                         std::string add)
    : port(p)
    , id(hid)
    , broadcast_id(bid)
    , virt_pos(vhp)
    , address(add)
{
}

NodeSettings::NodeSettings(HubSettings hub_set, RadioSettings radio_set,
                           Cell cell_set)
    : hub(hub_set)
    , radio(radio_set)
    , cell(cell_set)
{
}

UeSettings::UeSettings(HubSettings hub_set, RadioSettings radio_set,
                       Cell cell_set)
    : NodeSettings(hub_set, radio_set, cell_set)
{
}

GnbSettings::GnbSettings(HubSettings h, RadioSettings r_set, Cell c, double r)
    : NodeSettings(h, r_set, c)
    , radius(r)
{
}

BaseNodeContext::BaseNodeContext(uint32_t node_id, Point2D position)
    : id(node_id)
    , pos(position)
{
}

GnbRuntimeContext::GnbRuntimeContext(const uint32_t node_id,
                                     const GnbSettings settings,
                                     const Point2D pos)
    : BaseNodeContext(node_id, pos)
    , set(settings)
{
}

UeRuntimeContext::UeRuntimeContext(uint32_t node_id, UeSettings settings,
                                   Point2D pos)
    : BaseNodeContext(node_id, pos)
    , set(settings)
{
}
