#include "sshterminal.h"
#include <QApplication>
#include <QScreen>

SSHTerminal::SSHTerminal(QWidget *parent)
    : QMainWindow(parent)
    , m_sshProcess(nullptr)
    , m_connected(false)
{
    setupUI();
    setupTerminal();

    // Window settings
    setWindowTitle("SSH Terminal - AppsQt5Utilitarios");
    setMinimumSize(800, 600);
    resize(1000, 700);

    // Center window on screen
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

SSHTerminal::~SSHTerminal()
{
    if (m_sshProcess && m_sshProcess->state() == QProcess::Running) {
        m_sshProcess->kill();
        m_sshProcess->waitForFinished(3000);
    }
}

void SSHTerminal::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    // Main layout with splitter
    m_mainSplitter = new QSplitter(Qt::Vertical, this);
    QVBoxLayout *mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->addWidget(m_mainSplitter);

    // Connection Group
    m_connectionGroup = new QGroupBox("Conexão SSH", this);
    m_connectionGroup->setMaximumHeight(150);

    QGridLayout *connLayout = new QGridLayout(m_connectionGroup);

    // IP/Host
    connLayout->addWidget(new QLabel("Host/IP:"), 0, 0);
    m_ipEdit = new QLineEdit(this);
    m_ipEdit->setPlaceholderText("192.168.1.100 ou servidor.com");
    connLayout->addWidget(m_ipEdit, 0, 1, 1, 2);

    // Username
    connLayout->addWidget(new QLabel("Usuário:"), 1, 0);
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("root");
    connLayout->addWidget(m_usernameEdit, 1, 1, 1, 2);

    // Password
    connLayout->addWidget(new QLabel("Senha:"), 2, 0);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Digite sua senha");
    connLayout->addWidget(m_passwordEdit, 2, 1, 1, 2);

    // Buttons
    m_connectButton = new QPushButton("Conectar", this);
    m_connectButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 14px; }");
    connLayout->addWidget(m_connectButton, 3, 0);

    m_disconnectButton = new QPushButton("Desconectar", this);
    m_disconnectButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; padding: 14px; }");
    m_disconnectButton->setEnabled(false);
    connLayout->addWidget(m_disconnectButton, 3, 1);

    // Status
    m_statusLabel = new QLabel("Desconectado", this);
    m_statusLabel->setStyleSheet("QLabel { color: #f44336; font-weight: bold; padding: 8px; }");
    connLayout->addWidget(m_statusLabel, 3, 2);

    m_mainSplitter->addWidget(m_connectionGroup);

    // Terminal Group
    m_terminalGroup = new QGroupBox("Terminal", this);
    QVBoxLayout *termLayout = new QVBoxLayout(m_terminalGroup);

    // Terminal output
    m_terminalOutput = new QTextEdit(this);
    m_terminalOutput->setReadOnly(true);
    m_terminalOutput->setStyleSheet(
        "QTextEdit { "
        "background-color: #1e1e1e; "
        "color: #ffffff; "
        "border: 1px solid #333; "
        "font-family: 'Courier New', monospace; "
        "font-size: 11pt; "
        "selection-background-color: #3390ff; "
        "}"
    );
    termLayout->addWidget(m_terminalOutput);

    // Command input area
    QHBoxLayout *cmdLayout = new QHBoxLayout();

    m_commandInput = new TerminalInput(this);
    m_commandInput->setPlaceholderText("Digite comando SSH aqui... (Enter para enviar)");
    m_commandInput->setStyleSheet(
        "QLineEdit { "
        "background-color: #2d2d2d; "
        "color: #ffffff; "
        "border: 1px solid #333; "
        "padding: 8px; "
        "font-family: 'Courier New', monospace; "
        "font-size: 11pt; "
        "}"
    );
    m_commandInput->setEnabled(false);
    cmdLayout->addWidget(m_commandInput);

    m_sendButton = new QPushButton("Enviar", this);
    m_sendButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px 16px; }");
    m_sendButton->setEnabled(false);
    cmdLayout->addWidget(m_sendButton);

    m_clearButton = new QPushButton("Limpar", this);
    m_clearButton->setStyleSheet("QPushButton { background-color: #607D8B; color: white; padding: 8px 16px; }");
    cmdLayout->addWidget(m_clearButton);

    termLayout->addLayout(cmdLayout);
    m_mainSplitter->addWidget(m_terminalGroup);

    // Set splitter proportions
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);

    // Connect signals
    connect(m_connectButton, &QPushButton::clicked, this, &SSHTerminal::connectToSSH);
    connect(m_disconnectButton, &QPushButton::clicked, this, &SSHTerminal::disconnectSSH);
    connect(m_sendButton, &QPushButton::clicked, this, &SSHTerminal::sendCommand);
    connect(m_clearButton, &QPushButton::clicked, this, &SSHTerminal::clearTerminal);
    connect(m_commandInput, &QLineEdit::returnPressed, this, &SSHTerminal::sendCommand);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &SSHTerminal::connectToSSH);

    // Terminal input special handling
    TerminalInput *termInput = qobject_cast<TerminalInput*>(m_commandInput);
    if (termInput) {
        connect(termInput, &TerminalInput::commandEntered, this, &SSHTerminal::executeCommand);
    }
}

