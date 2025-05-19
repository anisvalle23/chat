#include "usuario.h"
#include <QFile>
#include <QTextStream>
#include <QDir>

Usuario::Usuario(const QString &usuario,
                 const QString &nombre,
                 const QString &correo,
                 const QString &contrasena,
                 const QString &avatar,
                 int edad,
                 const QString &pregunta,
                 const QString &respuesta,
                 bool estadoConectado)
    : usuario(usuario), nombre(nombre), correo(correo), contrasena(contrasena),
    avatar(avatar), edad(edad), pregunta(pregunta), respuesta(respuesta),
    estadoConectado(estadoConectado) {}

bool Usuario::guardarEnArchivo(const QString &rutaArchivo) const {
    QString rutaCompleta = "/Users/anavalle/Desktop/chat/usuarios.txt";

    QFile file(rutaCompleta);
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << usuario << ","
        << nombre << ","
        << correo << ","
        << contrasena << ","
        << avatar << ","
        << edad << ","
        << pregunta << ","
        << respuesta << ","
        << "0" << "\n";  // Estado desconectado por defecto

    file.close();
    return true;
}

bool Usuario::existeUsuario(const QString &rutaArchivo, const QString &usuarioBuscado) {
    QString rutaCompleta = "/Users/anavalle/Desktop/chat/usuarios.txt";

    QFile file(rutaCompleta);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString linea = in.readLine();
        QStringList partes = linea.split(",");
        if (!partes.isEmpty() && partes[0] == usuarioBuscado) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

QList<Usuario> cargarUsuariosDesdeArchivo() {
    QList<Usuario> usuarios;
    QString rutaCompleta = "/Users/anavalle/Desktop/chat/usuarios.txt";

    QFile file(rutaCompleta);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return usuarios;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString linea = in.readLine();
        QStringList partes = linea.split(",");
        if (partes.size() >= 9) {
            Usuario usuario(
                partes[0],
                partes[1],
                partes[2],
                partes[3],
                partes[4],
                partes[5].toInt(),
                partes[6],
                partes[7],
                partes[8].trimmed() == "1"
                );
            usuarios.append(usuario);
        }
    }
    file.close();
    return usuarios;
}

void Usuario::actualizarEstadoEnArchivo(const QString &usuarioBuscado, bool conectado) {
    QString rutaCompleta = "/Users/anavalle/Desktop/chat/usuarios.txt";
    QFile file(rutaCompleta);
    QStringList lineasActualizadas;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed();
            QStringList partes = linea.split(",");

            if (partes.size() >= 8 && partes[0] == usuarioBuscado) {
                while (partes.size() < 9) partes << "0";  // Asegura que tenga campo de estado
                partes[8] = conectado ? "1" : "0";
                linea = partes.join(",");
            }

            lineasActualizadas << linea;
        }
        file.close();
    }

    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        for (const QString &linea : lineasActualizadas) {
            out << linea << "\n";
        }
        file.close();
    }
}

bool Usuario::sonContactosMutuos(const QString &usuario1, const QString &usuario2)
{
    QString nombre1 = usuario1.toLower();
    QString nombre2 = usuario2.toLower();

    QString basePath = "/Users/anavalle/Desktop/chat/contactos/";
    QString ruta1 = basePath + "contactos_" + nombre1 + ".txt";
    QString ruta2 = basePath + "contactos_" + nombre2 + ".txt";

    qDebug() << "📁 Verificando contactos mutuos entre:" << nombre1 << "y" << nombre2;
    qDebug() << "📄 Ruta1:" << ruta1 << "Existe?" << QFile::exists(ruta1);
    qDebug() << "📄 Ruta2:" << ruta2 << "Existe?" << QFile::exists(ruta2);

    bool unoTieneAlOtro = false;
    bool otroTieneAlUno = false;

    QFile f1(ruta1);
    if (f1.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f1);
        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed().toLower();
            if (linea.startsWith(nombre2 + ",")) {
                unoTieneAlOtro = true;
                break;
            }
        }
        f1.close();
    }

    QFile f2(ruta2);
    if (f2.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f2);
        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed().toLower();
            if (linea.startsWith(nombre1 + ",")) {
                otroTieneAlUno = true;
                break;
            }
        }
        f2.close();
    }

    qDebug() << "🤝 Contacto mutuo confirmado:" << (unoTieneAlOtro && otroTieneAlUno);
    return unoTieneAlOtro && otroTieneAlUno;
}
