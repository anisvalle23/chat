#ifndef USUARIO_H
#define USUARIO_H

#include <QString>

class Usuario {
public:
    Usuario(const QString &usuario,
            const QString &nombre,
            const QString &correo,
            const QString &contrasena,
            const QString &avatar,
            int edad,
            const QString &pregunta,
            const QString &respuesta,
            bool estadoConectado = false);

    bool guardarEnArchivo(const QString &rutaArchivo) const;
    static bool existeUsuario(const QString &rutaArchivo, const QString &usuarioBuscado);


    QString getUsuario() const { return usuario; }
    QString getNombre() const { return nombre; }
    QString getCorreo() const { return correo; }
    QString getContrasena() const { return contrasena; }
    QString getAvatar() const { return avatar; }
    int getEdad() const { return edad; }
    QString getPregunta() const { return pregunta; }
    QString getRespuesta() const { return respuesta; }
    bool getEstado() const { return estadoConectado; }


    void setAvatar(const QString &nuevaRuta) { avatar = nuevaRuta; }
    void setNombre(const QString &nuevoNombre) { nombre = nuevoNombre; }
    void setUsuario(const QString &nuevoUsuario) { usuario = nuevoUsuario; }
    void setContrasena(const QString &nuevaContrasena) { contrasena = nuevaContrasena; }
    void setEstado(bool nuevoEstado) { estadoConectado = nuevoEstado; }


    static void actualizarEstadoEnArchivo(const QString &usuarioBuscado, bool conectado);
    static bool sonContactosMutuos(const QString &usuario1, const QString &usuario2);

private:
    QString usuario;
    QString nombre;
    QString correo;
    QString contrasena;
    QString avatar;
    int edad;
    QString pregunta;
    QString respuesta;
    bool estadoConectado = false;
};

#endif
