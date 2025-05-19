#ifndef CLIENTE_SOCKET_H
#define CLIENTE_SOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QMap>

class ClienteSocket : public QObject
{
    Q_OBJECT

public:
    explicit ClienteSocket(QObject *parent = nullptr);
    void conectarServidor(const QString &host, quint16 puerto);
    void enviarMensaje(const QString &mensaje);
    void setNombreUsuario(const QString &usuario);
    void desconectar();

signals:
    void mensajeRecibido(const QString &remitente, const QString &mensaje);
    void estadoUsuariosActualizado(const QMap<QString, QString> &estados);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();

private:
    QTcpSocket *socket;
    QString nombreUsuario;
    QString host;
    quint16 puerto;
    void reconectar();
};

#endif // CLIENTE_SOCKET_H
