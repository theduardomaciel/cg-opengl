#include "core/Application.h"
#include <iostream>

int main() {
    cg::AppConfig cfg; // pode customizar aqui futuramente
    cg::Application app(cfg);
    if (!app.init()) {
        std::cerr << "Falha ao inicializar aplicação" << std::endl;
        return -1;
    }
    app.run();
    return 0;
}