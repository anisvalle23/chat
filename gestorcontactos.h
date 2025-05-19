#ifndef GESTORCONTACTOS_H
#define GESTORCONTACTOS_H

#include <QString>
#include <QStringList>

class GestorContactos {
public:
    // Devuelve la ruta del archivo del usuario
    static QString rutaArchivo(const QString& usuario);

    // Carga los contactos del archivo (usuario,nombre,avatar por línea)
    static QStringList cargarContactos(const QString& usuario);

    // Guarda la lista de contactos (usuario,nombre,avatar por línea)
    static void guardarContactos(const QString& usuario, const QStringList& contactos);
};

#endif // GESTORCONTACTOS_H
