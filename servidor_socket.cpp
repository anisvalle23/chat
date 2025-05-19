#include "servidor_socket.h"
#include <QDebug>
#include <QTextStream>
#include <QTimer>
#include <QRegularExpression>
#include <QFile>
#include "usuario.h"

ServidorSocket::ServidorSocket(QObject* parent)
    : QTcpServer(parent)
{
}

void ServidorSocket::startServidor(quint16 puerto) {
    if (this->listen(QHostAddress::Any, puerto)) {
        qDebug() << "🟢 Servidor iniciado en el puerto:" << puerto;
    } else {
        qDebug() << "🔴 Error al iniciar el servidor:" << this->errorString();
    }
}

void ServidorSocket::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket* socket = new QTcpSocket(this);
    if (socket->setSocketDescriptor(socketDescriptor)) {
        connect(socket, &QTcpSocket::readyRead, this, &ServidorSocket::leerMensaje);
        connect(socket, &QTcpSocket::disconnected, this, &ServidorSocket::clienteDesconectado);
        clientes[socket] = "";  // Usuario aún no identificado
        qDebug() << "👤 Cliente conectado desde:" << socket->peerAddress().toString();
    } else {
        delete socket;
    }
}

void ServidorSocket::leerMensaje() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    while (socket->canReadLine()) {
        QString linea = QString::fromUtf8(socket->readLine()).trimmed();

        if (linea.startsWith("ENLINEA:")) {
            QString nombre = linea.mid(QString("ENLINEA:").length()).trimmed();
            clientes[socket] = nombre;
            usuariosConectados.insert(nombre);
            qDebug() << "🟢 Usuario conectado:" << nombre;

            actualizarEstadoEnArchivo(nombre, true);
            enviarEstadoConectados();
            continue;
        }

        if (linea.startsWith("DESCONECTADO:")) {
            QString nombre = linea.mid(QString("DESCONECTADO:").length()).trimmed();
            usuariosConectados.remove(nombre);
            actualizarEstadoEnArchivo(nombre, false);
            enviarEstadoConectados();
            continue;
        }

        if (linea.startsWith("ESTADO")) {
            QString solicitante = clientes.value(socket);
            if (!solicitante.isEmpty()) {
                qDebug() << "🔍 Estado solicitado por:" << solicitante;
                enviarEstadoConectados();
            } else {
            }
            continue;
        }

        QString emisor = clientes.value(socket);
        if (!emisor.isEmpty()) {
            QString mensajeFormateado = emisor + ": " + linea;

            for (QTcpSocket* otro : clientes.keys()) {
                if (otro != socket && otro->isOpen()) {
                    otro->write((mensajeFormateado + "\n").toUtf8());
                    otro->flush();
                }
            }
        } else {
        }
    }
}

void ServidorSocket::clienteDesconectado() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QString nombre = clientes.value(socket);
    if (!nombre.isEmpty()) {
        usuariosConectados.remove(nombre);
        actualizarEstadoEnArchivo(nombre, false);
        enviarEstadoConectados();
    }

    clientes.remove(socket);
    socket->deleteLater();
}

void ServidorSocket::enviarEstadoConectados() {
    QStringList listaUsuarios;

    QSet<QString> todosUsuarios;
    for (const QString& u : clientes.values()) {
        if (!u.isEmpty()) todosUsuarios.insert(u);
    }

    for (const QString& nombre : todosUsuarios) {
        QString estado = usuariosConectados.contains(nombre) ? "1" : "0";
        listaUsuarios << nombre + "," + estado;
    }

    QString mensaje = "ESTADOS:" + listaUsuarios.join(";");

    for (QTcpSocket* cliente : clientes.keys()) {
        if (cliente->isOpen()) {
            cliente->write((mensaje + "\n").toUtf8());
            cliente->flush();
        }
    }
}

void ServidorSocket::actualizarEstadoEnArchivo(const QString &usuario, bool conectado) {

    QString archivoRuta = "/Users/anavalle/Desktop/chat/usuarios.txt";
    QFile file(archivoRuta);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QStringList lineas;
    QTextStream in(&file);

    while (!in.atEnd()) {
        QString linea = in.readLine().trimmed();
        QStringList partes = linea.split(",");

        if (partes.size() >= 9 && partes[0].trimmed() == usuario.trimmed()) {
            partes[8] = conectado ? "1" : "0";
            linea = partes.join(",");
        }

        lineas << linea;
    }

    file.close();

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return;
    }

    QTextStream out(&file);
    for (const QString &l : lineas) {
        out << l << "\n";
    }
    file.close();
}
