#include "chatscreen.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QScrollArea>
#include <QScrollBar>
#include <QStackedLayout>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFile>
#include <QTextStream>
#include "mensaje_guardado.h"
#include <QDir>
#include <QTime>
#include <QRegularExpression>
#include "usuario.h"
#include <QDialog>
#include "chatwindow.h"
#include "gestornoleidos.h"
#include "gestornotificaciones.h"

ChatScreen::ChatScreen(QWidget *parent)
    : QWidget(parent), clienteSocket(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    avatarLabel = new QLabel();
    avatarLabel->setFixedSize(40, 40);
    avatarLabel->setStyleSheet("border-radius: 20px; background-color: #ccc;");

    contactNameLabel = new QLabel("Nombre del contacto");
    contactNameLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-left: 10px;");

    btnMostrarBusqueda = new QPushButton("🔍");
    btnMostrarBusqueda->setFixedSize(28, 28);
    btnMostrarBusqueda->setStyleSheet("border: none; font-size: 16px;");
    btnMostrarBusqueda->setCursor(Qt::PointingHandCursor);

    headerLayout->addWidget(avatarLabel);
    headerLayout->addWidget(contactNameLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(btnMostrarBusqueda);

    mainLayout->addLayout(headerLayout);

    panelBusqueda = new QWidget();
    panelBusqueda->setVisible(false);

    QHBoxLayout *searchLayout = new QHBoxLayout(panelBusqueda);
    buscadorLineEdit = new QLineEdit();
    buscadorLineEdit->setPlaceholderText("Buscar palabra...");
    btnAnterior = new QPushButton("←");
    btnSiguiente = new QPushButton("→");
    resultadoLabel = new QLabel("0 resultados");
    btnCerrarBusqueda = new QPushButton("❌");

    searchLayout->addWidget(buscadorLineEdit);
    searchLayout->addWidget(btnAnterior);
    searchLayout->addWidget(btnSiguiente);
    searchLayout->addWidget(resultadoLabel);
    searchLayout->addWidget(btnCerrarBusqueda);
    searchLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addWidget(panelBusqueda);

    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("background-color: white; border: none;");
    messagesWidget = new QWidget();
    messagesLayout = new QVBoxLayout(messagesWidget);
    messagesLayout->setAlignment(Qt::AlignTop);
    messagesLayout->setSpacing(1);
    scrollArea->setWidget(messagesWidget);
    mainLayout->addWidget(scrollArea);

    connect(btnMostrarBusqueda, &QPushButton::clicked, this, [=]() {
        panelBusqueda->setVisible(true);
    });


    connect(btnCerrarBusqueda, &QPushButton::clicked, this, [=]() {
        panelBusqueda->setVisible(false);
        buscadorLineEdit->clear();
        resultadoLabel->setText("0 resultados");


        for (QObject *obj : messagesWidget->children()) {
            QWidget *bubble = qobject_cast<QWidget*>(obj);
            if (!bubble) continue;

            QLabel *label = bubble->findChild<QLabel*>();
            if (!label) continue;

            QString originalStyle = label->property("originalStyle").toString();
            if (!originalStyle.isEmpty()) {
                label->setStyleSheet(originalStyle);
            }
        }

        mensajesResaltados.clear();
        indiceActual = -1;
    });

    connect(buscadorLineEdit, &QLineEdit::returnPressed, this, [=]() {
        buscarMensajes(buscadorLineEdit->text().trimmed());
    });

    connect(btnAnterior, &QPushButton::clicked, this, [=]() {
        navegarResultado(-1);
    });
    connect(btnSiguiente, &QPushButton::clicked, this, [=]() {
        navegarResultado(1);
    });


    QHBoxLayout *inputLayout = new QHBoxLayout();
    QPushButton *stickerButton = new QPushButton("😊");
    stickerButton->setFixedSize(34, 34);
    stickerButton->setStyleSheet("border: none;");
    inputLayout->addWidget(stickerButton);

    messageInput = new QLineEdit();
    messageInput->setPlaceholderText("Escribe un mensaje...");
    messageInput->setStyleSheet("padding: 8px; border-radius: 10px; border: 1px solid #ccc;");
    inputLayout->addWidget(messageInput);

    QPushButton *undoButton = new QPushButton("↩️");
    undoButton->setFixedSize(34, 34);
    undoButton->setStyleSheet("border: none;");
    connect(undoButton, &QPushButton::clicked, this, &ChatScreen::deshacerUltimoMensaje);

    sendButton = new QPushButton("➤");
    sendButton->setFixedSize(34, 34);
    sendButton->setStyleSheet("background-color: #800020; color: white; border-radius: 10px;");
    inputLayout->addWidget(undoButton);
    inputLayout->addWidget(sendButton);
    mainLayout->addLayout(inputLayout);

    crearGaleriaStickers();
    connect(stickerButton, &QPushButton::clicked, this, [=]() {
        QPoint globalPos = stickerButton->mapToGlobal(QPoint(0, -stickerPopup->height()));
        stickerPopup->move(globalPos);
        stickerPopup->show();
    });

    connect(sendButton, &QPushButton::clicked, this, [=]() {
        QString texto = messageInput->text().trimmed();
        if (!texto.isEmpty()) {
            QWidget *bubbleWidget = new QWidget();

            QLabel *messageLabel = new QLabel(texto);
            messageLabel->setStyleSheet("background-color: #800020; color: white; padding: 8px 12px; border-radius: 10px;");
            messageLabel->setMaximumWidth(this->width() * 0.80);
            messageLabel->setProperty("originalStyle", messageLabel->styleSheet());
            messageLabel->setWordWrap(true);


            QPushButton *deleteBtn = new QPushButton("🗑️");
            deleteBtn->setStyleSheet("border: none;");
            deleteBtn->setFixedSize(20, 20);
            deleteBtn->hide();

            QVBoxLayout *messageLayout = new QVBoxLayout();
            messageLayout->addWidget(messageLabel);
            messageLayout->addWidget(deleteBtn, 0, Qt::AlignRight);
            messageLayout->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout *bubbleLayout = new QHBoxLayout();
            bubbleLayout->addStretch();
            bubbleLayout->addLayout(messageLayout);
            bubbleWidget->setLayout(bubbleLayout);
            bubbleWidget->installEventFilter(this);

            connect(deleteBtn, &QPushButton::clicked, this, [=]() {
                if (QMessageBox::question(this, "Eliminar mensaje", "¿Seguro que quieres eliminar este mensaje?") == QMessageBox::Yes) {
                    int posicion = messagesLayout->indexOf(bubbleWidget);
                    messagesLayout->removeWidget(bubbleWidget);
                    QLabel* label = bubbleWidget->findChild<QLabel*>();
                    QString contenido = label ? label->text().trimmed() : "";
                    historialMensajes.apilar(MensajeGuardado(bubbleWidget, posicion, contenido));
                    bubbleWidget->hide();
                }
            });

            messagesLayout->addWidget(bubbleWidget);
            messageInput->clear();

            QString ruta = nombreArchivoChat(usuarioActual, contactNameLabel->text());
            QFile archivo(ruta);
            if (archivo.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream out(&archivo);
                QString hora = QTime::currentTime().toString("hh:mm");
                out << usuarioActual << " [" << hora << "]: " << texto << "\n";
                archivo.close();
            }

            QTimer::singleShot(0, scrollArea->verticalScrollBar(), [=]() {
                scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
            });

            if (clienteSocket) {
                clienteSocket->enviarMensaje(texto);
            }
        }
    });

    clienteSocket = nullptr;

    timerActualizarChat = new QTimer(this);
    connect(timerActualizarChat, &QTimer::timeout, this, &ChatScreen::verificarActualizacionArchivo);
    timerActualizarChat->start(1000);
}


void ChatScreen::setContacto(const QString &nombre, const QString &avatarPath)
{
    contactoActual = nombre;
    contactNameLabel->setText(nombre);

    QPixmap pixmap(avatarPath);
    if (!pixmap.isNull()) {
        QPixmap redondo(40, 40);
        redondo.fill(Qt::transparent);
        QPainter painter(&redondo);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addEllipse(0, 0, 40, 40);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, pixmap.scaled(40, 40, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        avatarLabel->setPixmap(redondo);
    }

    bool ambosAgregados = Usuario::sonContactosMutuos(usuarioActual, contactoActual);

    QString rutaEliminados = "/Users/anavalle/Desktop/chat/eliminados/eliminados_" + usuarioActual + ".txt";
    if (ambosAgregados) {
        QFile archivo(rutaEliminados);
        QStringList lineasLimpias;

        if (archivo.exists() && archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&archivo);
            while (!in.atEnd()) {
                QString linea = in.readLine().trimmed();
                if (linea != contactoActual) {
                    lineasLimpias << linea;
                }
            }
            archivo.close();
        }

        if (archivo.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&archivo);
            for (const QString &l : lineasLimpias)
                out << l << "\n";
            archivo.close();
        }
    }

    bool eliminadoPorElOtro = false;
    if (!ambosAgregados) {
        QFile archivoEliminado(rutaEliminados);
        if (archivoEliminado.exists() && archivoEliminado.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&archivoEliminado);
            while (!in.atEnd()) {
                if (in.readLine().trimmed() == contactoActual) {
                    QFile::remove(nombreArchivoChat(usuarioActual, contactoActual));
                    eliminadoPorElOtro = true;
                    break;
                }
            }
            archivoEliminado.close();
        }
    }

    if (eliminadoPorElOtro) {
        QMessageBox::warning(this, "Contacto eliminado",
                             "Este contacto te ha eliminado. El historial de chat fue borrado.");
    }

    while (QLayoutItem* item = messagesLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    if (!ambosAgregados) {
        messageInput->setEnabled(false);
        sendButton->setEnabled(false);

        QLabel *mensaje = new QLabel("🚫 Este contacto no te ha agregado todavía.\nNo puedes chatear.");
        mensaje->setAlignment(Qt::AlignCenter);
        mensaje->setStyleSheet("color: red; font-size: 15px; font-weight: bold;");

        QPushButton *btnEnviarSolicitud = new QPushButton("Enviar solicitud");
        btnEnviarSolicitud->setStyleSheet("background-color: #800020; color: white; padding: 8px 16px; border-radius: 8px;");

        QPushButton *btnEliminar = new QPushButton("Eliminar contacto");
        btnEliminar->setStyleSheet("background-color: #ccc; color: black; padding: 8px 16px; border-radius: 8px;");

        QHBoxLayout *acciones = new QHBoxLayout();
        acciones->addWidget(btnEnviarSolicitud);
        acciones->addWidget(btnEliminar);
        acciones->setSpacing(15);
        acciones->setAlignment(Qt::AlignCenter);

        QVBoxLayout *contenedorLayout = new QVBoxLayout();
        contenedorLayout->addWidget(mensaje);
        contenedorLayout->addLayout(acciones);
        contenedorLayout->setSpacing(20);

        QWidget *bloqueoWidget = new QWidget();
        bloqueoWidget->setStyleSheet("background-color: #f9f9f9; border-radius: 12px;");
        bloqueoWidget->setLayout(contenedorLayout);
        bloqueoWidget->setContentsMargins(20, 20, 20, 20);

        messagesLayout->addStretch();
        messagesLayout->addWidget(bloqueoWidget, 0, Qt::AlignCenter);
        messagesLayout->addStretch();

        connect(btnEnviarSolicitud, &QPushButton::clicked, this, [=]() {
            QString rutaSolicitud = "/Users/anavalle/Desktop/chat/solicitudes/solicitudes_" + contactoActual + ".txt";
            QString usuarioOrigen = usuarioActual;
            QString nombreOrigen = "";
            QString avatarOrigen = "";

            QString rutaUsuarios = "/Users/anavalle/Desktop/chat/usuarios.txt";
            QFile ufile(rutaUsuarios);
            if (ufile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&ufile);
                while (!in.atEnd()) {
                    QString linea = in.readLine().trimmed();
                    QStringList partes = linea.split(",");
                    if (partes.size() >= 3 && partes[0].toLower() == usuarioOrigen.toLower()) {
                        nombreOrigen = partes[1];
                        avatarOrigen = partes[2];
                        break;
                    }
                }
                ufile.close();
            }

            if (nombreOrigen.isEmpty() || avatarOrigen.isEmpty()) {
                QMessageBox::warning(this, "Error", "No se pudo encontrar la información del usuario para enviar la solicitud.");
                return;
            }


            bool yaEnviada = false;
            QFile archivoCheck(rutaSolicitud);
            if (archivoCheck.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&archivoCheck);
                while (!in.atEnd()) {
                    QString linea = in.readLine().trimmed();
                    if (linea.startsWith(usuarioOrigen + ",")) {
                        yaEnviada = true;
                        break;
                    }
                }
                archivoCheck.close();
            }

            if (yaEnviada) {
                QMessageBox::information(this, "Solicitud ya enviada", "Ya habías enviado una solicitud a este contacto.");
                return;
            }

            QFile archivo(rutaSolicitud);
            if (archivo.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream out(&archivo);
                out << usuarioOrigen << "," << nombreOrigen << "," << avatarOrigen << "\n";
                archivo.close();

                QMessageBox::information(this, "Solicitud enviada", "Tu solicitud fue enviada correctamente.");

                btnEnviarSolicitud->hide();

                QLabel *confirmacion = new QLabel("📨 Solicitud enviada. Esperando respuesta...");
                confirmacion->setStyleSheet("color: gray; font-style: italic;");
                messagesLayout->addWidget(confirmacion);
            } else {
                QMessageBox::warning(this, "Error", "No se pudo guardar la solicitud.");
            }
        });

        connect(btnEliminar, &QPushButton::clicked, this, [=]() {
            emit eliminarContacto(contactoActual);
        });

        return;
    }

    messageInput->setEnabled(true);
    sendButton->setEnabled(true);
    cargarMensajesDesdeArchivo();
    ultimaModificacionArchivo = QFileInfo(nombreArchivoChat(usuarioActual, contactoActual)).lastModified();
}

