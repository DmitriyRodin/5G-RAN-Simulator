#ifndef BASE_ENTITY_HPP
#define BASE_ENTITY_HPP

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUdpSocket>

#include <string>

#include <common/types.hpp>

class BaseEntity: public QObject {
    Q_OBJECT
public:
    BaseEntity(int id, const EntityType& type, QObject* parent = nullptr);
    virtual ~BaseEntity();

    void stop();
    virtual void run() = 0;

protected slots:
    virtual void readPendingDatagrams();

protected:
    void setup_network();
    void send_message(const std::string& payload);

    int id_;
    EntityType type_;
    QUdpSocket* socket_;
    QHostAddress hubAddress_;
    quint16 hubPort_;

private:
    virtual void processIncoming(const QByteArray& data,
                         const QHostAddress& sender_ip,
                         quint16 sender_port) = 0;
};

#endif // BASE_ENTITY_HPP
