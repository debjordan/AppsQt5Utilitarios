#ifndef SSHTERMINAL_H
#define SSHTERMINAL_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QGroupBox>
#include <QProcess>
#include <QSplitter>
#include <QKeyEvent>
#include <QFont>
#include <QTimer>
#include <QMessageBox>
#include <QApplication>

class SSHTerminal : public QMainWindow
{
    Q_OBJECT

public:
    SSHTerminal(QWidget *parent = nullptr);
    ~SSHTerminal();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void connectToSSH();
    void disconnectSSH();
    void sendCommand();
    void onSSHOutput();
    void onSSHError();
    void onSSHFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void clearTerminal();
    void onTerminalKeyPress(QKeyEvent *event);

private:
    void setupUI();
    void setupTerminal();
    void updateConnectionStatus(bool connected);
    void addToTerminal(const QString &text, const QString &color = "white");
    void executeCommand(const QString &command);

    // UI Components
    QWidget *m_centralWidget;
    QSplitter *m_mainSplitter;

    // Connection Panel
    QGroupBox *m_connectionGroup;
    QLineEdit *m_ipEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_connectButton;
    QPushButton *m_disconnectButton;
    QLabel *m_statusLabel;

    // Terminal Panel
    QGroupBox *m_terminalGroup;
    QTextEdit *m_terminalOutput;
    QLineEdit *m_commandInput;
    QPushButton *m_sendButton;
    QPushButton *m_clearButton;

    // SSH Process
    QProcess *m_sshProcess;
    bool m_connected;
    QString m_currentHost;
    QString m_commandHistory;

    // Terminal styling
    QFont m_terminalFont;
};

// Custom QLineEdit for terminal input with history
class TerminalInput : public QLineEdit
{
    Q_OBJECT

public:
    explicit TerminalInput(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void commandEntered(const QString &command);
    void specialKeyPressed(QKeyEvent *event);

private:
    QStringList m_history;
    int m_historyIndex;
};

#endif // SSHTERMINAL_H
