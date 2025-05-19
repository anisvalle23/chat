#ifndef MENSAJE_GUARDADO_H
#define MENSAJE_GUARDADO_H

#include <QWidget>
#include <QString>

struct MensajeGuardado {
    QWidget* widget;
    int posicion;
    QString contenido;
    MensajeGuardado() : widget(nullptr), posicion(-1), contenido("") {}
    MensajeGuardado(QWidget* w, int pos, const QString& cont)
        : widget(w), posicion(pos), contenido(cont) {}
};

#endif
