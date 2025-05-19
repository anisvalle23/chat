#include "chatwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QFile>
#include <QDebug>
#include <QSpacerItem>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QTextStream>
#include "agregarcontactodialog.h"
#include <QFileDialog>
#include <QCheckBox>
#include "loginwindow.h"
#include "gestorcontactos.h"
#include "chatscreen.h"
#include <QProcess>
#include "PasswordLineEdit.h"
#include "cliente_socket.h"
#include <QTimer>
#include "chatwindow.h"
#include "gestorvistachat.h"
#include "gestornoleidos.h"
#include "gestornotificaciones.h"

ChatWindow::ChatWindow(const Usuario &usuarioActivo, ClienteSocket* socket, QWidget *parent)
    : QWidget(parent), usuario(usuarioActivo), clienteSocket(socket)
{
    setFixedSize(800, 650);
    setStyleSheet("background-color: #f5f5f5; color: #333;");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ✅ Conecta la señal de actualización de estado de usuarios
    connect(clienteSocket, &ClienteSocket::estadoUsuariosActualizado,
            this, &ChatWindow::actualizarEstadosEnLista);

    // --- PANEL LATERAL ---
    QWidget *sidePanel = new QWidget(this);
    sidePanel->setFixedWidth(70);
    sidePanel->setStyleSheet("background-color: #800020;");

    QVBoxLayout *sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(10, 20, 10, 20);
    sideLayout->setSpacing(20);

    QLabel *avatarLabel = new QLabel(sidePanel);
    avatarLabel->setFixedSize(48, 48);
    avatarLabel->setStyleSheet("border-radius: 24px; background-color: white;");

    QPixmap avatarPixmap(usuario.getAvatar());
    if (!avatarPixmap.isNull()) {
        QPixmap rounded(48, 48);
        rounded.fill(Qt::transparent);
        QPainter painter(&rounded);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addEllipse(0, 0, 48, 48);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, avatarPixmap.scaled(48, 48, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        avatarLabel->setPixmap(rounded);
    }

    sideLayout->addWidget(avatarLabel, 0, Qt::AlignHCenter);
    sideLayout->addSpacing(20);

    QPushButton *chatButton = new QPushButton("\xF0\x9F\x92\xAC", sidePanel);
    chatButton->setFixedSize(48, 48);
    chatButton->setCheckable(true);
    chatButton->setChecked(true);
    chatButton->setStyleSheet("QPushButton { font-size: 24px; color: white; background-color: transparent; border: none; } QPushButton:checked { background-color: rgba(255,255,255,0.3); border-radius: 24px; } QPushButton:hover { background-color: rgba(255,255,255,0.2); border-radius: 24px; }");
    sideLayout->addWidget(chatButton, 0, Qt::AlignHCenter);

    solicitudesButton = new QPushButton("🔔", sidePanel);
    solicitudesButton->setFixedSize(48, 48);
    solicitudesButton->setCheckable(true);
    solicitudesButton->setStyleSheet(chatButton->styleSheet());
    sideLayout->addWidget(solicitudesButton, 0, Qt::AlignHCenter);

    sideLayout->addStretch();

    QPushButton *settingsButton = new QPushButton("\xE2\x9A\x99", sidePanel);
    settingsButton->setFixedSize(48, 48);
    settingsButton->setCheckable(true);
    settingsButton->setStyleSheet(chatButton->styleSheet());
    sideLayout->addWidget(settingsButton, 0, Qt::AlignHCenter);

    // STACK DE CONTENIDO
    contentStack = new QStackedWidget(this);

    chatScreen = new ChatScreen(this);
    contentStack->addWidget(chatScreen);

    // Pantalla de Chat
    QWidget *chatHomeScreen = new QWidget(this);
    QHBoxLayout *contentLayout = new QHBoxLayout(chatHomeScreen);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    QWidget *leftPanel = new QWidget(chatHomeScreen);
    leftPanel->setFixedWidth(280);
    leftPanel->setStyleSheet("background-color: white; border-right: 1px solid #ddd;");

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(10, 10, 10, 10);
    leftLayout->setSpacing(10);

    QHBoxLayout *topLeftLayout = new QHBoxLayout();
    QLabel *chatsLabel = new QLabel("Chats", leftPanel);
    chatsLabel->setStyleSheet("font-weight: bold; font-size: 18px;");

    QPushButton *addButton = new QPushButton("\xE2\x9E\x95", leftPanel);
    QPushButton *removeButton = new QPushButton("\xE2\x9E\x96", leftPanel);
    addButton->setFixedSize(24, 24);
    removeButton->setFixedSize(24, 24);
    addButton->setStyleSheet("border: none; background-color: transparent; font-size: 16px;");
    removeButton->setStyleSheet("border: none; background-color: transparent; font-size: 16px;");

    topLeftLayout->addWidget(chatsLabel);
    topLeftLayout->addStretch();
    topLeftLayout->addWidget(addButton);
    topLeftLayout->addWidget(removeButton);

    QComboBox *ordenCombo = new QComboBox(leftPanel);
    ordenCombo->addItem("Orden normal");                // index 0 → normal
    ordenCombo->addItem("Alfabético");                  // index 1 → alfabetico
    ordenCombo->addItem("Longitud del chat");           // index 2 → longitud
    ordenCombo->addItem("Más reciente (mensaje)");      // index 3 → fecha
    ordenCombo->addItem("Más reciente (agregado)");     // index 4 → reciente_agregado

    ordenCombo->setStyleSheet("padding: 6px; border: 1px solid #ccc; border-radius: 6px; font-size: 13px;");
    leftLayout->addWidget(ordenCombo);

    connect(ordenCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        QString criterio;

        switch (index) {
        case 1: criterio = "alfabetico"; break;
        case 2: criterio = "longitud"; break;
        case 3: criterio = "fecha"; break;
        case 4: criterio = "reciente_agregado"; break;
        default: criterio = "normal"; break;
        }

        cargarContactosOrdenados(criterio);
    });

    contactList = new QListWidget(leftPanel);
    contactList->setStyleSheet(R"(
    QListWidget {
        border: none;
        padding: 5px;
        background-color: white;
    }
    QListWidget::item {
        border: none;
        margin: 0px 0;
        padding: 0px;
    }
    QListWidget::item:selected {
        background-color: #f0f0f0;
        border-radius: 8px;
    }
)");

    leftLayout->addLayout(topLeftLayout);
    // leftLayout->addWidget(searchField);

    solicitudesLabel = new QLabel("Solicitudes de contacto:", leftPanel);
    solicitudesLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    solicitudesLabel->setVisible(false);

    solicitudesList = new QListWidget(leftPanel);
    solicitudesList->setStyleSheet(contactList->styleSheet());
    solicitudesList->setVisible(false);

    leftLayout->addWidget(solicitudesLabel);
    leftLayout->addWidget(solicitudesList);
    leftLayout->addWidget(contactList);

    // ✅ Marcar usuario como conectado en el archivo usuarios.txt
    QString rutaUsuarios = "/Users/anavalle/Desktop/chat/usuarios.txt";
    QFile file(rutaUsuarios);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QStringList lineas;
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString linea = in.readLine();
            QStringList partes = linea.split(",");

            if (partes.size() == 8 && partes[0].trimmed() == usuario.getUsuario().trimmed()) {
                partes << "1";  // Agregar campo estado si falta
                lineas << partes.join(",");
            } else if (partes.size() >= 9 && partes[0].trimmed() == usuario.getUsuario().trimmed()) {
                partes[8] = "1";  // Actualizar estado a conectado
                lineas << partes.join(",");
            } else {
                lineas << linea;
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

    // Panel derecho (para chat o bienvenida)
    QWidget *rightPanel = new QWidget(chatHomeScreen);
    QVBoxLayout *rightSideLayout = new QVBoxLayout(rightPanel);
    rightSideLayout->setContentsMargins(0, 0, 0, 0);
    rightSideLayout->setSpacing(0);

    // Widget de bienvenida (se podrá ocultar cuando se abra un chat)
    welcomeWidget = new QWidget(rightPanel);  // Declarado en chatwindow.h
    QVBoxLayout *centerLayout = new QVBoxLayout(welcomeWidget);
    centerLayout->setAlignment(Qt::AlignCenter);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(20);

    // Avatar grande
    QWidget *avatarContainer = new QWidget(welcomeWidget);
    QVBoxLayout *avatarLayout = new QVBoxLayout(avatarContainer);
    avatarLayout->setContentsMargins(0, 0, 0, 0);
    avatarLayout->setAlignment(Qt::AlignCenter);

    // Imagen del avatar central (logoHome)
    logoHome = new QLabel(avatarContainer);  // Declarado en chatwindow.h
    logoHome->setFixedSize(150, 150);

    if (!avatarPixmap.isNull()) {
        QPixmap avatarScaled = avatarPixmap.scaled(150, 150, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPixmap roundedAvatar(150, 150);
        roundedAvatar.fill(Qt::transparent);
        QPainter painter(&roundedAvatar);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addEllipse(0, 0, 150, 150);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, avatarScaled);
        logoHome->setPixmap(roundedAvatar);
    }

    avatarLayout->addWidget(logoHome);
    avatarContainer->setLayout(avatarLayout);

    // Texto de bienvenida
    welcomeText = new QLabel("Bienvenido, " + usuario.getNombre(), welcomeWidget);  // Declarado en chatwindow.h
    welcomeText->setAlignment(Qt::AlignCenter);
    welcomeText->setStyleSheet("font-size: 22px; color: #555; font-weight: bold;");

    // Añadir elementos al layout del centro
    centerLayout->addWidget(avatarContainer);
    centerLayout->addWidget(welcomeText);

    // Añadir la pantalla de bienvenida al panel derecho
    rightSideLayout->addStretch();
    rightSideLayout->addWidget(welcomeWidget);
    rightSideLayout->addStretch();

    // Agregar ambos paneles al layout principal
    contentLayout->addWidget(leftPanel);
    contentLayout->addWidget(rightPanel);

    // CONFIGURACIÓN COMPLETA
    QWidget *settingsScreen = new QWidget(this);
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsScreen);
    settingsLayout->setAlignment(Qt::AlignTop);
    settingsLayout->setContentsMargins(40, 40, 40, 40);

    QLabel *title = new QLabel("Configuración", settingsScreen);
    title->setStyleSheet("font-size: 26px; font-weight: bold; margin-bottom: 30px;");
    title->setAlignment(Qt::AlignCenter);
    settingsLayout->addWidget(title);

    QPushButton *cambiarAvatarBtn = new QPushButton("Cambiar avatar");
    QPushButton *editarNombreBtn = new QPushButton("Editar nombre");
    QPushButton *cambiarContrasenaBtn = new QPushButton("Cambiar contraseña");
    QPushButton *cerrarSesionBtn = new QPushButton("Cerrar sesión");
    QPushButton *eliminarCuentaBtn = new QPushButton("Eliminar mi cuenta");
    QList<QPushButton*> botones = { cambiarAvatarBtn, editarNombreBtn, cambiarContrasenaBtn, cerrarSesionBtn,eliminarCuentaBtn  };
    for (auto btn : botones) {
        btn->setFixedHeight(40);
        btn->setStyleSheet("padding: 8px; border-radius: 8px; background-color: #eee; font-size: 16px;");
    }

    QStackedWidget *configStack = new QStackedWidget(settingsScreen);

    QWidget *menuWidget = new QWidget();
    QVBoxLayout *menuLayout = new QVBoxLayout(menuWidget);
    menuLayout->setAlignment(Qt::AlignCenter);
    menuLayout->setSpacing(15);

    // Agrega todos los botones anteriores
    for (auto btn : botones)
        menuLayout->addWidget(btn);

    // Espacio y botón "Iniciar otra sesión"
    menuLayout->addSpacing(20);

    QPushButton *multiSesionBtn = new QPushButton("\u2795 Iniciar otra sesión");
    multiSesionBtn->setStyleSheet("color: #800020; font-weight: bold; background: none; border: none; font-size: 14px;");
    menuLayout->addWidget(multiSesionBtn, 0, Qt::AlignCenter);

    menuWidget->setLayout(menuLayout);

    // 👉 Conexión del botón para abrir una nueva instancia de la app
    connect(multiSesionBtn, &QPushButton::clicked, this, []() {
        QString ruta = "/Users/anavalle/Desktop/chat/build/Desktop_arm_darwin_generic_mach_o_32bit-Debug/chat.app/Contents/MacOS/chat";
        if (!QProcess::startDetached(ruta)) {
            QMessageBox::critical(nullptr, "Error", "No se pudo abrir una nueva sesión. Verifica que el ejecutable exista y tenga permisos.");
        }
    });
    // Utilidad para crear vistas con "Volver"
    auto crearVistaConVolver = [&](const QString &titulo, QWidget *contenido, int regresarA = 0) {
        QWidget *w = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(w);
        layout->setAlignment(Qt::AlignTop);
        layout->setContentsMargins(40, 40, 40, 40);
        layout->setSpacing(20);

        QLabel *label = new QLabel(titulo, w);
        label->setStyleSheet("font-size: 22px; font-weight: bold;");
        label->setAlignment(Qt::AlignCenter);

        QPushButton *btnVolver = new QPushButton("\u2190 Volver", w);
        btnVolver->setFixedWidth(100);
        btnVolver->setStyleSheet("padding: 6px; border-radius: 6px; background-color: #ddd; font-size: 14px;");

        layout->addWidget(label);
        layout->addSpacing(10);
        layout->addWidget(contenido);
        layout->addSpacing(20);
        layout->addWidget(btnVolver, 0, Qt::AlignLeft);

        connect(btnVolver, &QPushButton::clicked, this, [=]() {
            configStack->setCurrentIndex(regresarA);
        });
        return w;
    };

    // Vista: Cambiar Avatar
    QWidget *avatarForm = new QWidget();
    QVBoxLayout *avatarFormLayout = new QVBoxLayout(avatarForm);
    QLineEdit *avatarInput = new QLineEdit();
    avatarInput->setPlaceholderText("Ruta del nuevo avatar o URL");
    avatarInput->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 8px;");
    QPushButton *btnAvatarArchivo = new QPushButton("Seleccionar archivo");
    btnAvatarArchivo->setStyleSheet("padding: 8px; background-color: #ccc; border-radius: 8px;");
    QPushButton *btnAvatarGuardar = new QPushButton("Guardar");
    btnAvatarGuardar->setStyleSheet("padding: 8px; background-color: #800020; color: white; border-radius: 8px;");

    connect(btnAvatarGuardar, &QPushButton::clicked, this, [=]() {
        QString nuevoAvatar = avatarInput->text().trimmed();

        if (nuevoAvatar.isEmpty()) {
            QMessageBox::warning(this, "Campo vacío", "Por favor ingresa la ruta o selecciona un archivo.");
            return;
        }

        QFile archivo(nuevoAvatar);
        if (!archivo.exists()) {
            QMessageBox::warning(this, "Archivo no encontrado", "La ruta especificada no existe.");
            return;
        }

        // Cambiar en memoria
        usuario.setAvatar(nuevoAvatar);

        // Cambiar visualmente (en el panel lateral y pantalla de bienvenida)
        QPixmap nuevoPixmap(nuevoAvatar);
        if (!nuevoPixmap.isNull()) {
            QPixmap redondo(48, 48);
            redondo.fill(Qt::transparent);
            QPainter p1(&redondo);
            p1.setRenderHint(QPainter::Antialiasing);
            QPainterPath path1;
            path1.addEllipse(0, 0, 48, 48);
            p1.setClipPath(path1);
            p1.drawPixmap(0, 0, nuevoPixmap.scaled(48, 48, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            avatarLabel->setPixmap(redondo);

            QPixmap redondoGrande(150, 150);
            redondoGrande.fill(Qt::transparent);
            QPainter p2(&redondoGrande);
            p2.setRenderHint(QPainter::Antialiasing);
            QPainterPath path2;
            path2.addEllipse(0, 0, 150, 150);
            p2.setClipPath(path2);
            p2.drawPixmap(0, 0, nuevoPixmap.scaled(150, 150, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            logoHome->setPixmap(redondoGrande);
        }

        // Actualizar en usuarios.txt
        for (Usuario &u : usuariosTotales) {
            if (u.getUsuario() == usuario.getUsuario()) {
                u.setAvatar(nuevoAvatar);
                break;
            }
        }

        QFile file("/Users/anavalle/Desktop/chat/usuarios.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            for (const Usuario &u : usuariosTotales) {
                out << u.getUsuario() << ","
                    << u.getNombre() << ","
                    << u.getCorreo() << ","
                    << u.getContrasena() << ","
                    << u.getAvatar() << ","
                    << u.getEdad() << ","
                    << u.getPregunta() << ","
                    << u.getRespuesta() << ","
                    << (u.getEstado() ? "1" : "0") << "\n";  // 👈 Aquí agregas el estado
            }
            file.close();
        }
        QMessageBox::information(this, "Éxito", "Tu avatar ha sido actualizado.");
    });

    avatarFormLayout->addWidget(avatarInput);
    avatarFormLayout->addWidget(btnAvatarArchivo);
    avatarFormLayout->addWidget(btnAvatarGuardar);

    connect(btnAvatarArchivo, &QPushButton::clicked, this, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Seleccionar imagen de avatar", "", "Imágenes (*.png *.jpg *.jpeg)");
        if (!fileName.isEmpty()) avatarInput->setText(fileName);
    });

    QWidget *vistaAvatar = crearVistaConVolver("Cambiar avatar", avatarForm);

    // ----- FORMULARIO PARA EDITAR USUARIO Y NOMBRE COMPLETO -----
    QWidget *nombreForm = new QWidget();
    QVBoxLayout *nombreLayout = new QVBoxLayout(nombreForm);
    nombreLayout->setSpacing(10);
    nombreLayout->setContentsMargins(20, 20, 20, 20);

    // Título
    QLabel *tituloEditar = new QLabel("Editar usuario y nombre completo");
    tituloEditar->setAlignment(Qt::AlignCenter);
    tituloEditar->setStyleSheet("font-size: 18px; font-weight: bold;");
    nombreLayout->addWidget(tituloEditar);

    // Usuario
    QLabel *labelUsuario = new QLabel("Usuario actual:");
    QLineEdit *usuarioNuevo = new QLineEdit();
    usuarioNuevo->setPlaceholderText(usuario.getUsuario());
    usuarioNuevo->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 8px;");

    // Nombre
    QLabel *labelNombre = new QLabel("Nombre completo actual:");
    QLineEdit *nombreNuevo = new QLineEdit();
    nombreNuevo->setPlaceholderText(usuario.getNombre());
    nombreNuevo->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 8px;");

    // Botón guardar
    QPushButton *btnGuardarNombre = new QPushButton("Guardar");
    btnGuardarNombre->setStyleSheet("padding: 10px; background-color: #800020; color: white; border-radius: 8px;");
    btnGuardarNombre->setFixedHeight(40);

    // Agregar al layout
    nombreLayout->addWidget(labelUsuario);
    nombreLayout->addWidget(usuarioNuevo);
    nombreLayout->addWidget(labelNombre);
    nombreLayout->addWidget(nombreNuevo);
    nombreLayout->addSpacing(10);
    nombreLayout->addWidget(btnGuardarNombre);

    // Envolver en vista con botón "Volver"
    QWidget *vistaNombre = crearVistaConVolver("Configuración", nombreForm);

    // Lógica para guardar cambios
    connect(btnGuardarNombre, &QPushButton::clicked, this, [=]() {
        QString nuevoUsuario = usuarioNuevo->text().trimmed();
        QString nuevoNombre = nombreNuevo->text().trimmed();
        QString anteriorUsuario = usuario.getUsuario();

        bool cambioUsuario = !nuevoUsuario.isEmpty() && nuevoUsuario != usuario.getUsuario();
        bool cambioNombre = !nuevoNombre.isEmpty() && nuevoNombre != usuario.getNombre();

        if (!cambioUsuario && !cambioNombre) {
            QMessageBox::information(this, "Sin cambios", "No se realizaron modificaciones.");
            return;
        }

        // Cambios en objeto en memoria
        if (cambioUsuario) usuario.setUsuario(nuevoUsuario);
        if (cambioNombre)  usuario.setNombre(nuevoNombre);

        // Cambiar texto de bienvenida
        if (cambioNombre) welcomeText->setText("Bienvenido, " + usuario.getNombre());

        // Actualizar contacto en lista si coincide el usuario anterior
        for (int i = 0; i < contactList->count(); ++i) {
            QListWidgetItem *item = contactList->item(i);
            if (item->text() == anteriorUsuario && cambioUsuario) {
                item->setText(usuario.getUsuario());
                break;
            }
        }

        // Actualizar archivo
        QFile file("/Users/anavalle/Desktop/chat/usuarios.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for (Usuario &u : usuariosTotales) {
                if (u.getUsuario() == anteriorUsuario) {
                    if (cambioUsuario) u.setUsuario(nuevoUsuario);
                    if (cambioNombre)  u.setNombre(nuevoNombre);
                }
                out << u.getUsuario() << "," << u.getNombre() << "," << u.getCorreo() << ","
                    << u.getContrasena() << "," << u.getAvatar() << "," << u.getEdad() << ","
                    << u.getPregunta() << "," << u.getRespuesta() << ","
                    << (u.getEstado() ? "1" : "0") << "\n";  // 👈 aquí agregas el estado
            }
            file.close();
            QMessageBox::information(this, "Actualizado", "Datos guardados correctamente.");
        } else {
            QMessageBox::critical(this, "Error", "No se pudo escribir en el archivo.");
        }
    });

    // Vista: Cambiar contraseña
    QWidget *passwordForm = new QWidget();
    QVBoxLayout *passLayout = new QVBoxLayout(passwordForm);

    // Campos de contraseña visibles al pasar cursor
    PasswordLineEdit *actual = new PasswordLineEdit();
    actual->setPlaceholderText("Contraseña actual");

    PasswordLineEdit *nueva = new PasswordLineEdit();
    nueva->setPlaceholderText("Nueva contraseña");

    PasswordLineEdit *confirmar = new PasswordLineEdit();
    confirmar->setPlaceholderText("Confirmar nueva contraseña");

    // Estilo común
    for (auto *line : {actual, nueva, confirmar})
        line->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 8px;");

    // Mostrar solo la pregunta registrada
    QLabel *preguntaLabel = new QLabel("Pregunta de seguridad:");
    preguntaLabel->setStyleSheet("font-weight: bold;");

    QLabel *preguntaMostrada = new QLabel(usuario.getPregunta());  // ✅ Aquí usas la que tiene guardada
    preguntaMostrada->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 8px; background-color: #f0f0f0;");

    // Campo para responder
    QLineEdit *respuestaSeguridad = new QLineEdit();
    respuestaSeguridad->setPlaceholderText("Respuesta de seguridad");
    respuestaSeguridad->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 8px;");

    // Botón
    QPushButton *btnCambiar = new QPushButton("Cambiar");
    btnCambiar->setStyleSheet("padding: 8px; background-color: #800020; color: white; border-radius: 8px;");

    // Añadir widgets al layout
    passLayout->addWidget(actual);
    passLayout->addWidget(nueva);
    passLayout->addWidget(confirmar);
    passLayout->addWidget(preguntaLabel);
    passLayout->addWidget(preguntaMostrada);
    passLayout->addWidget(respuestaSeguridad);
    passLayout->addWidget(btnCambiar);

    // Lógica al hacer clic
    connect(btnCambiar, &QPushButton::clicked, this, [=]() {
        QString actualPass = actual->text().trimmed();
        QString nuevaPass = nueva->text().trimmed();
        QString confirmarPass = confirmar->text().trimmed();
        QString respuesta = respuestaSeguridad->text().trimmed();

        QRegularExpression regexSegura("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d).{6,}$");

        if (actualPass.isEmpty() || nuevaPass.isEmpty() || confirmarPass.isEmpty() || respuesta.isEmpty()) {
            QMessageBox::warning(this, "Campos vacíos", "Completa todos los campos.");
            return;
        }

        if (actualPass != usuario.getContrasena()) {
            QMessageBox::warning(this, "Contraseña incorrecta", "La contraseña actual no coincide.");
            return;
        }

        if (respuesta != usuario.getRespuesta()) {
            QMessageBox::warning(this, "Error de seguridad", "La respuesta de seguridad no coincide.");
            return;
        }

        if (!regexSegura.match(nuevaPass).hasMatch()) {
            QMessageBox::warning(this, "Contraseña insegura",
                                 "La nueva contraseña debe tener al menos:\n- Una letra mayúscula\n- Una minúscula\n- Un número\n- Mínimo 6 caracteres.");
            return;
        }

        if (nuevaPass != confirmarPass) {
            QMessageBox::warning(this, "Confirmación incorrecta", "Las contraseñas no coinciden.");
            return;
        }

        if (nuevaPass == actualPass) {
            QMessageBox::information(this, "Sin cambios", "La nueva contraseña es igual a la actual.");
            return;
        }

        // Actualizar contraseña en objeto y archivo
        usuario.setContrasena(nuevaPass);
        QFile file("/Users/anavalle/Desktop/chat/usuarios.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for (Usuario &u : usuariosTotales) {
                if (u.getUsuario() == usuario.getUsuario()) {
                    u.setContrasena(nuevaPass);
                }
                out << u.getUsuario() << "," << u.getNombre() << "," << u.getCorreo() << ","
                    << u.getContrasena() << "," << u.getAvatar() << "," << u.getEdad() << ","
                    << u.getPregunta() << "," << u.getRespuesta() << ","
                    << (u.getEstado() ? "1" : "0") << "\n";  // 👈 aquí agregas el estado
            }
            file.close();
            QMessageBox::information(this, "Contraseña cambiada", "Tu contraseña ha sido actualizada correctamente.");
        } else {
            QMessageBox::warning(this, "Error", "No se pudo guardar la nueva contraseña.");
        }
    });

    QWidget *vistaContrasena = crearVistaConVolver("Cambiar contraseña", passwordForm);

    // Vista: Cerrar sesión
    QWidget *cerrarForm = new QWidget();
    QVBoxLayout *cerrarLayout = new QVBoxLayout(cerrarForm);
    cerrarLayout->setSpacing(15);
    cerrarLayout->setAlignment(Qt::AlignCenter);

    QLabel *confirmacion = new QLabel("¿Estás seguro que deseas cerrar sesión?");
    confirmacion->setStyleSheet("font-size: 16px; color: #444;");
    confirmacion->setAlignment(Qt::AlignCenter);

    QPushButton *btnCerrarConfirmar = new QPushButton("Sí, cerrar sesión");
    btnCerrarConfirmar->setStyleSheet("padding: 10px; background-color: darkred; color: white; border-radius: 8px; font-weight: bold;");

    cerrarLayout->addWidget(confirmacion);
    cerrarLayout->addWidget(btnCerrarConfirmar, 0, Qt::AlignCenter);

    connect(btnCerrarConfirmar, &QPushButton::clicked, this, [=]() {
        if (clienteSocket) {
            clienteSocket->enviarMensaje("DESCONECTADO:" + usuario.getUsuario());
            clienteSocket->desconectar();  // ✅ Usamos método propio
        }

        // Actualizar archivo usuarios.txt → estado = 0
        QString archivoRuta = "/Users/anavalle/Desktop/chat/usuarios.txt";
        QFile file(archivoRuta);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QStringList lineas;
            QTextStream in(&file);

            while (!in.atEnd()) {
                QString linea = in.readLine().trimmed();
                QStringList partes = linea.split(",");

                if (partes.size() >= 9 && partes[0].trimmed() == usuario.getUsuario()) {
                    partes[8] = "0";  // Desconectado
                    linea = partes.join(",");
                }

                lineas << linea;
            }
            file.close();

            if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                QTextStream out(&file);
                for (const QString &l : lineas)
                    out << l << "\n";
                file.close();
            }
        }

        QMessageBox::information(this, "Sesión cerrada", "Has cerrado sesión.");
        loginWindow = new LoginWindow();
        loginWindow->show();
        this->close();
    });

    QWidget *vistaCerrarSesion = crearVistaConVolver("Cerrar sesión", cerrarForm);

    // Vista: Eliminar cuenta
    QWidget *eliminarForm = new QWidget();
    QVBoxLayout *eliminarFormLayout = new QVBoxLayout(eliminarForm);
    eliminarFormLayout->setSpacing(10);
    eliminarFormLayout->setContentsMargins(20, 20, 20, 20);

    // Título
    QLabel *tituloEliminar = new QLabel(" ");
    tituloEliminar->setAlignment(Qt::AlignCenter);
    tituloEliminar->setStyleSheet("font-size: 18px; font-weight: bold;");
    eliminarFormLayout->addWidget(tituloEliminar);

    // Contraseña
    QLineEdit *inputContrasena = new QLineEdit();
    inputContrasena->setPlaceholderText("Contraseña actual");
    inputContrasena->setEchoMode(QLineEdit::Password);
    inputContrasena->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 8px;");
    eliminarFormLayout->addWidget(inputContrasena);

    // Pregunta de seguridad
    QLabel *labelPregunta = new QLabel("Pregunta de seguridad:");
    labelPregunta->setStyleSheet("font-weight: bold;");
    eliminarFormLayout->addWidget(labelPregunta);

    QLabel *mostrarPregunta = new QLabel(usuario.getPregunta());
    mostrarPregunta->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 8px; background-color: #f0f0f0;");
    eliminarFormLayout->addWidget(mostrarPregunta);

    // Respuesta
    QLineEdit *inputRespuesta = new QLineEdit();
    inputRespuesta->setPlaceholderText("Respuesta de seguridad");
    inputRespuesta->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 8px;");
    eliminarFormLayout->addWidget(inputRespuesta);

    // Botón de confirmación
    QPushButton *btnConfirmarEliminar = new QPushButton("Eliminar definitivamente");
    btnConfirmarEliminar->setStyleSheet("padding: 10px; background-color: darkred; color: white; border-radius: 8px; font-weight: bold;");
    eliminarFormLayout->addWidget(btnConfirmarEliminar);

    // Conexión lógica
    connect(btnConfirmarEliminar, &QPushButton::clicked, this, [=]() {
        QString contrasenaIngresada = inputContrasena->text().trimmed();
        QString respuestaIngresada = inputRespuesta->text().trimmed().toLower();
        QString contrasenaCorrecta = usuario.getContrasena();
        QString respuestaCorrecta = usuario.getRespuesta().trimmed().toLower();

        if (contrasenaIngresada != contrasenaCorrecta) {
            QMessageBox::warning(this, "Error", "La contraseña es incorrecta.");
            return;
        }

        if (respuestaIngresada != respuestaCorrecta) {
            QMessageBox::warning(this, "Error", "La respuesta de seguridad es incorrecta.");
            return;
        }

        eliminarCuenta(usuario.getUsuario());
    });

    // ESTA es la línea que **crea** vistaEliminarCuenta
    QWidget *vistaEliminarCuenta = crearVistaConVolver("Eliminar cuenta", eliminarForm);
    connect(eliminarCuentaBtn, &QPushButton::clicked, this, [=]() { configStack->setCurrentIndex(5); });

    configStack->addWidget(menuWidget);
    configStack->addWidget(vistaAvatar);
    configStack->addWidget(vistaNombre);
    configStack->addWidget(vistaContrasena);
    configStack->addWidget(vistaCerrarSesion);
    configStack->addWidget(vistaEliminarCuenta);
    settingsLayout->addWidget(configStack);

    connect(cambiarAvatarBtn, &QPushButton::clicked, this, [=]() { configStack->setCurrentIndex(1); });
    connect(editarNombreBtn, &QPushButton::clicked, this, [=]() { configStack->setCurrentIndex(2); });
    connect(cambiarContrasenaBtn, &QPushButton::clicked, this, [=]() { configStack->setCurrentIndex(3); });
    connect(cerrarSesionBtn, &QPushButton::clicked, this, [=]() {
        if (clienteSocket) {
            clienteSocket->enviarMensaje("DESCONECTADO:" + usuario.getUsuario());
            clienteSocket->desconectar();
        }

        // Actualizar archivo usuarios.txt → estado = 0
        QString archivoRuta = "/Users/anavalle/Desktop/chat/usuarios.txt";
        QFile file(archivoRuta);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QStringList lineas;
            QTextStream in(&file);

            while (!in.atEnd()) {
                QString linea = in.readLine().trimmed();
                QStringList partes = linea.split(",");

                if (partes.size() >= 9 && partes[0].trimmed() == usuario.getUsuario()) {
                    partes[8] = "0";  // Desconectado
                    linea = partes.join(",");
                }

                lineas << linea;
            }
            file.close();

            if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                QTextStream out(&file);
                for (const QString &l : lineas)
                    out << l << "\n";
                file.close();
            }
        }

        LoginWindow *login = new LoginWindow();
        login->show();
        this->close();
    });

    contentStack->addWidget(chatHomeScreen);
    contentStack->addWidget(settingsScreen);
    contentStack->addWidget(chatScreen);

    mainLayout->addWidget(sidePanel);
    mainLayout->addWidget(contentStack);

    cargarUsuariosDesdeArchivo();
    filtrarUsuariosDisponibles();
    cargarContactosGuardados();       // 🟢 primero cargar desde archivo
    cargarSolicitudesPendientes();   // 🟡 luego cargar solicitudes
    guardarContactosActuales();       // 🔴 al final, guardar si hay cambios

    contentStack->setCurrentIndex(0);
    estadoTimer = new QTimer(this);
    connect(estadoTimer, &QTimer::timeout, this, &ChatWindow::verificarEstadosDesdeArchivo);
    estadoTimer->start(1000);  // cada 1 segundo

    solicitudesTimer = new QTimer(this);
    connect(solicitudesTimer, &QTimer::timeout, this, &ChatWindow::verificarSolicitudesDesdeArchivo);
    solicitudesTimer->start(1000);  // cada segundo

    contactosTimer = new QTimer(this);
    connect(contactosTimer, &QTimer::timeout, this, &ChatWindow::cargarContactosGuardados);
    contactosTimer->start(1500);  // cada 1.5 segundos

    solicitudesListTimer = new QTimer(this);
    connect(solicitudesListTimer, &QTimer::timeout, this, [=]() {
        int pendientes = GestorVistaChat::contarSolicitudes(usuario.getUsuario(), contactList);
        solicitudesButton->setText(pendientes > 0 ? QString("🔔 %1").arg(pendientes) : "🔔");
    });

    actualizarChatsTimer = new QTimer(this);
    connect(actualizarChatsTimer, &QTimer::timeout, this, &ChatWindow::verificarActualizacionArchivo);
    actualizarChatsTimer->start(1000);

    mensajesNoLeidosTimer = new QTimer(this);
    connect(mensajesNoLeidosTimer, &QTimer::timeout, this, &ChatWindow::verificarMensajesNoLeidos);
    mensajesNoLeidosTimer->start(1000); // Cada segundo

    notificacionesTimer = new QTimer(this);
    connect(notificacionesTimer, &QTimer::timeout, this, &ChatWindow::verificarNotificaciones);
    notificacionesTimer->start(1000);

    // Mostrar solo la vista de chats al inicio
    contactList->setVisible(true);
    solicitudesList->setVisible(false);
    solicitudesLabel->setVisible(false);
    chatButton->setChecked(true);
    solicitudesButton->setChecked(false);

    connect(chatButton, &QPushButton::clicked, this, [=]() {
        mostrarPantallaBienvenida();                      // Muestra el avatar y texto
        contactList->setVisible(true);                    // Muestra contactos
        solicitudesList->setVisible(false);               // Oculta solicitudes
        solicitudesLabel->setVisible(false);
        contentStack->setCurrentIndex(0);                 // Bienvenida/chat
    });

    connect(settingsButton, &QPushButton::clicked, this, [=]() {
        contentStack->setCurrentIndex(1);                 // Configuración
        contactList->setVisible(false);
    });

    connect(addButton, &QPushButton::clicked, this, [=]() {
        AgregarContactoDialog dialog(QList<Usuario>::fromVector(usuariosDisponibles), this);
        if (dialog.exec() == QDialog::Accepted) {
            QString seleccionado = dialog.getSeleccionado();
            if (!seleccionado.isEmpty()) {
                // Verifica si ya está en la lista visual
                for (int i = 0; i < contactList->count(); ++i) {
                    QListWidgetItem *item = contactList->item(i);
                    if (item && item->data(Qt::UserRole).toString() == seleccionado) {
                        QMessageBox::information(this, "Contacto existente", "Ya tienes agregado ese contacto.");
                        return;
                    }
                }

                // Enviar solicitud si no ha sido enviada antes
                for (const Usuario &u : usuariosTotales) {
                    if (u.getUsuario() == seleccionado) {
                        QString archivoSolicitud = "/Users/anavalle/Desktop/chat/solicitudes/solicitudes_" + u.getUsuario() + ".txt";

                        bool yaEnviada = false;
                        QFile archivo(archivoSolicitud);
                        if (archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
                            QTextStream in(&archivo);
                            while (!in.atEnd()) {
                                QString linea = in.readLine().trimmed();
                                if (linea.startsWith(usuario.getUsuario() + ",")) {
                                    yaEnviada = true;
                                    break;
                                }
                            }
                            archivo.close();
                        }

                        if (!yaEnviada) {
                            if (archivo.open(QIODevice::Append | QIODevice::Text)) {
                                QTextStream out(&archivo);
                                out << usuario.getUsuario() << "," << usuario.getNombre() << "," << usuario.getAvatar() << "\n";
                                archivo.close();
                                QMessageBox::information(this, "Solicitud enviada", "Se envió una solicitud a " + u.getUsuario());
                            } else {
                                QMessageBox::warning(this, "Error", "No se pudo escribir la solicitud.");
                            }
                        } else {
                            QMessageBox::information(this, "Ya solicitada", "Ya habías enviado solicitud a " + u.getUsuario());
                        }

                        break;
                    }
                }
            }
        }
    });

    connect(removeButton, &QPushButton::clicked, this, [=]() {
        QListWidgetItem *item = contactList->currentItem();
        if (!item) {
            QMessageBox::warning(this, "Eliminar contacto", "Selecciona un contacto para eliminar.");
            return;
        }

        QString contacto = item->data(Qt::UserRole).toString().trimmed();

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirmar eliminación",
                                      QString("¿Deseas eliminar a %1 de tus contactos?\nSe eliminará también el historial de chat.").arg(contacto),
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            // 🗑️ 1. Eliminar visualmente
            delete item;

            // 💾 2. Guardar contactos actualizados
            guardarContactosActuales();

            // 🧹 3. Eliminar historial del chat
            QString rutaChat = ChatScreen::nombreArchivoChat(usuario.getUsuario(), contacto);
            if (QFile::exists(rutaChat)) {
                QFile::remove(rutaChat);
            }

            // 📝 4. Registrar eliminación
            QString rutaEliminados = "/Users/anavalle/Desktop/chat/eliminados/eliminados_" + contacto + ".txt";
            QFile archivo(rutaEliminados);
            if (archivo.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream out(&archivo);
                out << usuario.getUsuario() << "\n";
                archivo.close();
            }

            // ✅ 5. Si el contacto eliminado es el que está abierto, cerrar chat y mostrar pantalla neutral
            if (chatWidget && contacto == chatWidget->getContactoActual()) {
                // Cerrar y eliminar chat activo
                chatHomeScreen->layout()->removeWidget(chatWidget);
                chatWidget->deleteLater();
                chatWidget = nullptr;

                // Mostrar pantalla de bienvenida
                if (logoHome) logoHome->setVisible(true);
                if (welcomeText) welcomeText->setVisible(false);
                if (welcomeWidget) welcomeWidget->setVisible(true);

                contentStack->setCurrentIndex(0);

                contactList->setVisible(true);
                solicitudesList->setVisible(false);
                solicitudesLabel->setVisible(false);
                chatButton->setChecked(true);
                solicitudesButton->setChecked(false);
            }
        }
    });

    connect(solicitudesButton, &QPushButton::clicked, this, [=]() {
        enModoSolicitudes = true;

        contactList->setVisible(false);
        solicitudesList->setVisible(true);
        solicitudesLabel->setVisible(true);

        chatButton->setChecked(false);
        solicitudesButton->setChecked(true);

        cargarSolicitudesPendientes();  // solo aquí se cargan visualmente
        contentStack->setCurrentIndex(0);
    });

    // ... dentro de tu función donde conectas el click de los contactos ...
    connect(contactList, &QListWidget::itemClicked, this, [=](QListWidgetItem *item) {
        QString usuarioSeleccionado = item->data(Qt::UserRole).toString();
        QString avatarSeleccionado;

        // Buscar el avatar del usuario seleccionado
        for (const Usuario &u : usuariosTotales) {
            if (u.getUsuario() == usuarioSeleccionado) {
                avatarSeleccionado = u.getAvatar();
                break;
            }
        }

        // Elimina el widget de chat anterior si existe
        if (chatWidget) {
            chatHomeScreen->layout()->removeWidget(chatWidget);
            chatWidget->deleteLater();
            chatWidget = nullptr;
        }

        // Crea un nuevo widget de chat
        chatWidget = new ChatScreen(this);
        chatWidget->setUsuarioActual(usuario.getUsuario());
        chatWidget->setContacto(usuarioSeleccionado, avatarSeleccionado);
        chatHomeScreen->layout()->addWidget(chatWidget);
        chatWidget->cargarMensajesDesdeArchivo();

        connect(chatWidget, &ChatScreen::eliminarContacto, this, [=](const QString &contacto) {
            // Eliminar contacto visualmente
            for (int i = 0; i < contactList->count(); ++i) {
                QListWidgetItem *item = contactList->item(i);
                if (item->data(Qt::UserRole).toString() == contacto) {
                    delete contactList->takeItem(i);
                    break;
                }
            }

            guardarContactosActuales();

            // 🗑️ Eliminar historial de chat
            QString rutaChat = ChatScreen::nombreArchivoChat(usuario.getUsuario(), contacto);
            if (QFile::exists(rutaChat)) {
                QFile::remove(rutaChat);  // ✅ Esto elimina "ana_cris.txt", "ana_monic.txt", etc.
            }

            // 💾 Registrar eliminación para el otro usuario
            QString archivoRuta = "/Users/anavalle/Desktop/chat/eliminados/eliminados_" + contacto + ".txt";
            QFile archivo(archivoRuta);
            if (archivo.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream out(&archivo);
                out << usuario.getUsuario() << "\n";
                archivo.close();
            }

            // Ocultar vista del chat
            if (chatWidget) {
                chatHomeScreen->layout()->removeWidget(chatWidget);
                chatWidget->deleteLater();
                chatWidget = nullptr;
            }
            if (logoHome) logoHome->setVisible(true);
            if (welcomeText) welcomeText->setVisible(false);
            if (welcomeWidget) welcomeWidget->setVisible(true);
        });

        // Oculta la vista de bienvenida (avatar grande)
        if (welcomeWidget) {
            welcomeWidget->setVisible(false);
        }

        // ❌ Botón "X" de cerrar (parte superior derecha del header)
        QPushButton *btnCerrarChat = new QPushButton("✖", chatWidget);
        btnCerrarChat->setFixedSize(30, 30);
        btnCerrarChat->setCursor(Qt::PointingHandCursor);
        btnCerrarChat->setStyleSheet("border: none; color: #800020; font-size: 18px; font-weight: bold;");

        // Insertar la "X" al header layout del chat
        QHBoxLayout *headerLayout = chatWidget->findChild<QHBoxLayout*>();
        if (headerLayout) {
            headerLayout->addStretch();
            headerLayout->addWidget(btnCerrarChat);
        }

        // Acción al presionar la "X"
        connect(btnCerrarChat, &QPushButton::clicked, this, [=]() {
            if (chatWidget) {
                chatHomeScreen->layout()->removeWidget(chatWidget);
                chatWidget->deleteLater();
                chatWidget = nullptr;
            }

            if (logoHome) logoHome->setVisible(true);
            if (welcomeText) welcomeText->setVisible(false);
            if (welcomeWidget) welcomeWidget->setVisible(true);

            contentStack->setCurrentIndex(0);

            contactList->setVisible(true);
            solicitudesList->setVisible(false);
            solicitudesLabel->setVisible(false);
            chatButton->setChecked(true);
            solicitudesButton->setChecked(false);
        });
    });
}

