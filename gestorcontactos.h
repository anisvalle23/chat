#ifndef GESTORCONTACTOS_H
#define GESTORCONTACTOS_H

#include <QString>
#include <QStringList>

class GestorContactos {
public:
    static QString rutaArchivo(const QString& usuario);
    static QStringList cargarContactos(const QString& usuario);
    static void guardarContactos(const QString& usuario, const QStringList& contactos);
};

#endif