void ChatScreen::crearGaleriaStickers() {
    stickerPopup = new QWidget(this, Qt::Popup);
    stickerPopup->setAttribute(Qt::WA_TranslucentBackground);
    stickerPopup->setStyleSheet(R"(
        QWidget {
            background-color: #ffffff;
            border-radius: 12px;
            padding: 10px;
        }
    )");
    stickerPopup->setFixedSize(240, 160);

    QVBoxLayout *popupLayout = new QVBoxLayout(stickerPopup);
    popupLayout->setContentsMargins(10, 10, 10, 10);
    popupLayout->setSpacing(8);

    QLabel *title = new QLabel("Stickers");
    title->setAlignment(Qt::AlignCenter);
    title->setFixedHeight(24);
    title->setStyleSheet("font-size: 14px; font-weight: bold; color: #800020; padding: 2px 0; margin: 0;");
    popupLayout->addWidget(title);

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(10);
    popupLayout->addLayout(grid);

    QStringList stickerPaths = {
        ":/stickers/sticker1.png",
        ":/stickers/sticker2.png",
        ":/stickers/sticker3.png",
        ":/stickers/sticker4.png",
        ":/stickers/sticker5.png",
        ":/stickers/sticker6.png",

    };

    int row = 0, col = 0;
    for (const QString &path : stickerPaths) {
        QPixmap pix(path);
        QPushButton *btn = new QPushButton();
        btn->setIcon(QIcon(pix));
        btn->setIconSize(QSize(48, 48));
        btn->setFixedSize(52, 52);
        btn->setStyleSheet("border: none; border-radius: 6px; background-color: transparent;");

        grid->addWidget(btn, row, col);
        col++;
        if (col == 3) { col = 0; row++; }

        connect(btn, &QPushButton::clicked, this, [=]() {
            QLabel *stickerLabel = new QLabel();
            stickerLabel->setPixmap(pix.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));

            QPushButton *deleteBtn = new QPushButton("🗑️");
            deleteBtn->setStyleSheet("border: none;");
            deleteBtn->setFixedSize(20, 20);
            deleteBtn->hide();

            QVBoxLayout *innerLayout = new QVBoxLayout();
            innerLayout->addWidget(stickerLabel);
            innerLayout->addWidget(deleteBtn, 0, Qt::AlignRight);
            innerLayout->setContentsMargins(0, 0, 0, 0);

            QWidget *bubbleWidget = new QWidget();
            QHBoxLayout *bubbleLayout = new QHBoxLayout();
            bubbleLayout->addStretch();
            bubbleLayout->addLayout(innerLayout);
            bubbleWidget->setLayout(bubbleLayout);

            bubbleWidget->installEventFilter(this);

            connect(deleteBtn, &QPushButton::clicked, this, [=]() {
                if (QMessageBox::question(this, "Eliminar sticker", "¿Seguro que quieres eliminar este sticker?") == QMessageBox::Yes) {
                    int posicion = messagesLayout->indexOf(bubbleWidget);
                    messagesLayout->removeWidget(bubbleWidget);
                    QLabel* label = bubbleWidget->findChild<QLabel*>();
                    QString contenido = label ? label->text().trimmed() : "";
                    historialMensajes.apilar(MensajeGuardado(bubbleWidget, posicion, contenido));
                    bubbleWidget->hide();
                }
            });

            messagesLayout->addWidget(bubbleWidget);
            stickerPopup->hide();

            QDir dir;
            if (!dir.exists("mensajes")) {
                dir.mkdir("mensajes");
            }

            QString ruta = nombreArchivoChat(usuarioActual, contactNameLabel->text());
            QFile archivo(ruta);
            if (archivo.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream out(&archivo);
                QString hora = QTime::currentTime().toString("hh:mm");
                out << usuarioActual << " [" << hora << "]: STICKER:" << path << "\n";
                archivo.close();
            }

            QTimer::singleShot(0, scrollArea->verticalScrollBar(), [=]() {
                scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
            });
        });
    }
}
void ChatScreen::deshacerUltimoMensaje() {
    if (!historialMensajes.estaVacia()) {
        MensajeGuardado restaurado = historialMensajes.desapilar();
        QString textoEliminado = restaurado.contenido.trimmed();
        QString ruta = nombreArchivoChat(usuarioActual, contactNameLabel->text());

        qDebug() << "🛠️ Deshaciendo mensaje...";
        qDebug() << "🔸 Contenido eliminado guardado:" << textoEliminado;
        qDebug() << "📄 Archivo de chat:" << ruta;

        QFile archivo(ruta);
        if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "❌ No se pudo abrir el archivo para lectura.";
            return;
        }

        QStringList lineas;
        QTextStream in(&archivo);
        while (!in.atEnd()) {
            lineas << in.readLine();
        }
        archivo.close();

        bool encontrado = false;

        for (int i = 0; i < lineas.size(); ++i) {
            QString linea = lineas[i].trimmed();
            qDebug() << "📌 Revisando línea:" << linea;

            QRegularExpression re(R"((.+?)\s+\[\d{2}:\d{2}\]:\s+\[ELIMINADO\]::(.*))");
            QRegularExpressionMatch match = re.match(linea);

            if (match.hasMatch()) {
                QString remitente = match.captured(1).trimmed();
                QString contenido = match.captured(2).trimmed();

                qDebug() << "🔍 Coincidencia: remitente=" << remitente << ", contenido=" << contenido;

                if (remitente == usuarioActual && contenido == textoEliminado) {
                    lineas[i].replace("[ELIMINADO]::", "");
                    encontrado = true;
                    qDebug() << "✅ Línea restaurada.";
                    break;
                }
            }
        }

        if (!encontrado) {
            qDebug() << "⚠️ No se encontró la línea eliminada que coincida con el contenido.";
        }

        if (archivo.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&archivo);
            for (const QString &l : lineas) {
                out << l << "\n";
            }
            archivo.close();
            qDebug() << "💾 Archivo reescrito correctamente.";
        } else {
            qDebug() << "❌ No se pudo abrir el archivo para escritura.";
        }

        cargarMensajesDesdeArchivo();
    } else {
        qDebug() << "⚠️ Pila de mensajes eliminados vacía.";
    }
}