void ChatWindow::cargarUsuariosDesdeArchivo() {
    QFile file("/Users/anavalle/Desktop/chat/usuarios.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        usuariosTotales.clear(); // Limpia antes de recargar

        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed();
            QStringList partes = linea.split(",");

            if (partes.size() >= 8) {
                QString usuarioU = partes[0].trimmed();
                QString nombre = partes[1].trimmed();
                QString correo = partes[2].trimmed();
                QString contrasena = partes[3].trimmed();
                QString avatar = partes[4].trimmed();
                int edad = partes[5].trimmed().toInt();
                QString pregunta = partes[6].trimmed();
                QString respuesta = partes[7].trimmed();

                bool estadoConectado = false;
                if (partes.size() >= 9) {
                    estadoConectado = (partes[8].trimmed() == "1");
                }

                Usuario u(usuarioU, nombre, correo, contrasena, avatar, edad, pregunta, respuesta);
                u.setEstado(estadoConectado);
                usuariosTotales.append(u);
            } else {
                qDebug() << "⚠️ Línea inválida en usuarios.txt:" << linea;
            }
        }

        file.close();
        qDebug() << "✅ Usuarios cargados desde archivo:" << usuariosTotales.size();
    } else {
        qDebug() << "❌ No se pudo abrir usuarios.txt";
    }
}
void ChatWindow::filtrarUsuariosDisponibles() {
    for (const auto &u : usuariosTotales) {
        if (u.getUsuario() != usuario.getUsuario()) {
            usuariosDisponibles.append(u);
        }
    }
}

