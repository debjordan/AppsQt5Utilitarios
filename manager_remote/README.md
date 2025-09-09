# Gerenciador de Arquivos Remoto

## Descrição
O **Gerenciador de Arquivos Remoto** é uma aplicação Qt5 desenvolvida em C++ que permite gerenciar arquivos em um servidor remoto via SSH/SCP. Com uma interface gráfica intuitiva, os usuários podem se conectar a servidores remotos, listar diretórios, fazer upload e download de arquivos, criar pastas, renomear e excluir arquivos, além de visualizar propriedades de arquivos. A aplicação utiliza o protocolo SSH para comunicação segura e suporta operações básicas de gerenciamento de arquivos em um ambiente remoto.

## Funcionalidades
- Conexão segura a servidores via SSH
- Listagem de arquivos e diretórios remotos
- Upload e download de arquivos via SCP
- Criação, exclusão e renomeação de arquivos e diretórios
- Visualização de propriedades de arquivos (nome, tamanho, tipo, permissões, data de modificação)
- Interface com tema escuro moderno usando o estilo "Fusion" do Qt
- Log de atividades e barra de progresso para transferências de arquivos

## Pré-requisitos
Para compilar e executar o projeto, você precisa dos seguintes pacotes instalados:
- **Qt5**: Biblioteca Qt5 com os módulos Core, Widgets e Network
- **CMake**: Versão 3.16 ou superior
- **Compilador C++**: Suporte para C++17 (GCC, Clang ou similar)
- **Ferramentas SSH/SCP**: Para operações reais de conexão e transferência (a versão atual simula essas operações)

### Instalação de Dependências (Debian/Ubuntu)
```bash
sudo apt-get update
sudo apt-get install qt5-default qtbase5-dev qtbase5-dev-tools cmake g++ openssh-client
```

## Uso
1. **Iniciar a Aplicação**:
   - Execute o binário compilado (`manager_remote`).
   - A interface gráfica será exibida com um painel de conexão SSH.

2. **Conectar a um Servidor**:
   - Insira o endereço do host (ex.: `192.168.1.100`), nome de usuário, senha e porta (padrão: 22).
   - Clique em "Conectar" para estabelecer a conexão SSH.
   - O status da conexão será exibido na barra de status.

3. **Gerenciar Arquivos**:
   - Navegue pelos diretórios usando o botão "↑ Voltar" ou clicando duas vezes em diretórios.
   - Use os botões "📥 Download", "📤 Upload", "🗑️ Excluir", "📁 Nova Pasta" e "✏️ Renomear" para realizar operações.
   - Visualize propriedades de arquivos com o botão "ℹ️ Propriedades".

4. **Monitoramento**:
   - O log de atividades é exibido na parte inferior da janela.
   - A barra de progresso mostra o andamento de transferências de arquivos.

## Estrutura do Projeto
- **`main.cpp`**: Ponto de entrada da aplicação, configura o tema e inicializa a janela principal.
- **`MainWindow.cpp/h`**: Implementa a interface gráfica principal e a lógica de interação com o usuário.
- **`SshConnection.cpp/h`**: Gerencia conexões SSH e operações de arquivos (atualmente simula operações SSH/SCP).
- **`CMakeLists.txt`**: Arquivo de configuração do CMake para compilação do projeto.
