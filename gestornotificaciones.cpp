#include "gestornotificaciones.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>

QMap<QString, int> GestorNotificaciones::leerNotificaciones(const QString& usuario) {
    QMap<QString, int> conteo;
    QString ruta = "/Users/anavalle/Desktop/chat/notificaciones/notificaciones_" + usuario + ".txt";

    QFile archivo(ruta);
    if (archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&archivo);
        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed();
            QStringList partes = linea.split(",");
            if (partes.size() == 2) {
                conteo[partes[0]] = partes[1].toInt();
            }
        }
        archivo.close();
    } else {
        qDebug() << "⚠️ No se pudo leer notificaciones de:" << ruta;
    }

    return conteo;
}

void GestorNotificaciones::guardarNotificaciones(const QString& usuario, const QMap<QString, int>& conteo) {
    QString carpeta = "/Users/anavalle/Desktop/chat/notificaciones";
    QDir dir;
    if (!dir.exists(carpeta)) {
        dir.mkpath(carpeta);
    }

    QString ruta = carpeta + "/notificaciones_" + usuario + ".txt";
    QFile archivo(ruta);
    if (archivo.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&archivo);
        for (auto it = conteo.begin(); it != conteo.end(); ++it) {
            out << it.key() << "," << it.value() << "\n";
        }
        archivo.close();
        qDebug() << "✅ Notificaciones guardadas en:" << ruta;
    } else {
        qDebug() << "❌ No se pudo guardar notificaciones en:" << ruta;
    }
}

void GestorNotificaciones::incrementar(const QString& usuario, const QString& remitente) {
    QMap<QString, int> conteo = leerNotificaciones(usuario);
    conteo[remitente] += 1;
    guardarNotificaciones(usuario, conteo);
}

void GestorNotificaciones::reiniciar(const QString& usuario, const QString& remitente) {
    QMap<QString, int> conteo = leerNotificaciones(usuario);
    conteo[remitente] = 0;
    guardarNotificaciones(usuario, conteo);
}


int GestorNotificaciones::obtenerConteo(const QString& usuario, const QString& remitente) {
    QMap<QString, int> conteo = leerNotificaciones(usuario);
    return conteo.value(remitente, 0);
}


bool GestorNotificaciones::existeArchivo(const QString& usuario) {
    QString ruta = "/Users/anavalle/Desktop/chat/notificaciones/notificaciones_" + usuario + ".txt";
    return QFile::exists(ruta);
}
