#ifndef GESTORNOLEIDOS_H
#define GESTORNOLEIDOS_H

#include <QString>
#include <QStringList>

class GestorNoLeidos {
public:
    static void registrar(const QString& usuario, const QString& contacto, const QString& mensaje);
    static QStringList obtenerMensajes(const QString& usuario, const QString& contacto);
    static int contarMensajes(const QString& usuario, const QString& contacto);
    static void limpiar(const QString& usuario, const QString& contacto);

private:
    static QString archivoRuta(const QString& usuario);
};

#endif // GESTORNOLEIDOS_H
