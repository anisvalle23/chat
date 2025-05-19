#ifndef MENSAJE_GUARDADO_H
#define MENSAJE_GUARDADO_H

#include <QWidget>
#include <QString>

struct MensajeGuardado {
    QWidget* widget;
    int posicion;
    QString contenido;  // ✅ NUEVO: texto del mensaje eliminado

    // Constructor por defecto
    MensajeGuardado() : widget(nullptr), posicion(-1), contenido("") {}

    // Constructor que usas al apilar
    MensajeGuardado(QWidget* w, int pos, const QString& cont)
        : widget(w), posicion(pos), contenido(cont) {}
};

#endif // MENSAJE_GUARDADO_H
