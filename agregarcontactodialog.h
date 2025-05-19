#ifndef AGREGARCONTACTODIALOG_H
#define AGREGARCONTACTODIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include "usuario.h"

class AgregarContactoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AgregarContactoDialog(const QList<Usuario> &usuarios, QWidget *parent = nullptr);

    QString getSeleccionado() const;

private slots:
    void onSearchTextChanged(const QString &text);
    void onAddClicked();
    void updateSelectionStyle();

private:
    void populateUserList(const QString &filter);

    QList<Usuario> usuariosDisponibles;
    QLineEdit *searchField;
    QListWidget *userList;
    QPushButton *addButton;
    QString usuarioSeleccionado;
};

#endif
