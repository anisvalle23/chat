#include "agregarcontactodialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>

AgregarContactoDialog::AgregarContactoDialog(const QList<Usuario> &usuarios, QWidget *parent)
    : QDialog(parent), usuariosDisponibles(usuarios)
{
    setWindowTitle("Agregar Contacto");
    setFixedSize(360, 540);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titulo = new QLabel("Selecciona un contacto para agregar", this);
    titulo->setStyleSheet("font-size: 20px; font-weight: bold; margin-bottom: 10px;");
    titulo->setAlignment(Qt::AlignCenter);

    searchField = new QLineEdit(this);
    searchField->setPlaceholderText("Buscar nombre o usuario...");
    searchField->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 8px; margin-bottom: 8px;");

    userList = new QListWidget(this);
    userList->setStyleSheet(R"(
    QListWidget {
        border: 1px solid #ccc;
        border-radius: 8px;
        padding: 5px;
    }
    QListWidget::item {
        margin-bottom: 8px;
        background-color: transparent;
    }
    QListWidget::item:selected {
        background-color: transparent;
        color: black;
    }
)");

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setOffset(0, 5);
    setGraphicsEffect(shadow);

    addButton = new QPushButton("Agregar", this);
    addButton->setStyleSheet("padding: 10px; background-color: #800020; color: white; border-radius: 8px; font-weight: bold;");

    QPushButton *cancelButton = new QPushButton("Cancelar", this);
    cancelButton->setStyleSheet("padding: 10px; background-color: #aaa; color: white; border-radius: 8px; font-weight: bold;");

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(addButton);

    mainLayout->addWidget(titulo);
    mainLayout->addWidget(searchField);
    mainLayout->addWidget(userList);
    mainLayout->addLayout(buttonsLayout);

    populateUserList("");

    connect(searchField, &QLineEdit::textChanged, this, &AgregarContactoDialog::onSearchTextChanged);
    connect(addButton, &QPushButton::clicked, this, &AgregarContactoDialog::onAddClicked);
    connect(cancelButton, &QPushButton::clicked, this, &AgregarContactoDialog::reject);
    connect(userList, &QListWidget::itemSelectionChanged, this, &AgregarContactoDialog::updateSelectionStyle);
}

void AgregarContactoDialog::populateUserList(const QString &filter)
{
    userList->clear();
    for (const Usuario &u : usuariosDisponibles) {
        if (filter.isEmpty() || u.getNombre().contains(filter, Qt::CaseInsensitive) || u.getUsuario().contains(filter, Qt::CaseInsensitive)) {
            QWidget *itemWidget = new QWidget();
            QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
            itemLayout->setContentsMargins(8, 8, 8, 8);
            itemLayout->setSpacing(10);

            QLabel *avatarLabel = new QLabel();
            QPixmap avatarPixmap(u.getAvatar());
            if (!avatarPixmap.isNull()) {
                avatarPixmap = avatarPixmap.scaled(42, 42, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

                QPixmap circularAvatar(42, 42);
                circularAvatar.fill(Qt::transparent);

                QPainter painter(&circularAvatar);
                painter.setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.addEllipse(0, 0, 42, 42);
                painter.setClipPath(path);
                painter.drawPixmap(0, 0, avatarPixmap);

                avatarLabel->setPixmap(circularAvatar);
                avatarLabel->setFixedSize(42, 42);
            }

            QVBoxLayout *textLayout = new QVBoxLayout();
            textLayout->setContentsMargins(0, 0, 0, 0);
            textLayout->setSpacing(2);

            QLabel *usernameLabel = new QLabel(u.getUsuario());
            usernameLabel->setStyleSheet("font-weight: bold; font-size: 15px; color: black;");

            QLabel *nombreLabel = new QLabel(u.getNombre());
            nombreLabel->setStyleSheet("color: gray; font-size: 12px;");

            textLayout->addWidget(usernameLabel);
            textLayout->addWidget(nombreLabel);

            itemLayout->addWidget(avatarLabel);
            itemLayout->addLayout(textLayout);

            QListWidgetItem *item = new QListWidgetItem();
            item->setData(Qt::UserRole, u.getUsuario());
            userList->addItem(item);
            userList->setItemWidget(item, itemWidget);
            item->setSizeHint(QSize(0, 65));
        }
    }
}

void AgregarContactoDialog::onSearchTextChanged(const QString &text)
{
    populateUserList(text);
}

void AgregarContactoDialog::onAddClicked()
{
    if (userList->currentItem()) {
        usuarioSeleccionado = userList->currentItem()->data(Qt::UserRole).toString();
        accept();
    } else {
        reject();
    }
}

QString AgregarContactoDialog::getSeleccionado() const
{
    return usuarioSeleccionado;
}

void AgregarContactoDialog::updateSelectionStyle()
{
    for (int i = 0; i < userList->count(); ++i) {
        QListWidgetItem *item = userList->item(i);
        QWidget *widget = userList->itemWidget(item);

        if (item->isSelected()) {
            widget->setStyleSheet("background-color: #f0f0f0;");
        } else {
            widget->setStyleSheet("background-color: transparent;");
        }

        widget->setGraphicsEffect(nullptr);
    }
}
