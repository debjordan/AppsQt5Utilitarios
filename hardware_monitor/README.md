# Hardware Monitor

Monitor simples de hardware para Linux em C++ com Qt5.

## O que faz

- Mostra modelo do processador e tamanho da RAM
- Monitora uso de CPU e RAM em tempo real
- Interface gráfica com barras de progresso
- Atualização a cada segundo

## Instalação

```bash
sudo apt install build-essential qtbase5-dev cmake
git clone https://github.com/debjordan/AppsQt5Utilitarios
cd hardware-monitor
mkdir build && cd build
cmake .. && make
./HardwareMonitor
```

## Requisitos

- Debian/Ubuntu
- Qt5
- CMake

## Arquivos

- `main.cpp` - Entrada da aplicação
- `mainwindow.*` - Interface gráfica
- `systeminfo.*` - Coleta dados do sistema via /proc/

---

Ferramenta leve e eficiente para monitorar seu sistema Linux.