void ChatWindow::guardarContactosActuales() {
    QString archivoRuta = "/Users/anavalle/Desktop/chat/contactos/contactos_" + usuario.getUsuario().toLower() + ".txt";
    QFile file(archivoRuta);

    qDebug() << "📂 Guardando contactos en:" << archivoRuta;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        int contactosGuardados = 0;

        for (int i = 0; i < contactList->count(); ++i) {
            QListWidgetItem *item = contactList->item(i);
            QString usuarioVisual = item->data(Qt::UserRole).toString().trimmed();        // Conserva casing original
            QString usuarioComparar = usuarioVisual.toLower();                            // Para comparación

            if (!usuarioComparar.isEmpty()) {
                qDebug() << "🔍 Buscando datos del usuario (comparación):" << usuarioComparar;

                for (const Usuario &u : usuariosTotales) {
                    if (u.getUsuario().toLower() == usuarioComparar) {
                        // Guardamos con el casing original del registro
                        QString linea = u.getUsuario() + "," + u.getNombre() + "," + u.getAvatar();
                        out << linea << "\n";
                        qDebug() << "✅ Guardado:" << linea;
                        contactosGuardados++;
                        break;
                    }
                }
            }
        }

        file.close();
        qDebug() << "✅ Total contactos guardados:" << contactosGuardados;
    } else {
        qDebug() << "❌ Error al abrir el archivo para escribir:" << archivoRuta;
    }
}

