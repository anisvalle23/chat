#include "loginwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QPixmap>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include "chatwindow.h"
#include "usuario.h"
#include <QMessageBox>
#include "PasswordLineEdit.h"
#include "StyledComboBox.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(800, 650);
    setStyleSheet("background-color: #1e1e1e; color: black;");

    // --- BIENVENIDA ---
    QFrame *contenedor = new QFrame(this);
    contenedor->setFixedSize(800, 650);
    contenedor->move((width() - contenedor->width()) / 2, (height() - contenedor->height()) / 2);

    ilustracionLabel = new QLabel(contenedor);
    ilustracionLabel->setFixedSize(240, 240);
    ilustracionLabel->setStyleSheet("background-color: #2e2e2e; border-radius: 120px;");
    ilustracionLabel->setAlignment(Qt::AlignCenter);

    QPixmap imagen("/Users/anavalle/Desktop/logo.png");
    ilustracionLabel->setPixmap(imagen.scaled(190, 190, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    tituloLabel = new QLabel("Bienvenido a ChatApp", contenedor);
    tituloLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");
    tituloLabel->setAlignment(Qt::AlignCenter);

    subtituloLabel = new QLabel("Conéctate de forma privada con tus contactos", contenedor);
    subtituloLabel->setStyleSheet("color: #cccccc; font-size: 14px;");
    subtituloLabel->setAlignment(Qt::AlignCenter);

    continuarButton = new QPushButton("Continuar", contenedor);
    continuarButton->setFixedSize(150, 40);
    continuarButton->setStyleSheet(
        "QPushButton { background-color: #800020; color: white; border: none; border-radius: 20px; font-weight: bold; }"
        "QPushButton:hover { background-color: #a83232; }");
    connect(continuarButton, &QPushButton::clicked, this, &LoginWindow::mostrarLogin);

    QVBoxLayout *layout = new QVBoxLayout(contenedor);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);
    layout->addWidget(ilustracionLabel, 0, Qt::AlignCenter);
    layout->addWidget(tituloLabel, 0, Qt::AlignCenter);
    layout->addWidget(subtituloLabel, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(continuarButton, 0, Qt::AlignHCenter);

    // --- LOGIN ---
    loginFrame = new QFrame(this);
    loginFrame->setFixedSize(800, 650);
    loginFrame->move(0, 0);
    loginFrame->setStyleSheet("background-color: rgba(0, 0, 0, 180);");
    loginFrame->hide();

    QFrame *panelLogin = new QFrame(loginFrame);
    panelLogin->setFixedSize(400, 300);
    QVBoxLayout *frameLayout = new QVBoxLayout(loginFrame);
    frameLayout->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(panelLogin);
    panelLogin->setStyleSheet("background-color: white; border-radius: 20px; color: black;");

    cerrarButton = new QPushButton("×", panelLogin);
    cerrarButton->setFixedSize(30, 30);
    cerrarButton->move(360, 10);
    cerrarButton->setStyleSheet("QPushButton { background-color: transparent; color: #999; font-size: 20px; border: none; }"
                                "QPushButton:hover { color: #e74c3c; }");
    connect(cerrarButton, &QPushButton::clicked, this, &LoginWindow::cerrarLogin);

    QLabel *loginTitulo = new QLabel("Iniciar sesión", panelLogin);
    loginTitulo->setAlignment(Qt::AlignCenter);
    loginTitulo->setStyleSheet("font-size: 20px; font-weight: bold; color: black;");

    usuarioEdit = new QLineEdit(panelLogin);
    usuarioEdit->setPlaceholderText("Usuario");
    usuarioEdit->setStyleSheet("color: black; padding: 10px; border: 1px solid #ccc; border-radius: 12px;");

    contrasenaEdit = new QLineEdit(panelLogin);
    contrasenaEdit->setPlaceholderText("Contraseña");
    contrasenaEdit->setEchoMode(QLineEdit::Password);
    contrasenaEdit->setStyleSheet("color: black; padding: 10px; border: 1px solid #ccc; border-radius: 12px;");

    iniciarButton = new QPushButton("Iniciar", panelLogin);
    iniciarButton->setStyleSheet("background-color: #800020; color: white; padding: 10px; border-radius: 12px; font-weight: bold;");

    connect(iniciarButton, &QPushButton::clicked, this, [=]() {
        QString nombreUsuario = usuarioEdit->text().trimmed();
        QString contrasenaIngresada = contrasenaEdit->text();
        QString rutaUsuarios = "/Users/anavalle/Desktop/chat/usuarios.txt";

        bool loginExitoso = false;
        Usuario usuarioActivo("", "", "", "", "", 0, "", "");

        QFile file(rutaUsuarios);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString linea = in.readLine();
                QStringList datos = linea.split(",");
                if (datos.size() >= 8) {
                    if (datos[0] == nombreUsuario && datos[3] == contrasenaIngresada) {
                        usuarioActivo = Usuario(datos[0], datos[1], datos[2], datos[3],
                                                datos[4], datos[5].toInt(), datos[6], datos[7]);
                        loginExitoso = true;

                        // ✅ Actualiza el estado a 1 en el archivo
                        QFile updateFile(rutaUsuarios);
                        if (updateFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                            QStringList lineas;
                            QTextStream inUpdate(&updateFile);
                            while (!inUpdate.atEnd()) {
                                QString l = inUpdate.readLine();
                                QStringList partes = l.split(",");
                                if (partes.size() >= 9 && partes[0] == nombreUsuario) {
                                    partes[8] = "1";
                                    lineas << partes.join(",");
                                } else {
                                    lineas << l;
                                }
                            }
                            updateFile.close();

                            if (updateFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                                QTextStream outUpdate(&updateFile);
                                for (const QString &l : lineas)
                                    outUpdate << l << "\n";
                                updateFile.close();
                            }
                        }

                        break;
                    }
                }
            }
            file.close();
        }

        if (loginExitoso) {
            ClienteSocket* socket = new ClienteSocket(this);
            socket->setNombreUsuario(usuarioActivo.getUsuario());
            socket->conectarServidor("127.0.0.1", 12345);

            chatWindow = new ChatWindow(usuarioActivo, socket);
            chatWindow->getChatScreen()->setUsuarioActual(usuarioActivo.getUsuario());
            chatWindow->getChatScreen()->setClienteSocket(socket);
            chatWindow->show();
            this->close();
        } else {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Error de Inicio de Sesión");
            msgBox.setText("⚠️ Usuario o contraseña incorrectos.");
            msgBox.setStyleSheet(R"(
    QMessageBox {
        background-color: #2e2e2e;
        color: white;
        font-size: 14px;
        border-radius: 12px;
    }
    QPushButton {
        background-color: #800020;
        color: white;
        border-radius: 8px;
        padding: 6px 14px;
        min-width: 80px;
    }
    QPushButton:hover {
        background-color: #a83232;
    }
)");
            msgBox.exec();
        }
    });

    registroButton = new QPushButton("Registrarse", panelLogin);
    registroButton->setFlat(true);
    registroButton->setStyleSheet("color: #800020; text-decoration: underline;");
    connect(registroButton, &QPushButton::clicked, this, &LoginWindow::mostrarRegistro);

    QVBoxLayout *loginLayout = new QVBoxLayout(panelLogin);
    loginLayout->setSpacing(12);
    loginLayout->setContentsMargins(30, 30, 30, 30);
    loginLayout->addWidget(loginTitulo);
    loginLayout->addWidget(usuarioEdit);
    loginLayout->addWidget(contrasenaEdit);
    loginLayout->addWidget(iniciarButton);
    loginLayout->addWidget(registroButton);

    // --- REGISTRO ---
    crearFormularioRegistro();
}

