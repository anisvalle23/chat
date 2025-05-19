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
    void setUsuarioActual(const QString &usuario);
    void cargarMensajesDesdeArchivo();
    void mostrarMensaje(const QString& mensaje, bool enviadoPorMi);
    void setClienteSocket(ClienteSocket *socket);
    void limpiarChat();
    QTimer *verificacionTimer;
    bool contactoMutuo = true;
    QFrame *bloqueoOverlay = nullptr;
    static QString nombreArchivoChat(const QString &usuario1, const QString &usuario2);
    void recibirMensaje(const QString& mensaje);
    QString getContactoActual() const;
        void setVentanaPrincipal(ChatWindow *ventana);

private slots:
    void buscarMensajes(const QString &palabra);
    void navegarResultado(int direccion);
    void centrarMensaje(QWidget *mensaje);


private:

    QLabel *contactNameLabel;
    QLabel *avatarLabel;
    QVBoxLayout *messagesLayout;
    QWidget *messagesWidget;
    QScrollArea *scrollArea;
    QLineEdit *messageInput;
    QPushButton *sendButton;
    QWidget *stickerPopup;
    void crearGaleriaStickers();
    Pila<MensajeGuardado> historialMensajes;
    QString usuarioActual;
    QString contactoActual;
    void deshacerUltimoMensaje();
    void eliminarMensaje(QWidget *msg);
    void agregarContextMenu(QWidget *msgWidget);
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