void ChatWindow::cargarContactosGuardados() {
    QString archivoRuta = "/Users/anavalle/Desktop/chat/contactos/contactos_" + usuario.getUsuario().toLower() + ".txt";
    qDebug() << "📄 Leyendo contactos desde:" << archivoRuta;

    QFile file(archivoRuta);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        int contactosCargados = 0;

        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed();
            QStringList partes = linea.split(",");
            if (partes.size() >= 3) {
                QString usuarioGuardado = partes[0];
                QString nombreGuardado = partes[1];
                QString avatarRuta = partes[2];
                QString usuarioGuardadoLower = usuarioGuardado.toLower();

                qDebug() << "🧾 Contacto leído:" << linea;

                // Verificar si ya está en la lista
                bool yaExiste = false;
                for (int i = 0; i < contactList->count(); ++i) {
                    QListWidgetItem *item = contactList->item(i);
                    QString existente = item->data(Qt::UserRole).toString();
                    if (!existente.isEmpty() && existente.toLower() == usuarioGuardadoLower) {
                        yaExiste = true;
                        break;
                    }
                }
                if (yaExiste) continue;

                // === Crear widget visual ===
                QWidget *itemWidget = new QWidget();
                itemWidget->setStyleSheet(R"(
    background-color: transparent;
    border: none;
    border-bottom: 1px solid #e0e0e0;
    margin-bottom: 0px;
)");
                itemWidget->setContentsMargins(0, 0, 0, 0);

                QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
                itemLayout->setContentsMargins(8, 6, 8, 6);
                itemLayout->setSpacing(8);

                QLabel *avatarLabel = new QLabel();
                avatarLabel->setFixedSize(40, 40);
                QPixmap avatarPixmap(avatarRuta);
                if (!avatarPixmap.isNull()) {
                    QPixmap redondo(40, 40);
                    redondo.fill(Qt::transparent);
                    QPainter painter(&redondo);
                    painter.setRenderHint(QPainter::Antialiasing);
                    QPainterPath path;
                    path.addEllipse(0, 0, 40, 40);
                    painter.setClipPath(path);
                    painter.drawPixmap(0, 0, avatarPixmap.scaled(40, 40, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    avatarLabel->setPixmap(redondo);
                }

                QVBoxLayout *textLayout = new QVBoxLayout();
                textLayout->setContentsMargins(0, 0, 0, 0);
                textLayout->setSpacing(2);

                // 🧾 Nombre
                QLabel *userLabel = new QLabel(nombreGuardado);
                userLabel->setStyleSheet("font-weight: bold; font-size: 15px; color: #333;");
                userLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

                // 🟢 Estado
                QLabel *estadoLabel = new QLabel();
                estadoLabel->setObjectName("estadoLabel");
                QString estado = obtenerEstadoDesdeArchivo(usuarioGuardado);
                if (estado == "1") {
                    estadoLabel->setText("🟢 ");
                    estadoLabel->setStyleSheet("color: green; font-size: 13px;");
                } else {
                    estadoLabel->setText("⚪ ");
                    estadoLabel->setStyleSheet("color: gray; font-size: 13px;");
                }

                // 📌 Contenedor horizontal: Nombre + Estado
                QWidget *filaSuperior = new QWidget();
                QHBoxLayout *filaLayout = new QHBoxLayout(filaSuperior);
                filaLayout->setContentsMargins(0, 0, 0, 0);
                filaLayout->addWidget(userLabel);
                filaLayout->addWidget(estadoLabel);
                filaLayout->addStretch();

                // 📨 Último mensaje
                QLabel *ultimoMensajeLabel = new QLabel(" ");
                ultimoMensajeLabel->setObjectName("ultimoMensajeLabel");
                ultimoMensajeLabel->setStyleSheet("color: #666; font-size: 13px;");

                // 📂 Leer último mensaje desde archivo
                QString rutaChat = ChatScreen::nombreArchivoChat(usuario.getUsuario(), usuarioGuardado);
                QFile archivoChat(rutaChat);
                QString ultimoMensaje = "";

                if (archivoChat.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&archivoChat);
                    while (!in.atEnd()) {
                        QString linea = in.readLine().trimmed();
                        if (!linea.isEmpty()) {
                            ultimoMensaje = linea;
                        }
                    }
                    archivoChat.close();
                }

                if (!ultimoMensaje.isEmpty()) {
                    QRegularExpression re(R"((.+?)\s+\[\d{2}:\d{2}\]:\s+(.*))");
                    QRegularExpressionMatch match = re.match(ultimoMensaje);
                    if (match.hasMatch()) {
                        QString remitente = match.captured(1).trimmed();
                        QString contenido = match.captured(2).trimmed();
                        if (remitente == usuario.getUsuario())
                            ultimoMensajeLabel->setText("Tú: " + contenido);
                        else
                            ultimoMensajeLabel->setText(contenido);
                    } else {
                        ultimoMensajeLabel->setText(ultimoMensaje);
                    }
                }

                // 🔴 Contador de no leídos
                // 🔴 Contador de no leídos
                QLabel *contadorLabel = new QLabel("");
                contadorLabel->setObjectName("contadorLabel");
                contadorLabel->setStyleSheet("background-color: green; color: white; font-size: 12px; padding: 2px 6px; border-radius: 10px;");
                contadorLabel->setAlignment(Qt::AlignCenter);
                contadorLabel->setFixedSize(20, 20);
                contadorLabel->setVisible(false);


                int noLeidos = GestorNoLeidos::contarMensajes(usuario.getUsuario(), usuarioGuardado);
                if (noLeidos > 0) {
                    contadorLabel->setText(QString::number(noLeidos));
                    contadorLabel->setVisible(true);
                }

                // ✅ Registrar el contador en el mapa
                contadoresNoLeidos[usuarioGuardado.toLower()] = contadorLabel;

                textLayout->addWidget(filaSuperior);
                textLayout->addWidget(ultimoMensajeLabel);

                itemLayout->addWidget(avatarLabel);
                itemLayout->addLayout(textLayout);
                itemLayout->addWidget(contadorLabel);

                QListWidgetItem *item = new QListWidgetItem();
                item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                item->setSizeHint(itemWidget->sizeHint());
                item->setData(Qt::UserRole, usuarioGuardado);
                item->setData(Qt::UserRole + 1, QVariant::fromValue<void*>(ultimoMensajeLabel));
                item->setData(Qt::UserRole + 2, QVariant::fromValue<void*>(contadorLabel));

                // 🔵 Notificaciones visuales
                // 🔵 Notificaciones visuales estilo burbuja
                QMap<QString, int> notificaciones = GestorNotificaciones::leerNotificaciones(usuario.getUsuario());
                if (notificaciones.contains(usuarioGuardado) && notificaciones[usuarioGuardado] > 0) {
                    QLabel* lblNotificacion = new QLabel(QString::number(notificaciones[usuarioGuardado]));
                    lblNotificacion->setFixedSize(24, 24);
                    lblNotificacion->setAlignment(Qt::AlignCenter);
                    lblNotificacion->setStyleSheet(R"(
        background-color: #2ecc71;  /* Verde estilo WhatsApp/Discord */
        color: black;
        font-size: 14px;
        font-weight: bold;
        border-radius: 12px;
    )");

                    // 📦 Layout vertical para ubicar la burbuja correctamente (arriba derecha)
                    QVBoxLayout *bubbleLayout = new QVBoxLayout();
                    bubbleLayout->setContentsMargins(0, 0, 0, 0);
                    bubbleLayout->addWidget(lblNotificacion, 0, Qt::AlignRight);

                    itemLayout->addLayout(bubbleLayout);  // 📌 Agrega a la derecha
                }

                contactList->addItem(item);
                contactList->setItemWidget(item, itemWidget);
                contactosCargados++;
            }
        }

        file.close();
        qDebug() << "✅ Total contactos cargados:" << contactosCargados;
    } else {
        qDebug() << "❌ No se pudo abrir el archivo:" << archivoRuta;
    }

    if (clienteSocket) {
        qDebug() << "📨 Enviando solicitud de estado al servidor desde cargarContactosGuardados()";
        clienteSocket->enviarMensaje("ESTADO?");
    }
}

