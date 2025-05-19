#include <QMetaType>
#include <QMap>
#include <QApplication>
#include "loginwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 🔧 Registra tipo personalizado usado en la señal
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QString>");

    LoginWindow w;
    w.show();
    return app.exec();
}
