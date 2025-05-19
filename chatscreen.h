#ifndef CHATSCREEN_H
#define CHATSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QMessageBox>
#include "pila_template.h"
#include "mensaje_guardado.h"
#include "cliente_socket.h"
#include <QDateTime>

class ChatWindow;

class ChatScreen : public QWidget
{
    Q_OBJECT

public:
    explicit ChatScreen(QWidget *parent = nullptr);
    void setContacto(const QString &nombre, const QString &avatarPath);
    void setUsuarioActual(const QString &usuario); // ← NUEVO
    void cargarMensajesDesdeArchivo(); // ← NUEVO
    void mostrarMensaje(const QString& mensaje, bool enviadoPorMi);
    void setClienteSocket(ClienteSocket *socket);
    void limpiarChat();
    QTimer *verificacionTimer;
    bool contactoMutuo = true;
    QFrame *bloqueoOverlay = nullptr;
    static QString nombreArchivoChat(const QString &usuario1, const QString &usuario2);
    void recibirMensaje(const QString& mensaje);  // manejador de sockets
    QString getContactoActual() const;
        void setVentanaPrincipal(ChatWindow *ventana); // Nuevo método

private slots:
    // void onEnviarClicked();  // conectado al botón "Enviar"
    void buscarMensajes(const QString &palabra);
    void navegarResultado(int direccion);
    void centrarMensaje(QWidget *mensaje);


private:
    // 👤 Encabezado
    QLabel *contactNameLabel;
    QLabel *avatarLabel;

    // 💬 Mensajes
    QVBoxLayout *messagesLayout;
    QWidget *messagesWidget;
    QScrollArea *scrollArea;

    // 📝 Entrada y botones
    QLineEdit *messageInput;
    QPushButton *sendButton;

    // 🗆 Stickers
    QWidget *stickerPopup;
    void crearGaleriaStickers();

    // 🧱 Deshacer mensaje eliminado con posición exacta
    Pila<MensajeGuardado> historialMensajes;

    // 👤 Usuario actual
    QString usuarioActual; // ← NUEVO
    QString contactoActual; // ← NUEVO: Nombre del contacto

    // Funciones principales
    void deshacerUltimoMensaje();       // ↩️ Restaurar último mensaje eliminado
    void eliminarMensaje(QWidget *msg); // ❌ Eliminar mensaje (con confirmación)

    // Funciones auxiliares
    void agregarContextMenu(QWidget *msgWidget); // Para mostrar botón de eliminar al hacer clic


    ClienteSocket* clienteSocket;

    QTimer *timerActualizarChat;
    QDateTime ultimaModificacionArchivo;
    void verificarActualizacionArchivo();
    QLineEdit *mensajeEdit;
    QPushButton *botonEnviar;
    void mostrarDialogoContactoNoAgregado();

    QWidget *panelBusqueda;
    QLineEdit *buscadorLineEdit;
    QPushButton *btnAnterior;
    QPushButton *btnSiguiente;
    QLabel *resultadoLabel;
    QPushButton *btnCerrarBusqueda;
    QPushButton *btnMostrarBusqueda;
    QList<QLabel*> mensajesResaltados;
    int indiceActual;

    ChatWindow *ventanaPrincipal = nullptr;



signals:
    void eliminarContacto(const QString &usuario);
        void mensajeRecibido(const QString& remitente, const QString& mensaje);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // CHATSCREEN_H