void ChatWindow::cargarSolicitudesPendientes() {
    QString archivoRuta = "/Users/anavalle/Desktop/chat/solicitudes/solicitudes_" + usuario.getUsuario() + ".txt";
    QFile file(archivoRuta);
    if (!file.exists()) return;

    solicitudesList->clear(); // Limpia solicitudes anteriores

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed();
            QStringList partes = linea.split(",");
            if (partes.size() >= 3) {
                QString userSolicitud = partes[0];
                QString nombre = partes[1];
                QString avatar = partes[2];
                QString solicitudLower = userSolicitud.toLower();

                // Verificar si ya fue agregado
                bool yaAgregado = false;
                for (int i = 0; i < contactList->count(); ++i) {
                    QString existente = contactList->item(i)->data(Qt::UserRole).toString().toLower();
                    if (existente == solicitudLower) {
                        yaAgregado = true;
                        break;
                    }
                }

                if (yaAgregado) continue;

                // === Diseño visual del item ===
                QFrame *itemWidget = new QFrame();
                itemWidget->setStyleSheet("QFrame { background: #fff; border: 1px solid #ddd; border-radius: 10px; }");
                QHBoxLayout *layout = new QHBoxLayout(itemWidget);
                layout->setContentsMargins(8, 5, 8, 5);
                layout->setSpacing(12);

                QLabel *avatarLabel = new QLabel();
                avatarLabel->setFixedSize(36, 36);
                QPixmap pixmap(avatar);
                if (!pixmap.isNull()) {
                    QPixmap redondo(36, 36);
                    redondo.fill(Qt::transparent);
                    QPainter painter(&redondo);
                    painter.setRenderHint(QPainter::Antialiasing);
                    QPainterPath path;
                    path.addEllipse(0, 0, 36, 36);
                    painter.setClipPath(path);
                    painter.drawPixmap(0, 0, pixmap.scaled(36, 36, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    avatarLabel->setPixmap(redondo);
                }

                QLabel *labelUser = new QLabel(userSolicitud);
                labelUser->setStyleSheet("font-weight: bold; font-size: 14px;");

                QPushButton *aceptarBtn = new QPushButton("Aceptar");
                QPushButton *rechazarBtn = new QPushButton("Rechazar");
                aceptarBtn->setStyleSheet("background-color: #4CAF50; color: white; border-radius: 6px; padding: 4px 8px;");
                rechazarBtn->setStyleSheet("background-color: #f44336; color: white; border-radius: 6px; padding: 4px 8px;");

                layout->addWidget(avatarLabel);
                layout->addWidget(labelUser);
                layout->addStretch();
                layout->addWidget(aceptarBtn);
                layout->addWidget(rechazarBtn);

                QListWidgetItem *item = new QListWidgetItem();
                item->setSizeHint(itemWidget->sizeHint());
                item->setData(Qt::UserRole, userSolicitud);

                solicitudesList->addItem(item);
                solicitudesList->setItemWidget(item, itemWidget);

                // ✅ Botón ACEPTAR
                connect(aceptarBtn, &QPushButton::clicked, this, [=]() {
                    agregarContactoMutuo(userSolicitud);
                    eliminarSolicitud(userSolicitud);
                    solicitudesList->takeItem(solicitudesList->row(item));

                    // 🔁 Actualizar contador con nueva clase
                    int pendientes = GestorVistaChat::contarSolicitudes(usuario.getUsuario(), contactList);
                    solicitudesButton->setText(pendientes > 0 ? QString("🔔 %1").arg(pendientes) : "🔔");

                    // 🔴 Eliminar de eliminados
                    QString rutaEliminados = "/Users/anavalle/Desktop/chat/eliminados/eliminados_" + usuario.getUsuario() + ".txt";
                    QFile archivo(rutaEliminados);
                    QStringList lineasLimpias;

                    if (archivo.exists() && archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        QTextStream in(&archivo);
                        while (!in.atEnd()) {
                            QString l = in.readLine().trimmed();
                            if (l != userSolicitud) lineasLimpias << l;
                        }
                        archivo.close();
                    }

                    if (archivo.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                        QTextStream out(&archivo);
                        for (const QString &l : lineasLimpias) out << l << "\n";
                        archivo.close();
                    }
                });

                // ❌ Botón RECHAZAR
                connect(rechazarBtn, &QPushButton::clicked, this, [=]() {
                    eliminarSolicitud(userSolicitud);
                    solicitudesList->takeItem(solicitudesList->row(item));

                    // 🔁 Actualizar contador con nueva clase
                    int pendientes = GestorVistaChat::contarSolicitudes(usuario.getUsuario(), contactList);
                    solicitudesButton->setText(pendientes > 0 ? QString("🔔 %1").arg(pendientes) : "🔔");
                });
            }
        }
        file.close();
    }

    // 🔁 Actualiza contador una vez más al final
    int pendientes = GestorVistaChat::contarSolicitudes(usuario.getUsuario(), contactList);
    solicitudesButton->setText(pendientes > 0 ? QString("🔔 %1").arg(pendientes) : "🔔");
}

