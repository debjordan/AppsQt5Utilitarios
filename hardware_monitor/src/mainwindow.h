#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include "systeminfo.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateDisplay(double cpuUsage, double memUsage);

private:
    void setupUI();

    SystemInfo *sysInfo;
    QLabel *cpuModelLabel;
    QLabel *ramSizeLabel;
    QLabel *cpuUsageLabel;
    QLabel *memUsageLabel;
    QProgressBar *cpuProgressBar;
    QProgressBar *memProgressBar;
};

#endif