bool ChatScreen::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *widget = qobject_cast<QWidget *>(watched);
        if (widget) {
            QList<QPushButton*> botones = widget->findChildren<QPushButton*>();
            for (QPushButton *btn : botones) {
                if (btn->text() == "🗑️") {
                    btn->setVisible(!btn->isVisible());
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

QString ChatScreen::nombreArchivoChat(const QString &usuario1, const QString &usuario2) {
    QString nombreA = usuario1.toLower().trimmed();
    QString nombreB = usuario2.toLower().trimmed();

    qDebug() << "📌 nombreA:" << nombreA;
    qDebug() << "📌 nombreB:" << nombreB;

    if (nombreA.isEmpty() || nombreB.isEmpty()) {
        qWarning() << "❗ No se puede crear el archivo: uno de los nombres está vacío.";
        return QString();
    }

    if (nombreA > nombreB)
        std::swap(nombreA, nombreB);

    QString rutaCarpeta = "/Users/anavalle/Desktop/chat/mensajes";

    QDir dir(rutaCarpeta);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    return rutaCarpeta + "/" + nombreA + "_" + nombreB + ".txt";
}

void ChatScreen::setUsuarioActual(const QString &usuario) {
    usuarioActual = usuario;
    if (clienteSocket) {
        clienteSocket->setNombreUsuario(usuario);
    }
}

void ChatScreen::cargarMensajesDesdeArchivo() {
    QString ruta = nombreArchivoChat(usuarioActual, contactNameLabel->text());
    QFile archivo(ruta);
    if (!archivo.exists()) return;

    while (QLayoutItem* item = messagesLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    QString ultimoMensajeMostrado = "";

    if (archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&archivo);
        while (!in.atEnd()) {
            QString texto = in.readLine().trimmed();
            if (texto.isEmpty()) continue;

            ultimoMensajeMostrado = texto;

            QRegularExpression re(R"((.+?)\s+\[\d{2}:\d{2}\]:\s+(.*))");
            QRegularExpressionMatch match = re.match(texto);
            QString remitente, contenido;
            if (match.hasMatch()) {
                remitente = match.captured(1).trimmed();
                contenido = match.captured(2).trimmed();
            } else {
                contenido = texto.trimmed();
            }

            QWidget *bubbleWidget = new QWidget();
            QVBoxLayout *messageLayout = new QVBoxLayout();
            QPushButton *deleteBtn = new QPushButton("🗑️");
            deleteBtn->setStyleSheet("border: none;");
            deleteBtn->setFixedSize(20, 20);
            deleteBtn->hide();

            if (contenido.startsWith("[ELIMINADO]::STICKER:")) {
                QLabel *messageLabel = new QLabel("🗑️ Sticker eliminado");
                messageLabel->setStyleSheet("color: gray; font-style: italic; padding: 8px 12px;");
                messageLayout->addWidget(messageLabel);
            }
            else if (contenido.startsWith("STICKER:")) {
                QString rutaSticker = contenido.mid(QString("STICKER:").length()).trimmed();
                QLabel *stickerLabel = new QLabel();
                QPixmap pix(rutaSticker);
                if (!pix.isNull()) {
                    stickerLabel->setPixmap(pix.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                } else {
                    stickerLabel->setText("❌ Error al cargar sticker");
                }
                messageLayout->addWidget(stickerLabel);
            }
            else if (contenido.startsWith("[ELIMINADO]::")) {
                QLabel *messageLabel = new QLabel("🗑️ Mensaje eliminado");
                messageLabel->setStyleSheet("color: gray; font-style: italic; padding: 8px 12px;");
                messageLayout->addWidget(messageLabel);
            }
            else {
                QLabel *messageLabel = new QLabel(contenido);
                messageLabel->setWordWrap(true);

                if (remitente == usuarioActual) {
                    messageLabel->setMaximumWidth(this->width() * 0.80);
                    messageLabel->setStyleSheet("background-color: #800020; color: white; padding: 8px 12px; border-radius: 10px;");
                    messageLabel->setProperty("originalStyle", messageLabel->styleSheet());
                } else {
                    messageLabel->setMaximumWidth(this->width() * 0.80);
                    messageLabel->setStyleSheet("background-color: #e0e0e0; color: black; padding: 8px 12px; border-radius: 10px;");
                    messageLabel->setProperty("originalStyle", messageLabel->styleSheet());
                }

                messageLabel->setProperty("originalStyle", messageLabel->styleSheet());
                messageLayout->addWidget(messageLabel);
            }

            messageLayout->addWidget(deleteBtn, 0, Qt::AlignRight);
            messageLayout->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout *bubbleLayout = new QHBoxLayout();
            if (remitente == usuarioActual) {
                bubbleLayout->addStretch();
                bubbleLayout->addLayout(messageLayout);
            } else {
                bubbleLayout->addLayout(messageLayout);
                bubbleLayout->addStretch();
            }

            bubbleWidget->setLayout(bubbleLayout);
            bubbleWidget->installEventFilter(this);

            connect(deleteBtn, &QPushButton::clicked, this, [=]() {
                if (QMessageBox::question(this, "Eliminar", "¿Seguro que quieres eliminar este mensaje?") == QMessageBox::Yes) {
                    QString ruta = nombreArchivoChat(usuarioActual, contactNameLabel->text());
                    QFile archivo(ruta);

                    if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text))
                        return;

                    QStringList lineas;
                    QTextStream in(&archivo);
                    while (!in.atEnd()) {
                        lineas << in.readLine();
                    }
                    archivo.close();

                    QLabel* label = bubbleWidget->findChild<QLabel*>();
                    if (!label) return;

                    QString contenidoWidget = label->text().trimmed();

                    for (int i = 0; i < lineas.size(); ++i) {
                        QString texto = lineas[i].trimmed();

                        QRegularExpression re(R"((.+?)\s+\[\d{2}:\d{2}\]:\s+(.*))");
                        QRegularExpressionMatch match = re.match(texto);
                        if (match.hasMatch()) {
                            QString remitente = match.captured(1).trimmed();
                            QString contenido = match.captured(2).trimmed();

                            if (remitente == usuarioActual && contenido == contenidoWidget) {
                                lineas[i] = remitente + " [" + QTime::currentTime().toString("hh:mm") + "]: [ELIMINADO]::" + contenido;
                                break;
                            }
                        }
                    }

                    if (archivo.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                        QTextStream out(&archivo);
                        for (const QString& l : lineas) {
                            out << l << "\n";
                        }
                        archivo.close();
                    }

                    int posicion = messagesLayout->indexOf(bubbleWidget);
                    QLabel* label2 = bubbleWidget->findChild<QLabel*>();
                    QString contenido = label2 ? label2->text().trimmed() : "";
                    historialMensajes.apilar(MensajeGuardado(bubbleWidget, posicion, contenido));
                    bubbleWidget->hide();
                }
            });

            messagesLayout->addWidget(bubbleWidget);
        }

        archivo.close();

        QTimer::singleShot(0, scrollArea->verticalScrollBar(), [=]() {
            scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
        });
    }

    if (!contactoActual.isEmpty() && !ultimoMensajeMostrado.isEmpty()) {
        QFile archivoLeidos("/Users/anavalle/Desktop/chat/leidos/leidos_" + usuarioActual + ".txt");

        QStringList nuevasLineas;
        if (archivoLeidos.exists() && archivoLeidos.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&archivoLeidos);
            while (!in.atEnd()) {
                QString linea = in.readLine().trimmed();
                if (!linea.startsWith(contactoActual + "|")) {
                    nuevasLineas << linea;
                }
            }
            archivoLeidos.close();
        }

        if (archivoLeidos.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&archivoLeidos);
            for (const QString &l : nuevasLineas)
                out << l << "\n";
            out << contactoActual << "|" << ultimoMensajeMostrado << "\n";
            archivoLeidos.close();
        }
    }
}
void ChatScreen::recibirMensaje(const QString& mensaje) {
    QRegularExpression re(R"((.+?)\s+\[\d{2}:\d{2}\]:\s+(.*))");
    QRegularExpressionMatch match = re.match(mensaje);

    QString remitente, contenido;
    if (match.hasMatch()) {
        remitente = match.captured(1).trimmed();
        contenido = match.captured(2).trimmed();
    } else {
        contenido = mensaje.trimmed();
    }

    QString ruta = nombreArchivoChat(usuarioActual, remitente);
    QFile archivo(ruta);
    if (archivo.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&archivo);
        QString hora = QTime::currentTime().toString("hh:mm");
        out << remitente << " [" << hora << "]: " << contenido << "\n";
        archivo.close();
    }

    QString contactoRemitente = remitente.toLower();
    QString contactoActualActivo = contactoActual.toLower();

    if (contactoRemitente != contactoActualActivo && ventanaPrincipal) {
        ventanaPrincipal->registrarMensajeNoLeido(contactoRemitente, mensaje);

        GestorNotificaciones::incrementar(usuarioActual, remitente);
    }

    QWidget *bubbleWidget = new QWidget();
    QLabel *messageLabel = new QLabel(contenido);
    messageLabel->setWordWrap(true);

    QHBoxLayout *bubbleLayout = new QHBoxLayout();
    QVBoxLayout *messageLayout = new QVBoxLayout();

    if (remitente == usuarioActual) {
        messageLabel->setStyleSheet("background-color: #800020; color: white; padding: 8px 12px; border-radius: 10px;");
        messageLabel->setMaximumWidth(this->width() * 0.80);
        bubbleLayout->addStretch();
        bubbleLayout->addLayout(messageLayout);
    } else {
        messageLabel->setStyleSheet("background-color: #e0e0e0; color: black; padding: 8px 12px; border-radius: 10px;");
        messageLabel->setMaximumWidth(this->width() * 0.80);
        bubbleLayout->addLayout(messageLayout);
        bubbleLayout->addStretch();
    }

    messageLabel->setProperty("originalStyle", messageLabel->styleSheet());
    messageLayout->addWidget(messageLabel);
    messageLayout->setContentsMargins(0, 0, 0, 0);
    bubbleWidget->setLayout(bubbleLayout);

    messagesLayout->addWidget(bubbleWidget);

    QTimer::singleShot(0, scrollArea->verticalScrollBar(), [=]() {
        scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
    });
}
void ChatScreen::setClienteSocket(ClienteSocket *socket)
{
    this->clienteSocket = socket;

    connect(clienteSocket, &ClienteSocket::mensajeRecibido, this, [=](const QString& remitente, const QString& mensaje) {
        if (remitente != contactoActual && remitente != usuarioActual) {
            GestorNoLeidos::registrar(usuarioActual, remitente, mensaje);
        }

        emit mensajeRecibido(remitente, mensaje);
    });
}
void ChatScreen::verificarActualizacionArchivo() {
    QString ruta = nombreArchivoChat(usuarioActual, contactNameLabel->text());
    QFileInfo info(ruta);

    if (!info.exists()) return;

    QDateTime modificacionActual = info.lastModified();
    if (modificacionActual > ultimaModificacionArchivo) {
        ultimaModificacionArchivo = modificacionActual;

        QLayoutItem* item;
        while ((item = messagesLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }


        cargarMensajesDesdeArchivo();
    }
}

void ChatScreen::limpiarChat() {
    while (QLayoutItem* item = messagesLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}
void ChatScreen::mostrarDialogoContactoNoAgregado()
{
    QDialog *dialogo = new QDialog(this);
    dialogo->setWindowTitle("Contacto no agregado");
    dialogo->setModal(true);
    dialogo->setStyleSheet("QDialog { background-color: white; border-radius: 10px; }");

    QLabel *label = new QLabel("🚫 Este contacto no te ha agregado todavía.\nNo puedes chatear.");
    label->setStyleSheet("color: red; font-size: 14px; font-weight: bold;");
    label->setAlignment(Qt::AlignCenter);

    QPushButton *btnSolicitud = new QPushButton("Enviar solicitud");
    btnSolicitud->setStyleSheet("background-color: #800020; color: white; padding: 6px 16px; border-radius: 8px;");

    QPushButton *btnEliminar = new QPushButton("Eliminar contacto");
    btnEliminar->setStyleSheet("background-color: #e0e0e0; color: black; padding: 6px 16px; border-radius: 8px;");


    QPushButton *btnCerrar = new QPushButton("✖");
    btnCerrar->setFixedSize(24, 24);
    btnCerrar->setStyleSheet("border: none; font-size: 16px; color: gray;");
    btnCerrar->setCursor(Qt::PointingHandCursor);


    QHBoxLayout *botonesLayout = new QHBoxLayout();
    botonesLayout->addStretch();
    botonesLayout->addWidget(btnSolicitud);
    botonesLayout->addWidget(btnEliminar);
    botonesLayout->addStretch();

    QVBoxLayout *layout = new QVBoxLayout(dialogo);
    layout->addWidget(btnCerrar, 0, Qt::AlignRight);
    layout->addWidget(label);
    layout->addSpacing(10);
    layout->addLayout(botonesLayout);
    layout->setContentsMargins(20, 20, 20, 20);

    connect(btnCerrar, &QPushButton::clicked, dialogo, &QDialog::close);

    connect(btnEliminar, &QPushButton::clicked, this, [=]() {
        emit eliminarContacto(contactoActual);
        dialogo->close();
    });

    connect(btnSolicitud, &QPushButton::clicked, this, [=]() {
        QString rutaSolicitud = "/Users/anavalle/Desktop/chat/solicitudes/solicitudes_" + contactoActual + ".txt";
        QFile archivo(rutaSolicitud);
        bool yaExiste = false;

        if (archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&archivo);
            while (!in.atEnd()) {
                if (in.readLine().startsWith(usuarioActual + ",")) {
                    yaExiste = true;
                    break;
                }
            }
            archivo.close();
        }

        if (yaExiste) {
            QMessageBox::information(this, "Ya enviada", "Ya habías enviado una solicitud.");
        } else if (archivo.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&archivo);
            out << usuarioActual << ",,,\n";
            archivo.close();
            QMessageBox::information(this, "Solicitud enviada", "Tu solicitud fue enviada.");
        }

        dialogo->close();
    });

    dialogo->exec();
}



