#ifndef SSHCONNECTION_H
#define SSHCONNECTION_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QStringList>
#include <QDateTime>

class SshWorker;

class SshConnection : public QObject
{
    Q_OBJECT

public:
    explicit SshConnection(QObject *parent = nullptr);
    ~SshConnection();

    struct ConnectionInfo {
        QString host;
        QString username;
        QString password;
        int port;
    };

    struct FileInfo {
        QString name;
        QString path;
        QString permissions;
        QString owner;
        QString group;
        qint64 size;
        QDateTime lastModified;
        bool isDirectory;
    };

    bool isConnected() const { return m_connected; }
    const ConnectionInfo& getConnectionInfo() const { return m_connectionInfo; }
    QString getCurrentPath() const { return m_currentPath; }

public slots:
    void connectToHost(const QString& host, const QString& username,
                      const QString& password, int port = 22);
    void disconnect();
    void listDirectory(const QString& path = QString());
    void downloadFile(const QString& remotePath, const QString& localPath);
    void uploadFile(const QString& localPath, const QString& remotePath);
    void deleteFile(const QString& remotePath);
    void createDirectory(const QString& remotePath);
    void renameFile(const QString& oldPath, const QString& newPath);
    void executeCommand(const QString& command);
    void getFileInfo(const QString& remotePath);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void commandExecuted(const QString& command, const QString& output);
    void fileListReceived(const QList<SshConnection::FileInfo>& files);
    void transferProgress(const QString& file, int percentage);
    void transferCompleted(const QString& file, bool success);
    void directoryChanged(const QString& path);

private slots:
    void onWorkerFinished();
    void onWorkerError(const QString& error);
    void onWorkerOutput(const QString& output);
    void onWorkerProgress(int percentage);

private:
    void setupWorker();
    void cleanupWorker();
    QString formatSshCommand(const QString& command) const;
    QString formatScpCommand(const QString& source, const QString& destination, bool upload) const;
    QList<FileInfo> parseLsOutput(const QString& output);

    bool m_connected;
    ConnectionInfo m_connectionInfo;
    QString m_currentPath;

    QThread* m_workerThread;
    SshWorker* m_worker;
    QMutex m_mutex;

    QQueue<QString> m_commandQueue;
    bool m_processingCommand;
};

class SshWorker : public QObject
{
    Q_OBJECT

public:
    explicit SshWorker(QObject *parent = nullptr);

    void setConnectionInfo(const SshConnection::ConnectionInfo& info);

public slots:
    void connectToHost();
    void executeCommand(const QString& command);
    void downloadFile(const QString& remotePath, const QString& localPath);
    void uploadFile(const QString& localPath, const QString& remotePath);
    void testConnection();

signals:
    void finished();
    void errorOccurred(const QString& error);
    void outputReceived(const QString& output);
    void progressUpdated(int percentage);
    void connected();
    void disconnected();
    void transferCompleted(const QString& file, bool success);

private:
    QString executeProcess(const QString& program, const QStringList& arguments);
    bool checkSshConnection();

    SshConnection::ConnectionInfo m_connectionInfo;
    bool m_isConnected;
    QProcess* m_currentProcess;
};

#endif // SSHCONNECTION_H
