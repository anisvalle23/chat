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
#include <QMap>  // ✅ Para manejar estados en línea
#include "usuario.h"
#include "chatscreen.h"
#include "cliente_socket.h"  // ✅ Cliente socket para estados
#include "cola.h"
#include <QMap>
#include "ordenadorcontacto.h"

class LoginWindow;  // ← ✅ Declaración adelantada

class ChatWindow : public QWidget {
    Q_OBJECT
public:
    explicit ChatWindow(const Usuario &usuarioActivo, ClienteSocket* socket, QWidget *parent = nullptr);

    ChatScreen* getChatScreen() const; // ← ✅ Método público para acceder a chatScreen
    QTimer *estadoTimer;
    void verificarEstadosDesdeArchivo();
    QMap<QString, QString> estadoAnteriorUsuarios;
    void mostrarPantallaBienvenida();
    QLineEdit *searchField;
    QVector<Usuario> contactosCargados;  // para almacenar todos los contactos antes del filtrado
    QTimer *solicitudesTimer;
    void verificarSolicitudesDesdeArchivo();
    void registrarMensajeNoLeido(const QString& contacto, const QString& mensaje);
    QMap<QString, Cola<QString>> mensajesNoLeidos;
    QMap<QString, QLabel*> contadoresNoLeidos;  // ← ✅ ¡Esta línea faltaba!
    void actualizarContador(const QString& contacto);
    void actualizarUltimoMensaje(const QString& contacto, const QString& mensaje);
    void limpiarContador(const QString& contacto);
    void verificarMensajesNoLeidos();
    void cargarContactosOrdenados(const QString& criterio);
    QList<Usuario> obtenerContactosDesdeArchivo();

private slots:
    void actualizarEstadosEnLista(const QMap<QString, QString> &estados);  // ✅ Slot para actualizar estados
    void cerrarSesion();
        void verificarNotificaciones();

private:
    Usuario usuario;
    QString obtenerEstadoDesdeArchivo(const QString& usuario);

    // 🔵 Stack principal de contenido
    QStackedWidget *contentStack;

    // 🟩 Pantalla de chats
    QListWidget *contactList;
    QLineEdit *messageInput;
    QPushButton *sendButton;
    QLabel *contactNameLabel;
    QVBoxLayout *chatLayout;

    // ⚙️ Pantalla de configuración
    QWidget *settingsScreen;

    // 🔵 Carga de usuarios
    QVector<Usuario> usuariosTotales;
    QVector<Usuario> usuariosDisponibles;

    // 🔧 Métodos auxiliares que ya existen en el .cpp
    void cargarUsuariosDesdeArchivo();
    void filtrarUsuariosDisponibles();
    void guardarContactosActuales();
    void cargarContactosGuardados();

    // 📨 Solicitudes de contacto
    QListWidget *solicitudesList = nullptr;
    QPushButton *solicitudesButton = nullptr;
    QLabel *solicitudesLabel = nullptr;

    void cargarSolicitudesPendientes();
    void agregarContactoMutuo(const QString &otroUsuario);
    void eliminarSolicitud(const QString &usuario);
    // void actualizarContadorSolicitudes();  // ← ✅ Método auxiliars

    // 🔐 Ventana de login (para cerrar sesión)
    LoginWindow *loginWindow;

    // 🏠 Bienvenida
    QWidget *welcomeWidget = nullptr;
    QLabel *logoHome = nullptr;
    QLabel *welcomeText = nullptr;
    ChatScreen *chatScreen = nullptr;  // 🔴 Agrega esta línea al private

    // 🧑‍💻 Chat individual
    ChatScreen *chatWidget = nullptr;

    // ✅ Cliente socket para recibir actualizaciones
    ClienteSocket *clienteSocket = nullptr;
    QTimer *solicitudesListTimer;

    QTimer *contactosTimer;

QTimer *actualizarChatsTimer;  // 👈 Nuevo timer
 QTimer* mensajesNoLeidosTimer;
bool enModoSolicitudes = false; // <--- Agregalo al final del constructor o en el .h
void verificarActualizacionArchivo(); // 🔄 Revisa cambios en los archivos de chat
void verificarActualizacionMensajesNoLeidos();

QTimer* notificacionesTimer;

    void eliminarCuenta(const QString &usuario);
};

#endif // CHATWINDOW_H
