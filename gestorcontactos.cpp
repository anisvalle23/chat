#include "gestorcontactos.h"
#include <QFile>
#include <QTextStream>
#include <QDir>

QString GestorContactos::rutaArchivo(const QString& usuario) {
    // Asegura que la carpeta ~/chat/contactos/ exista
    QDir dir(QDir::homePath() + "/chat/contactos");
    if (!dir.exists())
        dir.mkpath(".");
    return dir.filePath("contactos_" + usuario + ".txt");
}

QStringList GestorContactos::cargarContactos(const QString& usuario) {
    QString archivo = rutaArchivo(usuario);
    QStringList contactos;

    QFile file(archivo);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed();
            if (!linea.isEmpty())
                contactos.append(linea);
        }
        file.close();
    }
    return contactos;
}

void GestorContactos::guardarContactos(const QString& usuario, const QStringList& contactos) {
    QString archivo = rutaArchivo(usuario);
    QFile file(archivo);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        for (const QString& contacto : contactos) {
            out << contacto << "\n";
        }
        file.close();
    }
}