void LoginWindow::crearFormularioRegistro() {
    registroFrame = new QFrame(this);
    registroFrame->setFixedSize(800, 650);
    registroFrame->move(0, 0);
    registroFrame->setStyleSheet("background-color: rgba(0, 0, 0, 180);");
    registroFrame->hide();

    QFrame *panelRegistro = new QFrame(registroFrame);
    panelRegistro->setFixedSize(500, 600);
    panelRegistro->setStyleSheet("background-color: #fefefe; border-radius: 20px; color: black;");

    // ✅ Este layout centrará el panelRegistro dentro de registroFrame
    QVBoxLayout *frameLayout = new QVBoxLayout(registroFrame);
    frameLayout->setAlignment(Qt::AlignCenter);
    frameLayout->addWidget(panelRegistro);

    cerrarRegistroBtn = new QPushButton("\u00d7", panelRegistro);
    cerrarRegistroBtn->setFixedSize(30, 30);
    cerrarRegistroBtn->move(460, 10);
    cerrarRegistroBtn->setStyleSheet("QPushButton { background-color: transparent; color: #999; font-size: 20px; border: none; }"
                                     "QPushButton:hover { color: #e74c3c; }");
    connect(cerrarRegistroBtn, &QPushButton::clicked, this, &LoginWindow::cerrarRegistro);

    QLabel *tituloRegistro = new QLabel("Crear cuenta", panelRegistro);
    tituloRegistro->setAlignment(Qt::AlignCenter);
    tituloRegistro->setStyleSheet("font-size: 22px; font-weight: bold; color: black; margin-bottom: 6px;");

    QString estiloInputOvalado = R"(
    background-color: white;
    color: black;
    font-size: 15px;
    padding: 20px 20px;
    border: 1px solid #ccc;
    border-radius: 20px;
    margin-bottom: 2px;
)";

    auto crearInput = [&](const QString &placeholder) {
        QLineEdit *edit = new QLineEdit(panelRegistro);
        edit->setPlaceholderText(placeholder);
        edit->setStyleSheet(estiloInputOvalado);
        return edit;
    };

    QLineEdit *usuario = crearInput("Nombre de usuario");
    QLineEdit *nombre = crearInput("Nombre completo");
    QLineEdit *correo = crearInput("Correo electrónico");
    PasswordLineEdit *pass = new PasswordLineEdit(panelRegistro);
    pass->setPlaceholderText("Contraseña");
    pass->setStyleSheet(estiloInputOvalado);
    PasswordLineEdit *passConf = new PasswordLineEdit(panelRegistro);
    passConf->setPlaceholderText("Confirmar contraseña");
    passConf->setStyleSheet(estiloInputOvalado);
    QLineEdit *edad = crearInput("Edad (mayor a 12)");
    QLineEdit *respuestaSeguridad = crearInput("Respuesta de seguridad");

    QLabel *avatarPathLabel = new QLabel("Ning\u00fan archivo seleccionado", panelRegistro);
    avatarPathLabel->setStyleSheet("font-size: 11px; color: gray;");

    QLabel *preview = new QLabel(panelRegistro);
    preview->setFixedSize(48, 48);
    preview->setStyleSheet("border: 1px solid #ccc; border-radius: 24px; background-color: white;");
    preview->setAlignment(Qt::AlignCenter);

    QPushButton *avatarBtn = new QPushButton("📁 Seleccionar avatar", panelRegistro);
    avatarBtn->setStyleSheet(R"(
        QPushButton {
            padding: 2px 2px;
            background-color: #f4f4f4;
            color: black;
            font-size: 14px;
            font-weight: bold;
            border-radius: 14px;
            border: 1px solid #ddd;
        }
        QPushButton:hover {
            background-color: #eaeaea;
        }
    )");

    connect(avatarBtn, &QPushButton::clicked, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Seleccionar Avatar", "", "Im\u00e1genes (*.png *.jpg *.jpeg)");
        if (!fileName.isEmpty()) {
            avatarPathLabel->setText(fileName);
            QPixmap avatar(fileName);
            preview->setPixmap(avatar.scaled(preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    });

    QHBoxLayout *avatarLayout = new QHBoxLayout();
    avatarLayout->addWidget(avatarBtn);
    avatarLayout->addWidget(preview);
    avatarLayout->setSpacing(12);

    StyledComboBox *preguntaSeguridad = new StyledComboBox(panelRegistro);
    preguntaSeguridad->addItems({
        "Nombre de tu primera mascota",
        "Nombre de tu escuela primaria",
        "Ciudad de nacimiento"
    });

    preguntaSeguridad->setStyleSheet(R"(
    QComboBox {
        background-color: white;
        color: black;  /* Aplica si el sistema respeta esto */
        font-size: 15px;
        padding: 12px 20px;
        border: 1px solid #ccc;
        border-radius: 20px;
        margin-bottom: 6px;
    }

    QComboBox::drop-down {
        border: none;
        padding-right: 6px;
    }

    QComboBox::down-arrow {
        image: none;
    }

    QComboBox QAbstractItemView {
        background-color: white;
        color: black;
        border: 1px solid #ccc;
        border-radius: 12px;
        selection-background-color: #f0f0f0;
        padding: 6px;
        font-size: 14px;
    }
)");



    QPushButton *btnRegistrar = new QPushButton("Registrar", panelRegistro);
    btnRegistrar->setStyleSheet("background-color: #800020; color: white; font-weight: bold; font-size: 15px; padding: 3px; border-radius: 12px;");

    connect(btnRegistrar, &QPushButton::clicked, [=]() {
        QString usuarioText = usuario->text().trimmed();
        QString nombreText = nombre->text().trimmed();
        QString correoText = correo->text().trimmed();
        QString passText = pass->text();
        QString passConfText = passConf->text();
        QString edadText = edad->text();
        QString respuestaText = respuestaSeguridad->text().trimmed();
        QString avatarText = avatarPathLabel->text().trimmed();
        QString preguntaText = preguntaSeguridad->currentText();

        // Validar campos vacíos
        if (usuarioText.isEmpty() || nombreText.isEmpty() || correoText.isEmpty() ||
            passText.isEmpty() || passConfText.isEmpty() || edadText.isEmpty() ||
            respuestaText.isEmpty() || avatarText == "Ningún archivo seleccionado")
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Campos Vacíos");
            msgBox.setText("🚨 Por favor completa todos los campos.");
            msgBox.setStyleSheet(R"(
    QMessageBox {
        background-color: #2e2e2e;
        color: white;
        font-size: 14px;
        border-radius: 12px;
    }
    QLabel {
        color: white;
        font-size: 14px;
    }
    QPushButton {
        background-color: #800020;
        color: white;
        border-radius: 8px;
        padding: 6px 14px;
        min-width: 80px;
    }
    QPushButton:hover {
        background-color: #a83232;
    }
)");
            msgBox.exec();
            return;
        }

        // Validar correo
        if (!correoText.contains("@") || !correoText.contains(".")) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Correo inválido");
            msgBox.setText("📧 El correo debe contener '@' y '.'");
            msgBox.setStyleSheet(R"(
    QMessageBox {
        background-color: #2e2e2e;
        color: white;
        font-size: 14px;
        border-radius: 12px;
    }
    QLabel {
        color: white;
        font-size: 14px;
    }
    QPushButton {
        background-color: #800020;
        color: white;
        border-radius: 8px;
        padding: 6px 14px;
        min-width: 80px;
    }
    QPushButton:hover {
        background-color: #a83232;
    }
)");
            msgBox.exec();
            return;
        }

        // Validar contraseñas coinciden
        if (passText != passConfText) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Contraseña no coincide");
            msgBox.setText("🔑 Las contraseñas no coinciden.");
            msgBox.setStyleSheet(R"(
    QMessageBox {
        background-color: #2e2e2e;
        color: white;
        font-size: 14px;
        border-radius: 12px;
    }
    QLabel {
        color: white;
        font-size: 14px;
    }
    QPushButton {
        background-color: #800020;
        color: white;
        border-radius: 8px;
        padding: 6px 14px;
        min-width: 80px;
    }
    QPushButton:hover {
        background-color: #a83232;
    }
)");
            msgBox.exec();
            return;
        }

        QRegularExpression regexSegura("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d).{6,}$");

        if (!regexSegura.match(passText).hasMatch()) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Contraseña insegura");
            msgBox.setText("🔐 La contraseña debe tener al menos:\n- Una letra mayúscula\n- Una letra minúscula\n- Un número\n- Mínimo 6 caracteres");
            msgBox.setStyleSheet(R"(
    QMessageBox {
        background-color: #2e2e2e;
        color: white;
        font-size: 14px;
        border-radius: 12px;
    }
    QLabel {
        color: white;
        font-size: 14px;
    }
    QPushButton {
        background-color: #800020;
        color: white;
        border-radius: 8px;
        padding: 6px 14px;
        min-width: 80px;
    }
    QPushButton:hover {
        background-color: #a83232;
    }
)");
            msgBox.exec();
            return;
        }
        // Validar edad
        bool ok;
        int edadInt = edadText.toInt(&ok);
        if (!ok || edadInt < 13) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Edad inválida");
            msgBox.setText("🎂 Debes tener al menos 12 años para registrarte.");
            msgBox.setStyleSheet(R"(
    QMessageBox {
        background-color: #2e2e2e;
        color: white;
        font-size: 14px;
        border-radius: 12px;
    }
    QLabel {
        color: white;
        font-size: 14px;
    }
    QPushButton {
        background-color: #800020;
        color: white;
        border-radius: 8px;
        padding: 6px 14px;
        min-width: 80px;
    }
    QPushButton:hover {
        background-color: #a83232;
    }
)");
            msgBox.exec();
            return;
        }

        // Validar que usuario no exista
        if (Usuario::existeUsuario(QDir::currentPath() + "/usuarios.txt", usuarioText)) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Usuario existente");
            msgBox.setText("⚠️ El nombre de usuario ya está en uso. Por favor elige otro.");
            msgBox.setStyleSheet(R"(
    QMessageBox {
        background-color: #2e2e2e;
        color: white;
        font-size: 14px;
        border-radius: 12px;
    }
    QLabel {
        color: white;
        font-size: 14px;
    }
    QPushButton {
        background-color: #800020;
        color: white;
        border-radius: 8px;
        padding: 6px 14px;
        min-width: 80px;
    }
    QPushButton:hover {
        background-color: #a83232;
    }
)");
            msgBox.exec();
            return;
        }

        // Crear y guardar usuario
        Usuario nuevoUsuario(
            usuarioText,
            nombreText,
            correoText,
            passText,
            avatarText,
            edadInt,
            preguntaText,
            respuestaText,
               false  // 👈 explícito
            );

        if (nuevoUsuario.guardarEnArchivo(QDir::currentPath() + "/usuarios.txt")) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setWindowTitle("Registro exitoso");
            msgBox.setText("✅ ¡Te has registrado exitosamente!");
            msgBox.setStyleSheet(R"(
    QMessageBox {
        background-color: #2e2e2e;
        color: white;
        font-size: 14px;
        border-radius: 12px;
    }
    QLabel {
        color: white;
        font-size: 14px;
    }
    QPushButton {
        background-color: #800020;
        color: white;
        border-radius: 8px;
        padding: 6px 14px;
        min-width: 80px;
    }
    QPushButton:hover {
        background-color: #a83232;
    }
)");
            msgBox.exec();

            // 👉 Abrir chat directamente con el nuevo usuario
            abrirChatConUsuario(nuevoUsuario);

        } else {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setWindowTitle("Error");
            msgBox.setText("❌ No se pudo guardar el usuario.");
            msgBox.setStyleSheet(R"(
    QMessageBox {
        background-color: #2e2e2e;
        color: white;
        font-size: 14px;
        border-radius: 12px;
    }
    QLabel {
        color: white;
        font-size: 14px;
    }
    QPushButton {
        background-color: #800020;
        color: white;
        border-radius: 8px;
        padding: 6px 14px;
        min-width: 80px;
    }
    QPushButton:hover {
        background-color: #a83232;
    }
)");
            msgBox.exec();
        }
    });

    QPushButton *btnCancelar = new QPushButton("Cancelar", panelRegistro);
    btnCancelar->setFlat(true);
    btnCancelar->setStyleSheet("color: #800020; text-decoration: underline; font-size: 13px;");
    connect(btnCancelar, &QPushButton::clicked, this, &LoginWindow::cerrarRegistro);

    QVBoxLayout *registroLayout = new QVBoxLayout(panelRegistro);
    registroLayout->setSpacing(20);
    registroLayout->setContentsMargins(30, 30, 30, 30);
    registroLayout->addWidget(tituloRegistro);
    registroLayout->addWidget(usuario);
    registroLayout->addWidget(nombre);
    registroLayout->addWidget(correo);
    registroLayout->addWidget(pass);
    registroLayout->addWidget(passConf);
    registroLayout->addWidget(edad);
    registroLayout->addLayout(avatarLayout);
    registroLayout->addWidget(avatarPathLabel);
    registroLayout->addWidget(preguntaSeguridad);
    registroLayout->addWidget(respuestaSeguridad);
    registroLayout->addWidget(btnRegistrar);
    registroLayout->addWidget(btnCancelar);
}


