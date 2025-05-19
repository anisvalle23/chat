#ifndef GESTORVISTACHAT_H
#define GESTORVISTACHAT_H

#include <QString>
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include "usuario.h"

class GestorVistaChat {
public:
    static QWidget* crearWidgetContacto(const Usuario& contacto,
                                        const QString& usuarioActual,
                                        QLabel*& ultimoMensajeLabel,
                                        QLabel*& contadorLabel);

    static int contarSolicitudes(const QString& usuarioActual, const QListWidget* contactList);
    static QString obtenerEstadoDesdeArchivo(const QString& usuario);
};

#endif
