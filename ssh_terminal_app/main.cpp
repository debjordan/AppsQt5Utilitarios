#include <QApplication>
#include "sshterminal.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("SSH Terminal");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("AppsQt5Utilitarios");

    SSHTerminal window;
    window.show();

    return app.exec();
}