void LoginWindow::mostrarLogin() {
    loginFrame->show();
}

void LoginWindow::cerrarLogin() {
    loginFrame->hide();
}

void LoginWindow::mostrarRegistro() {
    loginFrame->hide();
    registroFrame->show();
}

void LoginWindow::cerrarRegistro() {
    registroFrame->hide();
    loginFrame->show();
}
void LoginWindow::abrirChatConUsuario(const Usuario &usuario)
{
    this->hide(); // Oculta ventana actual (registro o login)

    // 🔁 Actualizar estado a 1 en usuarios.txt
    QString rutaUsuarios = "/Users/anavalle/Desktop/chat/usuarios.txt";
    QFile file(rutaUsuarios);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QStringList lineas;
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString l = in.readLine();
            QStringList partes = l.split(",");
            if (partes.size() >= 9 && partes[0] == usuario.getUsuario()) {
                partes[8] = "1"; // Estado conectado
                lineas << partes.join(",");
            } else {
                lineas << l;
            }
        }
        file.close();

        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            for (const QString &l : lineas)
                out << l << "\n";
            file.close();
        }
    }

    // Conexión del socket y apertura del chat
    ClienteSocket* socket = new ClienteSocket(this);
    socket->setNombreUsuario(usuario.getUsuario());
    socket->conectarServidor("127.0.0.1", 12345);

    chatWindow = new ChatWindow(usuario, socket);
    chatWindow->getChatScreen()->setUsuarioActual(usuario.getUsuario());
    chatWindow->getChatScreen()->setClienteSocket(socket);
    chatWindow->show();
}
