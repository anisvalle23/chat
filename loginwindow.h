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

private:
    QLabel *ilustracionLabel;
    QLabel *tituloLabel;
    QLabel *subtituloLabel;
    QPushButton *continuarButton;
    QFrame *loginFrame;
    QLineEdit *usuarioEdit;
    QLineEdit *contrasenaEdit;
    QPushButton *iniciarButton;
    QPushButton *registroButton;
    QPushButton *cerrarButton;
    QFrame *registroFrame;
    QPushButton *cerrarRegistroBtn;
    void crearFormularioRegistro();

    ChatWindow *chatWindow;
    ClienteSocket *clienteSocket;
    void abrirChatConUsuario(const Usuario &usuario);
};

#endif
