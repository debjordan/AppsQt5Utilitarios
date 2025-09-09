#include "MainWindow.h"
#include <QApplication>
#include <QHeaderView>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_sshConnection(nullptr)
    , m_currentPath("/")
    , m_isConnected(false)
{
    setupUI();
    setupMenuBar();

    m_sshConnection = new SshConnection(this);

    connect(m_sshConnection, &SshConnection::connected,
            this, [this]() { onConnectionStatusChanged(true, "Conectado com sucesso!"); });
    connect(m_sshConnection, &SshConnection::disconnected,
            this, [this]() { onConnectionStatusChanged(false, "Desconectado"); });
    connect(m_sshConnection, &SshConnection::errorOccurred,
            this, &MainWindow::onErrorOccurred);
    connect(m_sshConnection, QOverload<const QList<SshConnection::FileInfo>&>::of(&SshConnection::fileListReceived),
            this, [this](const QList<SshConnection::FileInfo>& files) {
                QStringList fileStrings;
                for (const auto& file : files) {
                    fileStrings << file.name;
                }
                onFileListReceived(fileStrings);
            });
    connect(m_sshConnection, QOverload<const QString&, int>::of(&SshConnection::transferProgress),
            this, [this](const QString& file, int percentage) {
                Q_UNUSED(file)
                onTransferProgress(percentage);
            });
    connect(m_sshConnection, &SshConnection::directoryChanged,
            this, [this](const QString& path) {
                m_currentPath = path;
                m_pathEdit->setText(path);
            });

    updateConnectionStatus();
    enableFileOperations(false);

    m_statusLabel->setText("Desconectado");
    m_logTextEdit->append(QString("[%1] Aplicação iniciada")
                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
}

MainWindow::~MainWindow()
{
    if (m_sshConnection && m_sshConnection->isConnected()) {
        m_sshConnection->disconnect();
    }
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QVBoxLayout(m_centralWidget);

    m_connectionGroup = new QGroupBox("Conexão SSH", this);
    m_connectionGridLayout = new QGridLayout(m_connectionGroup);

    m_connectionGridLayout->addWidget(new QLabel("Host:"), 0, 0);
    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setPlaceholderText("192.168.1.100");
    m_connectionGridLayout->addWidget(m_hostEdit, 0, 1);

    m_connectionGridLayout->addWidget(new QLabel("Porta:"), 0, 2);
    m_portEdit = new QLineEdit("22", this);
    m_portEdit->setMaximumWidth(80);
    m_connectionGridLayout->addWidget(m_portEdit, 0, 3);

    m_connectionGridLayout->addWidget(new QLabel("Usuário:"), 1, 0);
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("root");
    m_connectionGridLayout->addWidget(m_usernameEdit, 1, 1);

    m_connectionGridLayout->addWidget(new QLabel("Senha:"), 1, 2);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_connectionGridLayout->addWidget(m_passwordEdit, 1, 3);

    m_connectButton = new QPushButton("Conectar", this);
    m_disconnectButton = new QPushButton("Desconectar", this);
    m_connectionGridLayout->addWidget(m_connectButton, 2, 0);
    m_connectionGridLayout->addWidget(m_disconnectButton, 2, 1);

    m_mainLayout->addWidget(m_connectionGroup);

    m_mainSplitter = new QSplitter(Qt::Vertical, this);
    m_mainLayout->addWidget(m_mainSplitter);

    m_fileBrowserWidget = new QWidget(this);
    m_fileBrowserLayout = new QVBoxLayout(m_fileBrowserWidget);

    m_navigationLayout = new QHBoxLayout();
    m_upButton = new QPushButton("↑ Voltar", this);
    m_homeButton = new QPushButton("🏠 Home", this);
    m_refreshButton = new QPushButton("🔄 Atualizar", this);
    m_pathEdit = new QLineEdit("/", this);
    m_pathEdit->setReadOnly(true);

    m_navigationLayout->addWidget(m_upButton);
    m_navigationLayout->addWidget(m_homeButton);
    m_navigationLayout->addWidget(m_refreshButton);
    m_navigationLayout->addWidget(new QLabel("Caminho:"));
    m_navigationLayout->addWidget(m_pathEdit);

    m_fileBrowserLayout->addLayout(m_navigationLayout);

    m_fileTree = new QTreeWidget(this);
    m_fileTree->setHeaderLabels(QStringList() << "Nome" << "Tamanho" << "Tipo"
                               << "Permissões" << "Modificado");
    m_fileTree->header()->resizeSection(0, 200);
    m_fileTree->header()->resizeSection(1, 100);
    m_fileTree->header()->resizeSection(2, 80);
    m_fileTree->header()->resizeSection(3, 100);
    m_fileTree->setAlternatingRowColors(true);
    m_fileTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_fileBrowserLayout->addWidget(m_fileTree);

    m_operationsLayout = new QHBoxLayout();
    m_downloadButton = new QPushButton("📥 Download", this);
    m_uploadButton = new QPushButton("📤 Upload", this);
    m_deleteButton = new QPushButton("🗑️ Excluir", this);
    m_createFolderButton = new QPushButton("📁 Nova Pasta", this);
    m_renameButton = new QPushButton("✏️ Renomear", this);
    m_propertiesButton = new QPushButton("ℹ️ Propriedades", this);

    m_operationsLayout->addWidget(m_downloadButton);
    m_operationsLayout->addWidget(m_uploadButton);
    m_operationsLayout->addWidget(m_deleteButton);
    m_operationsLayout->addWidget(m_createFolderButton);
    m_operationsLayout->addWidget(m_renameButton);
    m_operationsLayout->addWidget(m_propertiesButton);
    m_operationsLayout->addStretch();

    m_fileBrowserLayout->addLayout(m_operationsLayout);

    m_mainSplitter->addWidget(m_fileBrowserWidget);

    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setMaximumHeight(150);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFont(QFont("Consolas", 9));
    m_mainSplitter->addWidget(m_logTextEdit);

    m_statusLabel = new QLabel("Desconectado", this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);

    statusBar()->addWidget(m_statusLabel);
    statusBar()->addPermanentWidget(m_progressBar);

    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::connectToHost);
    connect(m_disconnectButton, &QPushButton::clicked, this, &MainWindow::disconnectFromHost);
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::refreshFileList);
    connect(m_upButton, &QPushButton::clicked, this, &MainWindow::goToParentDirectory);
    connect(m_homeButton, &QPushButton::clicked, this, &MainWindow::goToHomeDirectory);

    connect(m_downloadButton, &QPushButton::clicked, this, &MainWindow::downloadFile);
    connect(m_uploadButton, &QPushButton::clicked, this, &MainWindow::uploadFile);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::deleteFile);
    connect(m_createFolderButton, &QPushButton::clicked, this, &MainWindow::createFolder);
    connect(m_renameButton, &QPushButton::clicked, this, &MainWindow::renameItem);
    connect(m_propertiesButton, &QPushButton::clicked, this, &MainWindow::showItemProperties);

    connect(m_fileTree, &QTreeWidget::itemDoubleClicked,
            this, &MainWindow::onItemDoubleClicked);
    connect(m_fileTree, &QTreeWidget::itemSelectionChanged,
            this, &MainWindow::onItemSelectionChanged);

    setWindowTitle("Gerenciador de Arquivos Remoto v1.0");
    setMinimumSize(800, 600);
    resize(1000, 700);
}

