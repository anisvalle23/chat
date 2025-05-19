#include "ordenadorcontacto.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QRegularExpression>

QList<Usuario> OrdenadorContactos::ordenar(const QList<Usuario>& contactos, const QString& criterio, const QString& usuarioPrincipal) {
    QList<Usuario> copia = contactos;

    if (criterio == "alfabetico") {
        std::sort(copia.begin(), copia.end(), [](const Usuario& a, const Usuario& b) {
            return a.getUsuario().toLower() < b.getUsuario().toLower();
        });

    } else if (criterio == "longitud") {
        std::sort(copia.begin(), copia.end(), [&](const Usuario& a, const Usuario& b) {
            auto contarLineas = [&](const QString& contacto) {
                QString archivo1 = "Chats/" + usuarioPrincipal + "-" + contacto + ".txt";
                QString archivo2 = "Chats/" + contacto + "-" + usuarioPrincipal + ".txt";
                QString ruta = QFile::exists(archivo1) ? archivo1 : (QFile::exists(archivo2) ? archivo2 : archivo1);

                QFile file(ruta);
                int count = 0;
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    while (!in.atEnd()) {
                        in.readLine();
                        ++count;
                    }
                    file.close();
                }
                return count;
            };

            return contarLineas(b.getUsuario()) < contarLineas(a.getUsuario());
        });

    } else if (criterio == "fecha") {
        std::sort(copia.begin(), copia.end(), [&](const Usuario& a, const Usuario& b) {
            auto obtenerUltimaHora = [&](const QString& contacto) {
                QString archivo1 = "Chats/" + usuarioPrincipal + "-" + contacto + ".txt";
                QString archivo2 = "Chats/" + contacto + "-" + usuarioPrincipal + ".txt";
                QString ruta = QFile::exists(archivo1) ? archivo1 : (QFile::exists(archivo2) ? archivo2 : archivo1);

                QTime ultimaHora;
                QFile file(ruta);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    while (!in.atEnd()) {
                        QString linea = in.readLine().trimmed();
                        QRegularExpression re(R"(\[(\d{2}):(\d{2})\])");
                        QRegularExpressionMatch match = re.match(linea);
                        if (match.hasMatch()) {
                            int hora = match.captured(1).toInt();
                            int minuto = match.captured(2).toInt();
                            ultimaHora = QTime(hora, minuto);
                        }
                    }
                    file.close();
                }
                return ultimaHora;
            };

            return obtenerUltimaHora(b.getUsuario()) > obtenerUltimaHora(a.getUsuario());
        });

    } else if (criterio == "reciente_agregado") {
        std::reverse(copia.begin(), copia.end());

    } else if (criterio == "normal") {

    }

    return copia;
}
