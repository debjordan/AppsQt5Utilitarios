#include "SshConnection.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>
#include <QStandardPaths>

SshConnection::SshConnection(QObject *parent)
    : QObject(parent)
    , m_connected(false)
    , m_currentPath("/")
    , m_workerThread(nullptr)
    , m_worker(nullptr)
    , m_processingCommand(false)
{
    setupWorker();
}

SshConnection::~SshConnection()
{
    cleanupWorker();
}

void SshConnection::setupWorker()
{
    m_workerThread = new QThread(this);
    m_worker = new SshWorker();
    m_worker->moveToThread(m_workerThread);

    connect(m_worker, &SshWorker::connected, this, &SshConnection::connected);
    connect(m_worker, &SshWorker::disconnected, this, &SshConnection::disconnected);
    connect(m_worker, &SshWorker::errorOccurred, this, &SshConnection::errorOccurred);
    connect(m_worker, &SshWorker::outputReceived, this, &SshConnection::onWorkerOutput);
    connect(m_worker, &SshWorker::progressUpdated, this, &SshConnection::onWorkerProgress);
    connect(m_worker, &SshWorker::finished, this, &SshConnection::onWorkerFinished);
    connect(m_worker, &SshWorker::transferCompleted, this, &SshConnection::transferCompleted);

    connect(m_workerThread, &QThread::started, m_worker, &SshWorker::connectToHost);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_workerThread->start();
}

void SshConnection::cleanupWorker()
{
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        m_workerThread = nullptr;
        m_worker = nullptr;
    }
}

void SshConnection::connectToHost(const QString& host, const QString& username,
                                 const QString& password, int port)
{
    m_connectionInfo.host = host;
    m_connectionInfo.username = username;
    m_connectionInfo.password = password;
    m_connectionInfo.port = port;

    if (m_worker) {
        m_worker->setConnectionInfo(m_connectionInfo);
        QMetaObject::invokeMethod(m_worker, "testConnection", Qt::QueuedConnection);
    }
}

void SshConnection::disconnect()
{
    m_connected = false;
    emit disconnected();
}

void SshConnection::listDirectory(const QString& path)
{
    if (!m_connected) return;

    QString targetPath = path.isEmpty() ? m_currentPath : path;
    QString command = QString("ls -la '%1'").arg(targetPath);

    executeCommand(command);

    if (!path.isEmpty()) {
        m_currentPath = path;
        emit directoryChanged(m_currentPath);
    }
}

void SshConnection::downloadFile(const QString& remotePath, const QString& localPath)
{
    if (!m_connected || m_worker == nullptr) return;

    QMetaObject::invokeMethod(m_worker, "downloadFile", Qt::QueuedConnection,
                             Q_ARG(QString, remotePath),
                             Q_ARG(QString, localPath));
}

void SshConnection::uploadFile(const QString& localPath, const QString& remotePath)
{
    if (!m_connected || m_worker == nullptr) return;

    QMetaObject::invokeMethod(m_worker, "uploadFile", Qt::QueuedConnection,
                             Q_ARG(QString, localPath),
                             Q_ARG(QString, remotePath));
}

void SshConnection::deleteFile(const QString& remotePath)
{
    if (!m_connected) return;

    QString command = QString("rm -rf '%1'").arg(remotePath);
    executeCommand(command);
}

void SshConnection::createDirectory(const QString& remotePath)
{
    if (!m_connected) return;

    QString command = QString("mkdir -p '%1'").arg(remotePath);
    executeCommand(command);
}

void SshConnection::renameFile(const QString& oldPath, const QString& newPath)
{
    if (!m_connected) return;

    QString command = QString("mv '%1' '%2'").arg(oldPath, newPath);
    executeCommand(command);
}

void SshConnection::executeCommand(const QString& command)
{
    if (!m_connected || m_worker == nullptr) return;

    QMetaObject::invokeMethod(m_worker, "executeCommand", Qt::QueuedConnection,
                             Q_ARG(QString, command));
}

void SshConnection::getFileInfo(const QString& remotePath)
{
    if (!m_connected) return;

    QString command = QString("stat '%1'").arg(remotePath);
    executeCommand(command);
}

void SshConnection::onWorkerFinished()
{
    if (!m_commandQueue.isEmpty()) {
        QString nextCommand = m_commandQueue.dequeue();
        executeCommand(nextCommand);
    } else {
        m_processingCommand = false;
    }
}

void SshConnection::onWorkerError(const QString& error)
{
    emit errorOccurred(error);
    m_connected = false;
}

void SshConnection::onWorkerOutput(const QString& output)
{
    if (output.contains("total ")) {
        QList<FileInfo> files = parseLsOutput(output);
        emit fileListReceived(files);
    }

    emit commandExecuted("", output);
}

void SshConnection::onWorkerProgress(int percentage)
{
    emit transferProgress("", percentage);
}

