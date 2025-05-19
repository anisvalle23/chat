#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QVector>
#include <QMap>
#include "usuario.h"
#include "chatscreen.h"
#include "cliente_socket.h"
#include "cola.h"
#include <QMap>
#include "ordenadorcontacto.h"

class LoginWindow;

class ChatWindow : public QWidget {
    Q_OBJECT
public:
    explicit ChatWindow(const Usuario &usuarioActivo, ClienteSocket* socket, QWidget *parent = nullptr);

    ChatScreen* getChatScreen() const;
    QTimer *estadoTimer;
    void verificarEstadosDesdeArchivo();
    QMap<QString, QString> estadoAnteriorUsuarios;
    void mostrarPantallaBienvenida();
    QLineEdit *searchField;
    QVector<Usuario> contactosCargados;
    QTimer *solicitudesTimer;
    void verificarSolicitudesDesdeArchivo();
    void registrarMensajeNoLeido(const QString& contacto, const QString& mensaje);
    QMap<QString, Cola<QString>> mensajesNoLeidos;
    QMap<QString, QLabel*> contadoresNoLeidos;
    void actualizarContador(const QString& contacto);
    void actualizarUltimoMensaje(const QString& contacto, const QString& mensaje);
    void limpiarContador(const QString& contacto);
    void verificarMensajesNoLeidos();
    void cargarContactosOrdenados(const QString& criterio);
    QList<Usuario> obtenerContactosDesdeArchivo();

private slots:
    void actualizarEstadosEnLista(const QMap<QString, QString> &estados);
    void cerrarSesion();
        void verificarNotificaciones();

private:
    Usuario usuario;
    QString obtenerEstadoDesdeArchivo(const QString& usuario);
    QStackedWidget *contentStack;
    QListWidget *contactList;
    QLineEdit *messageInput;
    QPushButton *sendButton;
    QLabel *contactNameLabel;
    QVBoxLayout *chatLayout;
    QWidget *settingsScreen;
    QVector<Usuario> usuariosTotales;
    QVector<Usuario> usuariosDisponibles;
    void cargarUsuariosDesdeArchivo();
    void filtrarUsuariosDisponibles();
    void guardarContactosActuales();
    void cargarContactosGuardados();


    QListWidget *solicitudesList = nullptr;
    QPushButton *solicitudesButton = nullptr;
    QLabel *solicitudesLabel = nullptr;

    void cargarSolicitudesPendientes();
    void agregarContactoMutuo(const QString &otroUsuario);
    void eliminarSolicitud(const QString &usuario);


    LoginWindow *loginWindow;

    QWidget *welcomeWidget = nullptr;
    QLabel *logoHome = nullptr;
    QLabel *welcomeText = nullptr;
    ChatScreen *chatScreen = nullptr;
    ChatScreen *chatWidget = nullptr;
    ClienteSocket *clienteSocket = nullptr;
    QTimer *solicitudesListTimer;

    QTimer *contactosTimer;

QTimer *actualizarChatsTimer;
 QTimer* mensajesNoLeidosTimer;
bool enModoSolicitudes = false;
void verificarActualizacionArchivo();
void verificarActualizacionMensajesNoLeidos();

QTimer* notificacionesTimer;

    void eliminarCuenta(const QString &usuario);
};

#endif
