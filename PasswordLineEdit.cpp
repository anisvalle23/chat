#include "PasswordLineEdit.h"

PasswordLineEdit::PasswordLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setEchoMode(QLineEdit::Password);
}

void PasswordLineEdit::enterEvent(QEnterEvent *event)
{
    setEchoMode(QLineEdit::Normal);
    QLineEdit::enterEvent(event);
}

void PasswordLineEdit::leaveEvent(QEvent *event)
{
    setEchoMode(QLineEdit::Password);
    QLineEdit::leaveEvent(event);
}
