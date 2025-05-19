#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QLineEdit>
#include <QComboBox>
#include "chatwindow.h"
#include "cliente_socket.h"

class ChatWindow;

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    void usuarioLogueado(const QString &nombreUsuario);

private slots:
    void mostrarLogin();
    void cerrarLogin();
    void mostrarRegistro();
    void cerrarRegistro();
    // void autenticarUsuario();  // 👈 Añadido para iniciar sesión

private:
    // Bienvenida
    QLabel *ilustracionLabel;
    QLabel *tituloLabel;
    QLabel *subtituloLabel;
    QPushButton *continuarButton;

    // Login
    QFrame *loginFrame;
    QLineEdit *usuarioEdit;
    QLineEdit *contrasenaEdit;
    QPushButton *iniciarButton;
    QPushButton *registroButton;
    QPushButton *cerrarButton;

    // Registro
    QFrame *registroFrame;
    QPushButton *cerrarRegistroBtn;
    void crearFormularioRegistro();

    // Menú principal (chat)
    ChatWindow *chatWindow;

    // ✅ Socket que se inicializa al inicio
    ClienteSocket *clienteSocket;

    void abrirChatConUsuario(const Usuario &usuario);
};

#endif // LOGINWINDOW_H
