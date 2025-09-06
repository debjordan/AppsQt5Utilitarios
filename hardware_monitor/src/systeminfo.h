#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QString>
#include <QTimer>
#include <QObject>

class SystemInfo : public QObject
{
    Q_OBJECT

public:
    explicit SystemInfo(QObject *parent = nullptr);

    QString getCpuModel();
    QString getRamInfo();
    double getCpuUsage();
    double getMemoryUsage();

private slots:
    void updateStats();

signals:
    void statsUpdated(double cpuUsage, double memUsage);

private:
    QTimer *timer;
    QString readFile(const QString &path);
    double calculateCpuUsage();

    long long previousIdle;
    long long previousTotal;
};

#endif