void ChatWindow::agregarContactoMutuo(const QString &otroUsuario) {
    for (const Usuario &u : usuariosTotales) {
        if (u.getUsuario() == otroUsuario) {

            QString otroLower = otroUsuario.toLower();
            for (int i = 0; i < contactList->count(); ++i) {
                QString existente = contactList->item(i)->data(Qt::UserRole).toString();
                if (!existente.isEmpty() && existente.toLower() == otroLower) {
                    return; // Ya está agregado (sin importar casing)
                }
            }

            // ✅ Elimina al usuario del archivo de eliminados (si existía)
            QString archivoEliminado = "/Users/anavalle/Desktop/chat/eliminados/eliminados_" + usuario.getUsuario() + ".txt";
            QFile file(archivoEliminado);
            QStringList lineas;

            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    QString linea = in.readLine().trimmed();
                    if (linea != otroUsuario) {
                        lineas << linea;
                    }
                }
                file.close();
            }
            if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                QTextStream out(&file);
                for (const QString &l : lineas) {
                    out << l << "\n";
                }
                file.close();
            }

            // 🔁 Crear el widget visual usando la función reutilizable
            QLabel* ultimoLabel = nullptr;
            QLabel* contadorLabel = nullptr;

            QWidget *itemWidget = GestorVistaChat::crearWidgetContacto(
                u, usuario.getUsuario(), ultimoLabel, contadorLabel
                );

            // 🔁 Aplicar el mismo diseño visual aquí directamente
            itemWidget->setStyleSheet(R"(
    background-color: transparent;
    border: none;
    border-bottom: 1px solid #e0e0e0;
    margin-bottom: 0px;
)");
            itemWidget->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
            itemLayout->setContentsMargins(8, 6, 8, 6);
            itemLayout->setSpacing(8);

            QListWidgetItem *item = new QListWidgetItem();
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setSizeHint(itemWidget->sizeHint());
            item->setData(Qt::UserRole, u.getUsuario());
            item->setData(Qt::UserRole + 1, QVariant::fromValue<void*>(ultimoLabel));
            item->setData(Qt::UserRole + 2, QVariant::fromValue<void*>(contadorLabel));

            contactList->addItem(item);
            contactList->setItemWidget(item, itemWidget);

            // Registrar contador en el mapa
            contadoresNoLeidos[u.getUsuario().toLower()] = contadorLabel;

            // ✅ Guardar contacto en archivo local
            guardarContactosActuales();

            // ✅ Agregar al archivo del otro usuario
            QString archivoOtro = "/Users/anavalle/Desktop/chat/contactos/contactos_" + u.getUsuario() + ".txt";
            QFile file2(archivoOtro);
            if (file2.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream out(&file2);
                out << usuario.getUsuario() << "," << usuario.getNombre() << "," << usuario.getAvatar() << "\n";
                file2.close();
            }

            break;
        }
    }
}