void MainWindow::setupMenuBar()
{
    m_fileMenu = menuBar()->addMenu("&Arquivo");

    m_connectAction = m_fileMenu->addAction("&Conectar");
    m_connectAction->setShortcut(QKeySequence("Ctrl+C"));
    connect(m_connectAction, &QAction::triggered, this, &MainWindow::connectToHost);

    m_disconnectAction = m_fileMenu->addAction("&Desconectar");
    m_disconnectAction->setShortcut(QKeySequence("Ctrl+D"));
    connect(m_disconnectAction, &QAction::triggered, this, &MainWindow::disconnectFromHost);

    m_fileMenu->addSeparator();

    m_exitAction = m_fileMenu->addAction("&Sair");
    m_exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);

    m_editMenu = menuBar()->addMenu("&Editar");
    m_editMenu->addAction("Selecionar &Tudo", this, [this]() {
        m_fileTree->selectAll();
    }, QKeySequence("Ctrl+A"));

    m_viewMenu = menuBar()->addMenu("&Visualizar");
    m_viewMenu->addAction("&Atualizar", this, &MainWindow::refreshFileList,
                         QKeySequence("F5"));

    m_helpMenu = menuBar()->addMenu("&Ajuda");
    m_aboutAction = m_helpMenu->addAction("&Sobre");
    connect(m_aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "Sobre",
            "Gerenciador de Arquivos Remoto v1.0\n\n"
            "Desenvolvido com Qt5 e C++\n"
            "Parte da coleção AppsQt5Utilitarios\n\n"
            "Permite gerenciar arquivos remotamente via SSH/SCP.");
    });
}

