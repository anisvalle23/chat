#ifndef SERVIDOR_SOCKET_H
#define SERVIDOR_SOCKET_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QSet>
#include "usuario.h"

class ServidorSocket : public QTcpServer
{
    Q_OBJECT

public:
    explicit ServidorSocket(QObject* parent = nullptr);
    void startServidor(quint16 puerto);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void leerMensaje();
    void clienteDesconectado();

private:
    QMap<QTcpSocket*, QString> clientes;      // socket → nombre de usuario
    QSet<QString> usuariosConectados;         // nombres conectados actualmente

    void enviarEstadoConectados();            // notifica a todos el estado actualizado
    void actualizarEstadoEnArchivo(const QString &usuario, bool conectado); // 🔄 escribe en usuarios.txt
};

#endif // SERVIDOR_SOCKET_H