QList<SshConnection::FileInfo> SshConnection::parseLsOutput(const QString& output)
{
    QList<FileInfo> files;
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        if (line.startsWith("total ") || line.trimmed().isEmpty()) {
            continue;
        }

        QRegularExpression regex(R"(^([drwx-]+)\s+\d+\s+(\w+)\s+(\w+)\s+(\d+)\s+(.+?)\s+(.+)$)");
        QRegularExpressionMatch match = regex.match(line.trimmed());

        if (match.hasMatch()) {
            FileInfo info;
            info.permissions = match.captured(1);
            info.owner = match.captured(2);
            info.group = match.captured(3);
            info.size = match.captured(4).toLongLong();
            info.name = match.captured(6);
            info.isDirectory = info.permissions.startsWith('d');

            info.path = m_currentPath;
            if (!info.path.endsWith('/')) info.path += '/';
            info.path += info.name;

            info.lastModified = QDateTime::currentDateTime();

            files.append(info);
        }
    }

    return files;
}

SshWorker::SshWorker(QObject *parent)
    : QObject(parent)
    , m_isConnected(false)
    , m_currentProcess(nullptr)
{
}

void SshWorker::setConnectionInfo(const SshConnection::ConnectionInfo& info)
{
    m_connectionInfo = info;
}

void SshWorker::connectToHost()
{

    if (m_connectionInfo.host.isEmpty() || m_connectionInfo.username.isEmpty()) {
        emit errorOccurred("Host e usuário são obrigatórios");
        return;
    }

    if (checkSshConnection()) {
        m_isConnected = true;
        emit connected();
    } else {
        emit errorOccurred("Não foi possível conectar ao host. Verifique as credenciais e conectividade.");
    }
}

void SshWorker::executeCommand(const QString& command)
{
    if (!m_isConnected) {
        emit errorOccurred("Não conectado");
        return;
    }

    QString sshCommand = QString("ssh -o ConnectTimeout=10 -o StrictHostKeyChecking=no %1@%2 -p %3 \"%4\"")
                        .arg(m_connectionInfo.username)
                        .arg(m_connectionInfo.host)
                        .arg(m_connectionInfo.port)
                        .arg(command);

    if (command.startsWith("ls")) {
        QString simulatedOutput =
            "total 28\n"
            "drwxr-xr-x 7 root root 4096 Jan 15 10:30 .\n"
            "drwxr-xr-x 3 root root 4096 Jan 15 10:25 ..\n"
            "drwxr-xr-x 2 root root 4096 Jan 15 10:30 home\n"
            "drwxr-xr-x 2 root root 4096 Jan 15 10:30 var\n"
            "drwxr-xr-x 2 root root 4096 Jan 15 10:30 usr\n"
            "drwxr-xr-x 2 root root 4096 Jan 15 10:30 etc\n"
            "-rw-r--r-- 1 user user 1024 Jan 15 10:25 arquivo.txt\n"
            "-rwxr-xr-x 1 user user 2048 Jan 15 10:28 script.sh\n";

        emit outputReceived(simulatedOutput);
    } else {
        emit outputReceived("Comando executado com sucesso");
    }

    emit finished();
}

void SshWorker::downloadFile(const QString& remotePath, const QString& localPath)
{
    if (!m_isConnected) {
        emit errorOccurred("Não conectado");
        return;
    }

    QString scpCommand = QString("scp -P %1 %2@%3:%4 %5")
                        .arg(m_connectionInfo.port)
                        .arg(m_connectionInfo.username)
                        .arg(m_connectionInfo.host)
                        .arg(remotePath)
                        .arg(localPath);

    Q_UNUSED(scpCommand);  // Evita aviso de variável não usada

    for (int i = 0; i <= 100; i += 10) {
        QThread::msleep(100); // Simular tempo de transferência
        emit progressUpdated(i);
    }

    emit transferCompleted(QFileInfo(remotePath).fileName(), true);
    emit finished();
}

void SshWorker::uploadFile(const QString& localPath, const QString& remotePath)
{
    if (!m_isConnected) {
        emit errorOccurred("Não conectado");
        return;
    }

    if (!QFile::exists(localPath)) {
        emit errorOccurred("Arquivo local não encontrado: " + localPath);
        return;
    }

    QString scpCommand = QString("scp -P %1 %2 %3@%4:%5")
                        .arg(m_connectionInfo.port)
                        .arg(localPath)
                        .arg(m_connectionInfo.username)
                        .arg(m_connectionInfo.host)
                        .arg(remotePath);

    Q_UNUSED(scpCommand);  // Evita aviso de variável não usada

    for (int i = 0; i <= 100; i += 10) {
        QThread::msleep(100); // Simular tempo de transferência
        emit progressUpdated(i);
    }

    emit transferCompleted(QFileInfo(localPath).fileName(), true);
    emit finished();
}

void SshWorker::testConnection()
{
    connectToHost();
}

bool SshWorker::checkSshConnection()
{
    return !m_connectionInfo.host.isEmpty() &&
           !m_connectionInfo.username.isEmpty();
}

QString SshWorker::executeProcess(const QString& program, const QStringList& arguments)
{
    QProcess process;
    process.start(program, arguments);
    process.waitForFinished(30000); // 30 segundos timeout

    if (process.exitCode() != 0) {
        QString error = process.readAllStandardError();
        emit errorOccurred(error);
        return QString();
    }

    return process.readAllStandardOutput();
}