void ChatWindow::eliminarSolicitud(const QString &remitente) {
    QString archivo = "/Users/anavalle/Desktop/chat/solicitudes/solicitudes_" + usuario.getUsuario() + ".txt";
    QFile file(archivo);
    QStringList nuevasLineas;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed();
            if (!linea.startsWith(remitente + ",")) {
                nuevasLineas << linea;
            }
        }
        file.close();
    }

    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        for (const QString &l : nuevasLineas) {
            if (!l.isEmpty()) {
                out << l << "\n";
            }
        }
        file.close();
    }
}
// void ChatWindow::actualizarContadorSolicitudes() {
//     int restantes = solicitudesList->count();
//     solicitudesButton->setText(restantes > 0 ? QString("🔔 %1").arg(restantes) : "🔔");
//     solicitudesList->setVisible(restantes > 0);
//     solicitudesLabel->setVisible(restantes > 0);
// }

ChatScreen* ChatWindow::getChatScreen() const {
    return chatScreen;
}

void ChatWindow::actualizarEstadosEnLista(const QMap<QString, QString>& estados) {
    qDebug() << "🔁 Iniciando actualización visual de estados...";

    for (int i = 0; i < contactList->count(); ++i) {
        QListWidgetItem *item = contactList->item(i);
        QString username = item->data(Qt::UserRole).toString();

        qDebug() << "🔎 Procesando usuario:" << username;

        QWidget *widget = contactList->itemWidget(item);
        if (!widget) {
            qDebug() << "❌ No se encontró widget visual para" << username;
            continue;
        }

        QLabel *estadoLabel = widget->findChild<QLabel*>("estadoLabel");
        if (!estadoLabel) {
            qDebug() << "❌ No se encontró QLabel con objectName 'estadoLabel' para" << username;
            continue;
        }

        QString estado = estados.value(username, "0").trimmed(); // ← Asumimos 0 si no hay dato

        if (estado == "1") {
            estadoLabel->setText("🟢 ");
            estadoLabel->setStyleSheet("color: green; font-size: 13px;");
        } else {
            estadoLabel->setText("⚪ ");
            estadoLabel->setStyleSheet("color: gray; font-size: 13px;");
        }

        qDebug() << "✅ Estado actualizado visualmente:" << username << "->" << estado;
    }

    qDebug() << "✅ Finalizada actualización de estados.";
}

QString ChatWindow::obtenerEstadoDesdeArchivo(const QString& usuario) {
    QFile archivo("/Users/anavalle/Desktop/chat/usuarios.txt");
    if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text))
        return "0";  // Por defecto, desconectado

    QTextStream in(&archivo);
    while (!in.atEnd()) {
        QString linea = in.readLine().trimmed();
        QStringList partes = linea.split(",");
        if (partes.size() >= 9 && partes[0].trimmed() == usuario.trimmed()) {
            return partes[8].trimmed();  // "1" o "0"
        }
    }

    return "0";  // Usuario no encontrado → se asume desconectado
}

void ChatWindow::cerrarSesion() {
    // ✅ Enviar mensaje de desconexión al servidor (DESCONECTADO:<usuario>)
    if (clienteSocket) {
        clienteSocket->desconectar();  // Método dedicado para cerrar correctamente
    }

    // ✅ Actualizar estado en archivo a "0" (desconectado)
    QString archivoRuta = "/Users/anavalle/Desktop/chat/usuarios.txt";
    QFile file(archivoRuta);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QStringList lineas;
        QTextStream in(&file);

        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed();
            QStringList partes = linea.split(",");

            if (partes.size() >= 9 && partes[0].trimmed() == usuario.getUsuario()) {
                partes[8] = "0";  // Estado desconectado
                linea = partes.join(",");
                qDebug() << "✅ Línea modificada:" << linea;
            }

            lineas << linea;
        }

        file.close();

        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            for (const QString &l : lineas)
                out << l << "\n";
            file.close();
            qDebug() << "✅ Estado actualizado a 0 en archivo.";
        } else {
            qDebug() << "❌ No se pudo abrir archivo para escritura.";
        }

    } else {
        qDebug() << "❌ No se pudo abrir archivo para lectura.";
    }

    // 🧭 Volver a ventana de login
    loginWindow = new LoginWindow();
    loginWindow->show();
    this->close();
}
void ChatWindow::verificarEstadosDesdeArchivo() {
    QMap<QString, QString> mapaEstados;

    QFile file("/Users/anavalle/Desktop/chat/usuarios.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "❌ No se pudo leer usuarios.txt para estados.";
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString linea = in.readLine().trimmed();
        QStringList partes = linea.split(",");
        if (partes.size() >= 9) {
            QString usuario = partes[0].trimmed();
            QString estado = partes[8].trimmed();  // "1" o "0"
            mapaEstados[usuario] = estado;
        }
    }
    file.close();

    // Llama a tu método actual que actualiza los estados visuales
    actualizarEstadosEnLista(mapaEstados);
}

void ChatWindow::mostrarPantallaBienvenida() {
    // Mostrar mensaje de bienvenida
    contentStack->setCurrentIndex(0); // asume que el index 0 es welcomeWidget
}
void ChatWindow::verificarSolicitudesDesdeArchivo() {
    QString archivoRuta = "/Users/anavalle/Desktop/chat/solicitudes/solicitudes_" + usuario.getUsuario() + ".txt";
    QFile file(archivoRuta);
    int contador = 0;

    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed();
            if (!linea.isEmpty()) {
                QStringList partes = linea.split(",");
                if (partes.size() >= 3) {
                    QString userSolicitud = partes[0].toLower();

                    // Verificar si ya fue agregado
                    bool yaAgregado = false;
                    for (int i = 0; i < contactList->count(); ++i) {
                        QString existente = contactList->item(i)->data(Qt::UserRole).toString().toLower();
                        if (existente == userSolicitud) {
                            yaAgregado = true;
                            break;
                        }
                    }

                    if (!yaAgregado) {
                        contador++;
                    }
                }
            }
        }
        file.close();
    }

    // ✅ Mostrar solo el contador en el botón (sin abrir la vista)
    solicitudesButton->setText(contador > 0 ? QString("🔔 %1").arg(contador) : "🔔");

    // Si estás en modo solicitudes, actualiza el texto también
    if (enModoSolicitudes) {
        solicitudesLabel->setText(QString("🔔 Solicitudes (%1)").arg(contador));
    }
}

void ChatWindow::actualizarContador(const QString& contacto) {
    for (int i = 0; i < contactList->count(); ++i) {
        QListWidgetItem* item = contactList->item(i);
        QString nombre = item->data(Qt::UserRole).toString();
        if (nombre.toLower() == contacto.toLower()) {
            QLabel* contadorLabel = static_cast<QLabel*>(item->data(Qt::UserRole + 2).value<void*>());
            int cantidad = mensajesNoLeidos[contacto.toLower()].tamano();
            if (cantidad > 0) {
                contadorLabel->setText(QString::number(cantidad));
                contadorLabel->setVisible(true);
            } else {
                contadorLabel->setVisible(false);
            }
            break;
        }
    }
}

void ChatWindow::actualizarUltimoMensaje(const QString& contacto, const QString& mensaje) {
    for (int i = 0; i < contactList->count(); ++i) {
        QListWidgetItem* item = contactList->item(i);
        if (item->data(Qt::UserRole).toString().toLower() == contacto.toLower()) {
            QWidget *widget = contactList->itemWidget(item);
            if (!widget) return;

            QLabel* ultimoLabel = widget->findChild<QLabel*>("ultimoMensajeLabel");
            if (!ultimoLabel) return;

            ultimoLabel->setText(mensaje);
            break;
        }
    }
}

void ChatWindow::limpiarContador(const QString& contacto) {
    QString clave = contacto.toLower();

    // 1. Limpiar mensajes no leídos
    mensajesNoLeidos[clave].limpiar();

    // 2. Ocultar contador visual
    if (contadoresNoLeidos.contains(clave)) {
        QLabel* label = contadoresNoLeidos[clave];
        label->clear();
        label->setVisible(false);
    }
}

void ChatWindow::registrarMensajeNoLeido(const QString& contacto, const QString& mensaje) {
    QString clave = contacto.toLower();

    mensajesNoLeidos[clave].encolar(mensaje);

    // 🔵 Aumentar contador de notificaciones
    GestorNotificaciones::incrementar(usuario.getUsuario(), contacto);

    actualizarContador(contacto);
    actualizarUltimoMensaje(contacto, mensaje);
}

