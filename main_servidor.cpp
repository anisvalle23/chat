#include <QCoreApplication>
#include "servidor_socket.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    ServidorSocket servidor;
    servidor.startServidor(12345);

    qDebug() << "🖥️ Servidor corriendo... esperando conexiones";

    return app.exec();
}
