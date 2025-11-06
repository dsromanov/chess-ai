#include <QApplication>
#include "ui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Установка настроек приложения
    app.setApplicationName("Chess AI");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("ChessAI");
    
    Chess::UI::MainWindow window;
    window.show();
    
    return app.exec();
}

