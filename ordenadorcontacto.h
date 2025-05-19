#ifndef ORDENADORCONTACTOS_H
#define ORDENADORCONTACTOS_H

#include <QList>
#include <QString>
#include "usuario.h"

class OrdenadorContactos {
public:
    static QList<Usuario> ordenar(const QList<Usuario>& contactos, const QString& criterio, const QString& usuarioPrincipal);
};

#endif // ORDENADORCONTACTOS_H
