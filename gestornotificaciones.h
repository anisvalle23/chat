#ifndef GESTORNOTIFICACIONES_H
#define GESTORNOTIFICACIONES_H

#include <QString>
#include <QMap>

class GestorNotificaciones {
public:
    static QMap<QString, int> leerNotificaciones(const QString& usuario);
    static void guardarNotificaciones(const QString& usuario, const QMap<QString, int>& conteo);
    static void incrementar(const QString& usuario, const QString& remitente);
    static void reiniciar(const QString& usuario, const QString& remitente);
    static int obtenerConteo(const QString& usuario, const QString& remitente);
    static bool existeArchivo(const QString& usuario);
};
#endif
