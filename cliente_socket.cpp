#include "cliente_socket.h"
#include <QTimer>
#include <QDebug>

ClienteSocket::ClienteSocket(QObject *parent)
    : QObject(parent)
{
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, &ClienteSocket::onReadyRead);
    connect(socket, &QTcpSocket::connected, this, &ClienteSocket::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &ClienteSocket::onDisconnected);
}

void ClienteSocket::conectarServidor(const QString &host, quint16 puerto)
{
    this->host = host;
    this->puerto = puerto;

    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
    }

    qDebug() << "🔌 Intentando conectar a" << host << ":" << puerto;
    socket->connectToHost(host, puerto);

    if (!socket->waitForConnected(3000)) {
        qDebug() << "❌ No se pudo conectar. Reintentando en 3 segundos...";
        QTimer::singleShot(3000, this, &ClienteSocket::reconectar);
    }
}

void ClienteSocket::reconectar()
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        conectarServidor(host, puerto);
    }
}

void ClienteSocket::setNombreUsuario(const QString &usuario)
{
    nombreUsuario = usuario.trimmed();
}

void ClienteSocket::enviarMensaje(const QString &mensaje)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QString mensajeConSalto = mensaje.trimmed() + "\n";
        socket->write(mensajeConSalto.toUtf8());
        socket->flush();
        qDebug() << "📤 Enviado al servidor:" << mensajeConSalto;
    } else {
        qDebug() << "⚠️ No conectado al servidor. Mensaje no enviado:" << mensaje;
    }
}

void ClienteSocket::onReadyRead()
{
    while (socket->canReadLine()) {
        QString mensaje = QString::fromUtf8(socket->readLine()).trimmed();
        qDebug() << "📥 Recibido:" << mensaje;


        if (mensaje.startsWith("ENLINEA:") || mensaje == "ESTADO?" || mensaje.startsWith("DESCONECTADO:")) {
            qDebug() << "🔃 Comando interno recibido, ignorado.";
            continue;
        }

        if (mensaje.startsWith("ESTADOS:")) {
            QMap<QString, QString> mapa;
            QStringList pares = mensaje.mid(8).split(";", Qt::SkipEmptyParts);
            for (const QString &par : pares) {
                QStringList partes = par.split(",");
                if (partes.size() == 2) {
                    QString usuario = partes[0].trimmed();
                    QString estado = partes[1].trimmed();
                    mapa[usuario] = estado;
                    qDebug() << "👤 Estado recibido ->" << usuario << ":" << estado;
                }
            }
            emit estadoUsuariosActualizado(mapa);
        }
        else if (mensaje.contains(":")) {
            QString remitente = mensaje.section(":", 0, 0).trimmed();
            QString contenido = mensaje.section(":", 1).trimmed();
            emit mensajeRecibido(remitente, contenido);
        } else {
            qDebug() << "⚠️ Formato no reconocido, ignorado:" << mensaje;
        }
    }
}

void ClienteSocket::onConnected()
{
    qDebug() << "✅ Conectado al servidor.";

    if (!nombreUsuario.isEmpty()) {
        QString lineaConexion = "ENLINEA:" + nombreUsuario;
        enviarMensaje(lineaConexion);

        QTimer::singleShot(500, [this]() {
            enviarMensaje("ESTADO?");
        });
    } else {
        qDebug() << "⚠️ nombreUsuario vacío en onConnected()";
    }
}

void ClienteSocket::onDisconnected()
{
    qDebug() << "🔌 Desconectado del servidor.";
    QTimer::singleShot(3000, this, &ClienteSocket::reconectar);
}
void ClienteSocket::desconectar()
{
    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        enviarMensaje("DESCONECTADO:" + nombreUsuario);
        socket->disconnectFromHost();
        qDebug() << "🔌 Cliente desconectado manualmente.";
    }
}
