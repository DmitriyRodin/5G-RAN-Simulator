#ifndef NETWORK_NODE_HPP
#define NETWORK_NODE_HPP

#include <cstdint>

#include <QHostAddress>
#include <QPointF>

#include "types.hpp"

struct NodePassport {
    uint32_t id;
    EntityType type;
    QHostAddress address;
    quint16 port;
    QPointF position;

    NodePassport getNodePassport()
    {
        return {id, type, address, port, position};
    }
};

struct GnbData {
    static constexpr uint32_t INITIAL_UE_COUNT = 0;
    double radius = 0.0;
    uint32_t connected_ue_count = INITIAL_UE_COUNT;
};

struct UeData {
    static constexpr uint32_t INITIAL_TARGET_GNB = 0;
    bool is_connected = false;
    QString state = "IDLE";
    uint32_t target_gnb = INITIAL_TARGET_GNB;
};

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

struct NodeInfo : NodePassport {
    std::variant<GnbData, UeData> specific_data;
};

class INetworkNode
{
public:
    virtual ~INetworkNode() = default;
    virtual uint32_t getId() const = 0;
    virtual EntityType getType() const = 0;
    virtual QPointF position() const = 0;
    virtual void setPosition(QPointF pos) = 0;
    virtual quint16 port() const = 0;
    virtual void setPort(quint16 port) = 0;
};

class RemoteNodeProxy : public INetworkNode
{
public:
    RemoteNodeProxy(NodePassport node_pass)
        : node_pass_(node_pass)
    {
    }

    uint32_t getId() const override
    {
        return node_pass_.id;
    }
    EntityType getType() const override
    {
        return node_pass_.type;
    }
    QPointF position() const override
    {
        return node_pass_.position;
    }
    void setPosition(QPointF pos) override
    {
        node_pass_.position = pos;
    }

    quint16 port() const override
    {
        return node_pass_.port;
    }
    void setPort(quint16 port) override
    {
        node_pass_.port = port;
    }

    NodePassport getNodePass() const
    {
        return node_pass_;
    }

private:
    NodePassport node_pass_;
};

class RemoteUeProxy : public RemoteNodeProxy
{
public:
    RemoteUeProxy(NodeInfo node_info)
        : RemoteNodeProxy(node_info.getNodePassport())
        , data_(std::get<UeData>(node_info.specific_data))
    {
    }

    bool isConnected() const
    {
        return data_.is_connected;
    }
    QString stateString() const
    {
        return data_.state;
    }
    uint32_t getTargetGnb() const
    {
        return data_.target_gnb;
    }

    void setConnected(bool is_connected)
    {
        data_.is_connected = is_connected;
    }
    void setState(const QString& state)
    {
        data_.state = state;
    }
    void setTargetGnb(uint32_t gnb_id)
    {
        data_.target_gnb = gnb_id;
    }

private:
    UeData data_;
};

class RemoteGnbProxy : public RemoteNodeProxy
{
public:
    RemoteGnbProxy(NodeInfo node_info)
        : RemoteNodeProxy(node_info.getNodePassport())
        , data_(std::get<GnbData>(node_info.specific_data))
    {
    }

    const GnbData& getData() const
    {
        return data_;
    }
    void updateState(const GnbData& new_state)
    {
        data_ = new_state;
    }

    uint32_t getConnectedUeCount() const
    {
        return data_.connected_ue_count;
    }
    double getRadius() const
    {
        return data_.radius;
    }

private:
    GnbData data_;
};

#endif  // NETWORK_NODE_HPP
