#ifndef STYLEDCOMBOBOX_H
#define STYLEDCOMBOBOX_H

#include <QComboBox>
#include <QPainter>
#include <QStyleOptionComboBox>

class StyledComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit StyledComboBox(QWidget *parent = nullptr)
        : QComboBox(parent) {}

protected:
    void paintEvent(QPaintEvent *event) override {
        QStyleOptionComboBox opt;
        initStyleOption(&opt);
        QComboBox::paintEvent(event);
        QPainter p(this);
        p.setPen(Qt::black);
        p.drawText(rect().adjusted(12, 0, -30, 0), Qt::AlignVCenter | Qt::AlignLeft, currentText());
    }
};

#endif