void SSHTerminal::setupTerminal()
{
    m_terminalFont = QFont("Courier New", 11);
    m_terminalOutput->setFont(m_terminalFont);

    addToTerminal("=== SSH Terminal - AppsQt5Utilitarios ===", "#4CAF50");
    addToTerminal("Preencha os dados de conexão e clique em 'Conectar'", "#FFC107");
    addToTerminal("", "white");
}

void SSHTerminal::connectToSSH()
{
    QString host = m_ipEdit->text().trimmed();
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (host.isEmpty() || username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Dados Incompletos",
                           "Por favor, preencha todos os campos: Host/IP, Usuário e Senha.");
        return;
    }

    // Create SSH process
    m_sshProcess = new QProcess(this);

    // Connect process signals
    connect(m_sshProcess, &QProcess::readyReadStandardOutput, this, &SSHTerminal::onSSHOutput);
    connect(m_sshProcess, &QProcess::readyReadStandardError, this, &SSHTerminal::onSSHError);
    connect(m_sshProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SSHTerminal::onSSHFinished);

    // Prepare SSH command with sshpass for password authentication
    QStringList arguments;
    arguments << "-o" << "StrictHostKeyChecking=no"
              << "-o" << "UserKnownHostsFile=/dev/null"
              << QString("%1@%2").arg(username, host);

    addToTerminal(QString("Conectando em %1@%2...").arg(username, host), "#FFC107");

    // Use sshpass for password authentication
    m_sshProcess->setProgram("sshpass");
    QStringList sshpassArgs;
    sshpassArgs << "-p" << password << "ssh" << arguments;
    m_sshProcess->setArguments(sshpassArgs);

    m_sshProcess->start();

    if (!m_sshProcess->waitForStarted(5000)) {
        addToTerminal("ERRO: Falha ao iniciar processo SSH. Verifique se 'sshpass' está instalado.", "#f44336");
        addToTerminal("Para instalar: sudo apt-get install sshpass", "#FFC107");
        return;
    }

    m_currentHost = QString("%1@%2").arg(username, host);
    updateConnectionStatus(true);
}

void SSHTerminal::disconnectSSH()
{
    if (m_sshProcess && m_sshProcess->state() == QProcess::Running) {
        addToTerminal("Desconectando...", "#FFC107");
        m_sshProcess->write("exit\n");
        m_sshProcess->waitForFinished(3000);

        if (m_sshProcess->state() == QProcess::Running) {
            m_sshProcess->kill();
        }
    }
    updateConnectionStatus(false);
}

void SSHTerminal::sendCommand()
{
    if (!m_connected || !m_sshProcess) return;

    QString command = m_commandInput->text();
    if (command.isEmpty()) return;

    executeCommand(command);
    m_commandInput->clear();
}

void SSHTerminal::executeCommand(const QString &command)
{
    if (!m_connected || !m_sshProcess) return;

    // Add command to terminal display
    addToTerminal(QString("%1$ %2").arg(m_currentHost, command), "#4CAF50");

    // Send command to SSH process
    m_sshProcess->write(command.toUtf8() + "\n");

    // Special handling for exit command
    if (command.trimmed() == "exit" || command.trimmed() == "logout") {
        QTimer::singleShot(1000, this, &SSHTerminal::disconnectSSH);
    }
}

