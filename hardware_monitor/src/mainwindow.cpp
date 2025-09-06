#include "mainwindow.h"
#include <QWidget>
#include <QFont>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Monitor de Hardware");
    setFixedSize(500, 300);

    sysInfo = new SystemInfo(this);
    connect(sysInfo, &SystemInfo::statsUpdated, this, &MainWindow::updateDisplay);

    setupUI();

    cpuModelLabel->setText("CPU: " + sysInfo->getCpuModel());
    ramSizeLabel->setText("RAM: " + sysInfo->getRamInfo());
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QGroupBox *hardwareBox = new QGroupBox("Informações do Hardware");
    QVBoxLayout *hardwareLayout = new QVBoxLayout(hardwareBox);

    cpuModelLabel = new QLabel();
    cpuModelLabel->setWordWrap(true);
    QFont font = cpuModelLabel->font();
    font.setBold(true);
    cpuModelLabel->setFont(font);

    ramSizeLabel = new QLabel();
    ramSizeLabel->setFont(font);

    hardwareLayout->addWidget(cpuModelLabel);
    hardwareLayout->addWidget(ramSizeLabel);

    QGroupBox *usageBox = new QGroupBox("Uso em Tempo Real");
    QVBoxLayout *usageLayout = new QVBoxLayout(usageBox);

    QHBoxLayout *cpuLayout = new QHBoxLayout();
    cpuUsageLabel = new QLabel("CPU: 0.0%");
    cpuProgressBar = new QProgressBar();
    cpuProgressBar->setRange(0, 100);
    cpuLayout->addWidget(cpuUsageLabel);
    cpuLayout->addWidget(cpuProgressBar);

    QHBoxLayout *memLayout = new QHBoxLayout();
    memUsageLabel = new QLabel("RAM: 0.0%");
    memProgressBar = new QProgressBar();
    memProgressBar->setRange(0, 100);
    memLayout->addWidget(memUsageLabel);
    memLayout->addWidget(memProgressBar);

    usageLayout->addLayout(cpuLayout);
    usageLayout->addLayout(memLayout);

    mainLayout->addWidget(hardwareBox);
    mainLayout->addWidget(usageBox);
    mainLayout->addStretch();
}

void MainWindow::updateDisplay(double cpuUsage, double memUsage)
{
    cpuUsageLabel->setText(QString("CPU: %1%").arg(cpuUsage, 0, 'f', 1));
    cpuProgressBar->setValue((int)cpuUsage);

    memUsageLabel->setText(QString("RAM: %1%").arg(memUsage, 0, 'f', 1));
    memProgressBar->setValue((int)memUsage);
}

#include "mainwindow.moc"
