#include "gestorvistachat.h"
#include "gestornoleidos.h"  // ✅ Asegúrate de incluir esta cabecera
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QPainter>
#include <QPainterPath>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include "chatscreen.h"

QWidget* GestorVistaChat::crearWidgetContacto(const Usuario& contacto,
                                              const QString& usuarioActual,
                                              QLabel*& ultimoLabel,
                                              QLabel*& contadorLabelOut) {
    QWidget *itemWidget = new QWidget();
    itemWidget->setStyleSheet("background-color: transparent; border: none;");
    itemWidget->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
    itemLayout->setContentsMargins(0, 0, 0, 0);
    itemLayout->setSpacing(10);

    QLabel *avatarLabel = new QLabel();
    avatarLabel->setFixedSize(40, 40);
    QPixmap avatarPixmap(contacto.getAvatar());
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

    QLabel *userLabel = new QLabel(contacto.getNombre());
    userLabel->setStyleSheet("font-weight: bold; font-size: 15px; color: #333;");
    userLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QLabel *estadoLabel = new QLabel();
    estadoLabel->setObjectName("estadoLabel");
    QString estado = obtenerEstadoDesdeArchivo(contacto.getUsuario());
    if (estado == "1") {
        estadoLabel->setText("🟢 ");
        estadoLabel->setStyleSheet("color: green; font-size: 13px;");
    } else {
        estadoLabel->setText("⚪ ");
        estadoLabel->setStyleSheet("color: gray; font-size: 13px;");
    }

    QWidget *filaSuperior = new QWidget();
    QHBoxLayout *filaLayout = new QHBoxLayout(filaSuperior);
    filaLayout->setContentsMargins(0, 0, 0, 0);
    filaLayout->addWidget(userLabel);
    filaLayout->addWidget(estadoLabel);
    filaLayout->addStretch();

    QLabel *ultimoMensajeLabel = new QLabel(" ");
    ultimoMensajeLabel->setObjectName("ultimoMensajeLabel");
    ultimoMensajeLabel->setStyleSheet("color: #666; font-size: 13px;");

    QString rutaChat = ChatScreen::nombreArchivoChat(usuarioActual, contacto.getUsuario());
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
            if (remitente == usuarioActual)
                ultimoMensajeLabel->setText("Tú: " + contenido);
            else
                ultimoMensajeLabel->setText(contenido);
        } else {
            ultimoMensajeLabel->setText(ultimoMensaje);
        }
    }

    QLabel *contadorLabel = new QLabel("");
    contadorLabel->setObjectName("contadorLabel");
    contadorLabel->setStyleSheet("background-color: green; color: white; font-size: 12px; padding: 2px 6px; border-radius: 10px;");
    contadorLabel->setAlignment(Qt::AlignCenter);
    contadorLabel->setFixedSize(20, 20);
    contadorLabel->setVisible(false);

    // ✅ Mostrar mensajes no leídos
    int mensajesNoLeidos = GestorNoLeidos::contarMensajes(usuarioActual, contacto.getUsuario());
    if (mensajesNoLeidos > 0) {
        contadorLabel->setText(QString::number(mensajesNoLeidos));
        contadorLabel->setVisible(true);
    }

    textLayout->addWidget(filaSuperior);
    textLayout->addWidget(ultimoMensajeLabel);

    itemLayout->addWidget(avatarLabel);
    itemLayout->addLayout(textLayout);
    itemLayout->addWidget(contadorLabel);

    ultimoLabel = ultimoMensajeLabel;
    contadorLabelOut = contadorLabel;

    return itemWidget;
}

QString GestorVistaChat::obtenerEstadoDesdeArchivo(const QString& usuario) {
    QFile archivo("/Users/anavalle/Desktop/chat/usuarios.txt");
    if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text))
        return "0";

    QTextStream in(&archivo);
    while (!in.atEnd()) {
        QString linea = in.readLine().trimmed();
        QStringList partes = linea.split(",");
        if (partes.size() >= 9 && partes[0].trimmed() == usuario.trimmed()) {
            return partes[8].trimmed();
        }
    }
    return "0";
}

int GestorVistaChat::contarSolicitudes(const QString& usuarioActual, const QListWidget* contactList) {
    QString archivoRuta = "/Users/anavalle/Desktop/chat/solicitudes/solicitudes_" + usuarioActual + ".txt";
    QFile file(archivoRuta);
    if (!file.exists()) return 0;

    int contador = 0;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString linea = in.readLine().trimmed();
            if (!linea.isEmpty()) {
                QStringList partes = linea.split(",");
                if (partes.size() >= 3) {
                    QString userSolicitud = partes[0].toLower();

                    bool yaAgregado = false;
                    for (int i = 0; i < contactList->count(); ++i) {
                        QString existente = contactList->item(i)->data(Qt::UserRole).toString().toLower();
                        if (existente == userSolicitud) {
                            yaAgregado = true;
                            break;
                        }
                    }

                    if (!yaAgregado)
                        contador++;
                }
            }
        }
        file.close();
    }

    return contador;
}