QString ChatScreen::getContactoActual() const {
    return contactoActual;
}

void ChatScreen::buscarMensajes(const QString &palabra) {
    mensajesResaltados.clear();
    indiceActual = -1;

    for (QObject *obj : messagesWidget->children()) {
        QWidget *bubble = qobject_cast<QWidget*>(obj);
        if (!bubble) continue;

        QLabel *label = bubble->findChild<QLabel*>();
        if (!label) continue;

        QString texto = label->text();
        QString originalStyle = label->property("originalStyle").toString();

        if (!originalStyle.isEmpty()) {
            label->setStyleSheet(originalStyle);
        }

        if (texto.contains(palabra, Qt::CaseInsensitive)) {
            mensajesResaltados.append(label);
        }
    }

    resultadoLabel->setText(QString("%1 resultados").arg(mensajesResaltados.count()));

    if (!mensajesResaltados.isEmpty()) {
        indiceActual = 0;
        QLabel *primero = mensajesResaltados.at(indiceActual);
        QString originalStyle = primero->property("originalStyle").toString();
        primero->setStyleSheet(originalStyle + " background-color: yellow;");
        centrarMensaje(primero);
    }
}

void ChatScreen::navegarResultado(int direccion) {
    if (mensajesResaltados.isEmpty()) return;

    if (indiceActual >= 0 && indiceActual < mensajesResaltados.size()) {
        QLabel *actual = mensajesResaltados.at(indiceActual);
        QString originalStyle = actual->property("originalStyle").toString();
        actual->setStyleSheet(originalStyle);
    }

    indiceActual += direccion;
    if (indiceActual < 0) indiceActual = mensajesResaltados.count() - 1;
    if (indiceActual >= mensajesResaltados.count()) indiceActual = 0;

    QLabel *nuevo = mensajesResaltados.at(indiceActual);
    QString nuevoOriginal = nuevo->property("originalStyle").toString();
    nuevo->setStyleSheet(nuevoOriginal + " background-color: yellow;");
    centrarMensaje(nuevo);
}

void ChatScreen::centrarMensaje(QWidget *mensaje) {
    mensaje->setFocus();
    scrollArea->ensureWidgetVisible(mensaje);
}

void ChatScreen::setVentanaPrincipal(ChatWindow *ventana) {
    ventanaPrincipal = ventana;
}
