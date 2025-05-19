#include "gestornoleidos.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>

QString GestorNoLeidos::archivoRuta(const QString& usuario) {
    QString baseDir = "/Users/anavalle/Desktop/chat/noleidos/";
    QDir().mkpath(baseDir); // Asegura que exista el directorio
    return baseDir + "noleidos_" + usuario.toLower() + ".txt";
}

void GestorNoLeidos::registrar(const QString& usuario, const QString& contacto, const QString& mensaje) {
    QString ruta = archivoRuta(usuario);
    QFile file(ruta);

    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << contacto.toLower() << "|" << mensaje << "\n";
        file.close();
        qDebug() << "📥 Mensaje no leído registrado:" << contacto << mensaje;
    } else {
        qDebug() << "❌ No se pudo abrir el archivo para escribir:" << ruta;
    }
}

QStringList GestorNoLeidos::obtenerMensajes(const QString& usuario, const QString& contacto) {
    QStringList mensajes;
    QString ruta = archivoRuta(usuario);
    QFile file(ruta);

    if (!file.exists()) {
        return mensajes;
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed();
            QStringList partes = linea.split("|");
            if (partes.size() == 2 && partes[0].toLower() == contacto.toLower()) {
                mensajes.append(partes[1]);
            }
        }
        file.close();
    } else {
        qDebug() << "❌ No se pudo abrir archivo para leer mensajes:" << ruta;
    }

    return mensajes;
}


int GestorNoLeidos::contarMensajes(const QString& usuario, const QString& contacto) {
    return obtenerMensajes(usuario, contacto).size();
}


void GestorNoLeidos::limpiar(const QString& usuario, const QString& contacto) {
    QString ruta = archivoRuta(usuario);
    QFile file(ruta);
    QStringList nuevasLineas;

    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed();
            if (!linea.startsWith(contacto.toLower() + "|")) {
                nuevasLineas << linea;
            }
        }
        file.close();
    }

    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        for (const QString& l : nuevasLineas) {
            out << l << "\n";
        }
        file.close();
        qDebug() << "🧽 Mensajes no leídos eliminados de" << contacto;
    } else {
        qDebug() << "❌ No se pudo limpiar archivo:" << ruta;
    }
}
