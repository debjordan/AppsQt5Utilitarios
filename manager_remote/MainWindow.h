#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QProgressBar>
#include <QStatusBar>
#include <QLabel>
#include <QSplitter>
#include <QTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QTimer>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include "SshConnection.h"

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void connectToHost();
    void disconnectFromHost();
    void refreshFileList();
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onItemSelectionChanged();
    void downloadFile();
    void uploadFile();
    void deleteFile();
    void createFolder();
    void renameItem();
    void showItemProperties();
    void goToParentDirectory();
    void goToHomeDirectory();
    void onConnectionStatusChanged(bool connected, const QString& message);
    void onFileListReceived(const QStringList& files);
    void onTransferProgress(int percentage);
    void onErrorOccurred(const QString& error);

private:
    void setupUI();
    void setupMenuBar();
    void updateConnectionStatus();
    void updateFileListUI();
    void enableFileOperations(bool enabled);

    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_connectionLayout;
    QSplitter* m_mainSplitter;

    QGroupBox* m_connectionGroup;
    QGridLayout* m_connectionGridLayout;
    QLineEdit* m_hostEdit;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_portEdit;
    QPushButton* m_connectButton;
    QPushButton* m_disconnectButton;

    QWidget* m_fileBrowserWidget;
    QVBoxLayout* m_fileBrowserLayout;
    QHBoxLayout* m_navigationLayout;
    QPushButton* m_upButton;
    QPushButton* m_homeButton;
    QPushButton* m_refreshButton;
    QLineEdit* m_pathEdit;
    QTreeWidget* m_fileTree;

    QHBoxLayout* m_operationsLayout;
    QPushButton* m_downloadButton;
    QPushButton* m_uploadButton;
    QPushButton* m_deleteButton;
    QPushButton* m_createFolderButton;
    QPushButton* m_renameButton;
    QPushButton* m_propertiesButton;

    QTextEdit* m_logTextEdit;

    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;

    SshConnection* m_sshConnection;
    QString m_currentPath;
    bool m_isConnected;

    QMenu* m_fileMenu;
    QMenu* m_editMenu;
    QMenu* m_viewMenu;
    QMenu* m_helpMenu;

    QAction* m_connectAction;
    QAction* m_disconnectAction;
    QAction* m_exitAction;
    QAction* m_aboutAction;
};

#endif // MAINWINDOW_H