void ChatWindow::verificarActualizacionArchivo() {
    for (int i = 0; i < contactList->count(); ++i) {
        QListWidgetItem *item = contactList->item(i);
        QString contacto = item->data(Qt::UserRole).toString();

        QLabel *contadorLabel = static_cast<QLabel*>(item->data(Qt::UserRole + 2).value<void*>());
        QLabel *ultimoLabel = static_cast<QLabel*>(item->data(Qt::UserRole + 1).value<void*>());

        QString ruta = ChatScreen::nombreArchivoChat(usuario.getUsuario(), contacto);
        QFile archivo(ruta);
        QString ultimo = "";

        if (archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&archivo);
            while (!in.atEnd()) {
                QString linea = in.readLine().trimmed();
                if (!linea.isEmpty()) {
                    ultimo = linea;
                }
            }
            archivo.close();
        }

        // Determinar prefijo
        QString prefijo = "";
        if (ultimo.startsWith(usuario.getUsuario() + " ")) {
            prefijo = "Tú: ";
        }

        // Extraer contenido con expresión regular
        QRegularExpression re(R"((.+?)\s+\[\d{2}:\d{2}\]:\s+(.*))");
        QRegularExpressionMatch match = re.match(ultimo);
        if (match.hasMatch()) {
            QString contenido = match.captured(2).trimmed();
            ultimoLabel->setText(prefijo + contenido);
        } else {
            ultimoLabel->setText(prefijo + ultimo);
        }
    }
}

void ChatWindow::verificarMensajesNoLeidos() {
    for (int i = 0; i < contactList->count(); ++i) {
        QListWidgetItem* item = contactList->item(i);
        QString contacto = item->data(Qt::UserRole).toString();
        QLabel* contador = static_cast<QLabel*>(item->data(Qt::UserRole + 2).value<void*>());

        int count = GestorNoLeidos::contarMensajes(usuario.getUsuario(), contacto);
        if (contador) {
            if (count > 0) {
                contador->setText(QString::number(count));
                contador->setVisible(true);
            } else {
                contador->setVisible(false);
            }
        }
    }
}

void ChatWindow::verificarActualizacionMensajesNoLeidos() {
    qDebug() << "🔁 Verificando actualización de mensajes no leídos...";

    for (int i = 0; i < contactList->count(); ++i) {
        QListWidgetItem *item = contactList->item(i);
        QString contacto = item->data(Qt::UserRole).toString();

        QLabel *contadorLabel = static_cast<QLabel*>(item->data(Qt::UserRole + 2).value<void*>());
        if (!contadorLabel) {
            qDebug() << "❌ No se encontró contadorLabel para" << contacto;
            continue;
        }

        int cantidad = GestorNoLeidos::contarMensajes(usuario.getUsuario(), contacto);
        if (cantidad > 0) {
            contadorLabel->setText(QString::number(cantidad));
            contadorLabel->setVisible(true);
            qDebug() << "📬 " << cantidad << " mensajes no leídos para " << contacto;
        } else {
            contadorLabel->clear();
            contadorLabel->setVisible(false);
        }
    }

    qDebug() << "✅ Actualización de mensajes no leídos finalizada.";
}

void ChatWindow::eliminarCuenta(const QString &usuarioEliminar) {
    // 1. Eliminar del archivo usuarios.txt
    QString rutaUsuarios = "/Users/anavalle/Desktop/chat/usuarios.txt";
    QFile archivoUsuarios(rutaUsuarios);

    if (archivoUsuarios.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QStringList lineas;
        QTextStream in(&archivoUsuarios);

        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed();
            if (!linea.startsWith(usuarioEliminar + ",")) {
                lineas << linea;
            }
        }
        archivoUsuarios.close();

        if (archivoUsuarios.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&archivoUsuarios);
            for (const QString &linea : lineas)
                out << linea << "\n";
            archivoUsuarios.close();
        }
    }

    // 2. Eliminar archivo de contactos propios
    QFile::remove("/Users/anavalle/Desktop/chat/contactos/contactos_" + usuarioEliminar + ".txt");

    // 3. Eliminar archivo de solicitudes propias
    QFile::remove("/Users/anavalle/Desktop/chat/solicitudes/solicitudes_" + usuarioEliminar + ".txt");

    // 🔴 4. Eliminar al usuario de las solicitudes de otros usuarios
    QDir dirSolicitudes("/Users/anavalle/Desktop/chat/solicitudes");
    QStringList archivosSolicitudes = dirSolicitudes.entryList(QStringList() << "solicitudes_*.txt", QDir::Files);

    for (const QString &archivo : archivosSolicitudes) {
        QString ruta = dirSolicitudes.absoluteFilePath(archivo);
        QFile f(ruta);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QStringList lineas;
            QTextStream in(&f);
            while (!in.atEnd()) {
                QString linea = in.readLine().trimmed();
                if (!linea.startsWith(usuarioEliminar + ",")) {
                    lineas << linea;
                }
            }
            f.close();

            if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                QTextStream out(&f);
                for (const QString &linea : lineas)
                    out << linea << "\n";
                f.close();
            }
        }
    }

    // 5. Eliminar archivo de mensajes y leídos
    QDir mensajes("/Users/anavalle/Desktop/chat/mensajes");
    QDir leidos("/Users/anavalle/Desktop/chat/leidos");
    QStringList archivosMensajes = mensajes.entryList(QStringList() << "*" + usuarioEliminar + "*.txt", QDir::Files);
    QStringList archivosLeidos = leidos.entryList(QStringList() << "*" + usuarioEliminar + "*.txt", QDir::Files);

    for (const QString &archivo : archivosMensajes)
        mensajes.remove(archivo);
    for (const QString &archivo : archivosLeidos)
        leidos.remove(archivo);

    // 6. Eliminar al usuario de los contactos de otros usuarios
    QDir dirContactos("/Users/anavalle/Desktop/chat/contactos");
    QStringList archivos = dirContactos.entryList(QStringList() << "contactos_*.txt", QDir::Files);

    for (const QString &archivo : archivos) {
        QString ruta = dirContactos.absoluteFilePath(archivo);
        QFile f(ruta);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QStringList lineas;
            QTextStream in(&f);
            while (!in.atEnd()) {
                QString linea = in.readLine().trimmed();
                if (!linea.startsWith(usuarioEliminar + ",")) {
                    lineas << linea;
                }
            }
            f.close();

            if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                QTextStream out(&f);
                for (const QString &linea : lineas)
                    out << linea << "\n";
                f.close();
            }
        }
    }

    // 7. Confirmar y cerrar sesión
    QMessageBox::information(this, "Cuenta eliminada", "Tu cuenta ha sido eliminada correctamente.");

    LoginWindow *login = new LoginWindow();
    login->show();
    this->close();
}

void ChatWindow::cargarContactosOrdenados(const QString& criterio) {
    qDebug() << "📦 Orden actual:" << criterio;

    QList<Usuario> contactos = obtenerContactosDesdeArchivo();
    QList<Usuario> ordenados = OrdenadorContactos::ordenar(contactos, criterio, usuario.getUsuario());

    contactList->clear();
    for (const Usuario& u : ordenados) {
        QLabel *ultimoLabel = nullptr;
        QLabel *contadorLabel = nullptr;
        QWidget *itemWidget = GestorVistaChat::crearWidgetContacto(u, usuario.getUsuario(), ultimoLabel, contadorLabel);

        itemWidget->setStyleSheet(R"(
            background-color: transparent;
            border: none;
            border-bottom: 1px solid #e0e0e0;
            margin-bottom: 0px;
        )");

        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(itemWidget->sizeHint());
        item->setData(Qt::UserRole, u.getUsuario());
        item->setData(Qt::UserRole + 1, QVariant::fromValue<void*>(ultimoLabel));
        item->setData(Qt::UserRole + 2, QVariant::fromValue<void*>(contadorLabel));

        contactList->addItem(item);
        contactList->setItemWidget(item, itemWidget);
    }
}
QList<Usuario> ChatWindow::obtenerContactosDesdeArchivo() {
    QList<Usuario> lista;

    QString archivoRuta = "/Users/anavalle/Desktop/chat/contactos/contactos_" + usuario.getUsuario().toLower() + ".txt";
    QFile file(archivoRuta);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "❌ No se pudo abrir archivo de contactos para lectura.";
        return lista;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString linea = in.readLine().trimmed();
        QStringList partes = linea.split(",");
        if (partes.size() >= 3) {
            QString usuarioU = partes[0].trimmed();
            QString nombre = partes[1].trimmed();
            QString avatar = partes[2].trimmed();
            Usuario u(usuarioU, nombre, "", "", avatar, 0, "", "");
            lista.append(u);
        }
    }

    file.close();
    return lista;
}

void ChatWindow::verificarNotificaciones() {
    QMap<QString, int> conteo = GestorNotificaciones::leerNotificaciones(usuario.getUsuario());

    for (int i = 0; i < contactList->count(); ++i) {
        QListWidgetItem* item = contactList->item(i);
        QString contacto = item->data(Qt::UserRole).toString().toLower();
        QWidget* widget = contactList->itemWidget(item);
        if (!widget) continue;

        // Buscar si ya existe una burbuja
        QLabel* notiLabel = widget->findChild<QLabel*>("notificacionLabel");
        if (!conteo.contains(contacto) || conteo[contacto] == 0) {
            if (notiLabel) {
                notiLabel->hide();
            }
            continue;
        }

        if (!notiLabel) {
            notiLabel = new QLabel(widget);
            notiLabel->setObjectName("notificacionLabel");
            notiLabel->setFixedSize(24, 24);
            notiLabel->setAlignment(Qt::AlignCenter);
            notiLabel->setStyleSheet(R"(
                background-color: #2ecc71;
                color: black;
                font-size: 14px;
                font-weight: bold;
                border-radius: 12px;
            )");

            // 📌 Agrega visualmente a la derecha del layout (como burbuja flotante)
            QLayout* layout = widget->layout();
            if (layout) {
                QVBoxLayout* extra = new QVBoxLayout();
                extra->setContentsMargins(0, 0, 0, 0);
                extra->addWidget(notiLabel, 0, Qt::AlignRight);
                layout->addItem(extra);
            }
        }

        notiLabel->setText(QString::number(conteo[contacto]));
        notiLabel->show();
    }
}