void SSHTerminal::onSSHOutput()
{
    if (!m_sshProcess) return;

    QByteArray data = m_sshProcess->readAllStandardOutput();
    QString output = QString::fromUtf8(data);

    // Clean up the output and add to terminal
    output.remove('\r');
    if (!output.isEmpty()) {
        addToTerminal(output, "white");
    }
}

void SSHTerminal::onSSHError()
{
    if (!m_sshProcess) return;

    QByteArray data = m_sshProcess->readAllStandardError();
    QString error = QString::fromUtf8(data);

    if (!error.isEmpty()) {
        addToTerminal("ERRO: " + error, "#f44336");
    }
}

void SSHTerminal::onSSHFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)

    if (exitStatus == QProcess::CrashExit) {
        addToTerminal("Conexão SSH encerrada inesperadamente", "#f44336");
    } else {
        addToTerminal("Conexão SSH encerrada", "#FFC107");
    }

    updateConnectionStatus(false);
}

void SSHTerminal::clearTerminal()
{
    m_terminalOutput->clear();
    setupTerminal();
}

void SSHTerminal::updateConnectionStatus(bool connected)
{
    m_connected = connected;

    m_connectButton->setEnabled(!connected);
    m_disconnectButton->setEnabled(connected);
    m_commandInput->setEnabled(connected);
    m_sendButton->setEnabled(connected);

    if (connected) {
        m_statusLabel->setText("Conectado");
        m_statusLabel->setStyleSheet(
            "QLabel { "
            "color: #4CAF50; "
            "font-weight: bold; "
            "padding: 12px 16px; "
            "min-height: 35px; "
            "font-size: 12pt; "
            "margin-top: -16px; "
            "border: 1px solid #4CAF50; "
            "border-radius: 4px; "
            "background-color: #E8F5E8; "
            "}"
        );
        m_commandInput->setFocus();
    } else {
        m_statusLabel->setText("Desconectado");
        m_statusLabel->setStyleSheet(
            "QLabel { "
            "color: #f44336; "
            "font-weight: bold; "
            "padding: 12px 16px; "
            "min-height: 35px; "
            "font-size: 12pt; "
            "margin-top: -16px; "
            "border: 1px solid #f44336; "
            "border-radius: 4px; "
            "background-color: #FFF3F3; "
            "}"
        );
        if (m_sshProcess) {
            m_sshProcess->deleteLater();
            m_sshProcess = nullptr;
        }
    }
}

void SSHTerminal::addToTerminal(const QString &text, const QString &color)
{
    QTextCursor cursor = m_terminalOutput->textCursor();
    cursor.movePosition(QTextCursor::End);

    cursor.insertHtml(QString("<span style='color: %1'>%2</span><br>")
                     .arg(color)
                     .arg(text.toHtmlEscaped()));

    m_terminalOutput->setTextCursor(cursor);
    m_terminalOutput->ensureCursorVisible();
}

void SSHTerminal::keyPressEvent(QKeyEvent *event)
{
    // Global shortcuts
    if (event->key() == Qt::Key_F5) {
        if (m_connected) {
            disconnectSSH();
        } else {
            connectToSSH();
        }
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void SSHTerminal::onTerminalKeyPress(QKeyEvent *event)
{
    // Handle special terminal keys if needed
    Q_UNUSED(event)
}

// TerminalInput Implementation
TerminalInput::TerminalInput(QWidget *parent)
    : QLineEdit(parent), m_historyIndex(-1)
{
}

void TerminalInput::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        if (!m_history.isEmpty() && m_historyIndex > 0) {
            m_historyIndex--;
            setText(m_history.at(m_historyIndex));
        }
        break;

    case Qt::Key_Down:
        if (!m_history.isEmpty()) {
            if (m_historyIndex < m_history.size() - 1) {
                m_historyIndex++;
                setText(m_history.at(m_historyIndex));
            } else {
                m_historyIndex = m_history.size();
                clear();
            }
        }
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        {
            QString cmd = text().trimmed();
            if (!cmd.isEmpty()) {
                m_history.append(cmd);
                m_historyIndex = m_history.size();
                emit commandEntered(cmd);
            }
        }
        break;

    default:
        QLineEdit::keyPressEvent(event);
        break;
    }
}

#include "sshterminal.moc"