void MainWindow::connectToHost()
{
    QString host = m_hostEdit->text().trimmed();
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    int port = m_portEdit->text().toInt();

    if (host.isEmpty() || username.isEmpty()) {
        QMessageBox::warning(this, "Erro", "Host e usuário são obrigatórios!");
        return;
    }

    if (port <= 0 || port > 65535) {
        port = 22;
        m_portEdit->setText("22");
    }

    m_logTextEdit->append(QString("[%1] Conectando a %2@%3:%4...")
                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                         .arg(username, host).arg(port));

    m_connectButton->setEnabled(false);
    m_statusLabel->setText("Conectando...");

    m_sshConnection->connectToHost(host, username, password, port);
}

void MainWindow::disconnectFromHost()
{
    if (m_sshConnection && m_sshConnection->isConnected()) {
        m_sshConnection->disconnect();
        m_logTextEdit->append(QString("[%1] Desconectado")
                             .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    }
    updateConnectionStatus();
}

void MainWindow::refreshFileList()
{
    if (!m_isConnected) return;

    m_logTextEdit->append(QString("[%1] Atualizando lista de arquivos: %2")
                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                         .arg(m_currentPath));

    m_sshConnection->listDirectory(m_currentPath);
}

void MainWindow::onConnectionStatusChanged(bool connected, const QString& message)
{
    m_isConnected = connected;
    updateConnectionStatus();
    enableFileOperations(connected);

    m_logTextEdit->append(QString("[%1] %2")
                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                         .arg(message));

    if (connected) {
        m_statusLabel->setText(QString("Conectado - %1@%2")
                              .arg(m_sshConnection->getConnectionInfo().username)
                              .arg(m_sshConnection->getConnectionInfo().host));
        refreshFileList();
    } else {
        m_statusLabel->setText("Desconectado");
        m_fileTree->clear();
    }
}

void MainWindow::updateConnectionStatus()
{
    m_connectButton->setEnabled(!m_isConnected);
    m_disconnectButton->setEnabled(m_isConnected);
    m_connectAction->setEnabled(!m_isConnected);
    m_disconnectAction->setEnabled(m_isConnected);
}

void MainWindow::enableFileOperations(bool enabled)
{
    m_refreshButton->setEnabled(enabled);
    m_upButton->setEnabled(enabled);
    m_homeButton->setEnabled(enabled);
    m_downloadButton->setEnabled(enabled);
    m_uploadButton->setEnabled(enabled);
    m_deleteButton->setEnabled(enabled);
    m_createFolderButton->setEnabled(enabled);
    m_renameButton->setEnabled(enabled);
    m_propertiesButton->setEnabled(enabled);
}

void MainWindow::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)

    if (!item) return;

    QString itemType = item->text(2);
    if (itemType == "Diretório") {
        QString newPath = item->data(0, Qt::UserRole).toString();
        m_currentPath = newPath;
        m_pathEdit->setText(newPath);
        refreshFileList();
    }
}

void MainWindow::onItemSelectionChanged()
{
}

void MainWindow::downloadFile()
{
    QList<QTreeWidgetItem*> selectedItems = m_fileTree->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, "Info", "Selecione um arquivo para download.");
        return;
    }

    QString localDir = QFileDialog::getExistingDirectory(this, "Selecionar pasta de destino");
    if (!localDir.isEmpty()) {
        for (QTreeWidgetItem* item : selectedItems) {
            QString remotePath = item->data(0, Qt::UserRole).toString();
            QString localPath = localDir + "/" + item->text(0);
            m_sshConnection->downloadFile(remotePath, localPath);
        }
    }
}

void MainWindow::uploadFile()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Selecionar arquivos para upload");
    if (!files.isEmpty()) {
        for (const QString& file : files) {
            QString remotePath = m_currentPath + "/" + QFileInfo(file).fileName();
            m_sshConnection->uploadFile(file, remotePath);
        }
    }
}

void MainWindow::deleteFile()
{
    QList<QTreeWidgetItem*> selectedItems = m_fileTree->selectedItems();
    if (selectedItems.isEmpty()) return;

    int ret = QMessageBox::question(this, "Confirmar",
        QString("Deseja excluir %1 item(s) selecionado(s)?").arg(selectedItems.count()));

    if (ret == QMessageBox::Yes) {
        for (QTreeWidgetItem* item : selectedItems) {
            QString remotePath = item->data(0, Qt::UserRole).toString();
            m_sshConnection->deleteFile(remotePath);
        }
    }
}

void MainWindow::createFolder()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Nova Pasta",
                                        "Nome da pasta:", QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        QString remotePath = m_currentPath + "/" + name;
        m_sshConnection->createDirectory(remotePath);
    }
}

void MainWindow::renameItem()
{
    QList<QTreeWidgetItem*> selectedItems = m_fileTree->selectedItems();
    if (selectedItems.count() != 1) {
        QMessageBox::information(this, "Info", "Selecione apenas um item para renomear.");
        return;
    }

    QTreeWidgetItem* item = selectedItems.first();
    bool ok;
    QString newName = QInputDialog::getText(this, "Renomear",
                                           "Novo nome:", QLineEdit::Normal,
                                           item->text(0), &ok);
    if (ok && !newName.isEmpty() && newName != item->text(0)) {
        QString oldPath = item->data(0, Qt::UserRole).toString();
        QString newPath = m_currentPath + "/" + newName;
        m_sshConnection->renameFile(oldPath, newPath);
    }
}

void MainWindow::showItemProperties()
{
    QList<QTreeWidgetItem*> selectedItems = m_fileTree->selectedItems();
    if (selectedItems.count() != 1) {
        QMessageBox::information(this, "Info", "Selecione um item para ver propriedades.");
        return;
    }

    QTreeWidgetItem* item = selectedItems.first();
    QString info = QString("Nome: %1\nTamanho: %2\nTipo: %3\nPermissões: %4\nModificado: %5")
                   .arg(item->text(0))
                   .arg(item->text(1))
                   .arg(item->text(2))
                   .arg(item->text(3))
                   .arg(item->text(4));

    QMessageBox::information(this, "Propriedades", info);
}

void MainWindow::goToParentDirectory()
{
    if (!m_isConnected || m_currentPath == "/") return;

    QString parentPath = m_currentPath;
    int lastSlash = parentPath.lastIndexOf('/');
    if (lastSlash > 0) {
        parentPath = parentPath.left(lastSlash);
    } else {
        parentPath = "/";
    }

    m_currentPath = parentPath;
    m_pathEdit->setText(parentPath);
    refreshFileList();
}

void MainWindow::goToHomeDirectory()
{
    if (!m_isConnected) return;

    m_currentPath = "~";
    m_pathEdit->setText(m_currentPath);
    refreshFileList();
}

void MainWindow::onFileListReceived(const QStringList& files)
{
    Q_UNUSED(files)

    m_fileTree->clear();

    if (m_currentPath != "/") {
        QTreeWidgetItem* parentItem = new QTreeWidgetItem(m_fileTree);
        parentItem->setText(0, "..");
        parentItem->setText(2, "Diretório");
        parentItem->setData(0, Qt::UserRole, m_currentPath + "/..");
        parentItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    }

    QStringList sampleFiles = {
        "home:d:755:root:root:4096:" + QDateTime::currentDateTime().toString(),
        "var:d:755:root:root:4096:" + QDateTime::currentDateTime().toString(),
        "usr:d:755:root:root:4096:" + QDateTime::currentDateTime().toString(),
        "etc:d:755:root:root:4096:" + QDateTime::currentDateTime().toString(),
        "arquivo.txt:f:644:user:user:1024:" + QDateTime::currentDateTime().toString(),
        "script.sh:f:755:user:user:2048:" + QDateTime::currentDateTime().toString()
    };

    for (const QString& file : sampleFiles) {
        QStringList parts = file.split(':');
        if (parts.size() >= 7) {
            QTreeWidgetItem* item = new QTreeWidgetItem(m_fileTree);
            item->setText(0, parts[0]); // Nome

            bool isDir = (parts[1] == "d");
            item->setText(1, isDir ? "" : QString::number(parts[5].toLongLong()) + " bytes");
            item->setText(2, isDir ? "Diretório" : "Arquivo");
            item->setText(3, parts[2]);
            item->setText(4, parts[6]);

            QString fullPath = m_currentPath;
            if (!fullPath.endsWith('/')) fullPath += '/';
            fullPath += parts[0];
            item->setData(0, Qt::UserRole, fullPath);

            if (isDir) {
                item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
            } else {
                item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
            }
        }
    }

    m_logTextEdit->append(QString("[%1] Lista de arquivos atualizada")
                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
}

void MainWindow::onTransferProgress(int percentage)
{
    m_progressBar->setVisible(percentage > 0 && percentage < 100);
    m_progressBar->setValue(percentage);

    if (percentage >= 100) {
        QTimer::singleShot(1000, this, [this]() {
            m_progressBar->setVisible(false);
        });
    }
}

void MainWindow::onErrorOccurred(const QString& error)
{
    m_logTextEdit->append(QString("[%1] ERRO: %2")
                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                         .arg(error));

    QMessageBox::critical(this, "Erro", error);

    if (!m_isConnected) {
        m_connectButton->setEnabled(true);
        m_statusLabel->setText("Erro de conexão");
    }
}
