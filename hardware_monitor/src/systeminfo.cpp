#include "systeminfo.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QProcess>

SystemInfo::SystemInfo(QObject *parent)
    : QObject(parent), previousIdle(0), previousTotal(0)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SystemInfo::updateStats);
    timer->start(1000); // Atualiza a cada segundo
}

QString SystemInfo::readFile(const QString &path)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        return in.readAll();
    }
    return QString();
}

QString SystemInfo::getCpuModel()
{
    QString cpuInfo = readFile("/proc/cpuinfo");
    QStringList lines = cpuInfo.split('\n');

    for (const QString &line : lines) {
        if (line.startsWith("model name")) {
            QStringList parts = line.split(':');
            if (parts.size() > 1) {
                return parts[1].trimmed();
            }
        }
    }
    return "CPU não identificada";
}

QString SystemInfo::getRamInfo()
{
    QString memInfo = readFile("/proc/meminfo");
    QStringList lines = memInfo.split('\n');

    long totalKb = 0;
    for (const QString &line : lines) {
        if (line.startsWith("MemTotal:")) {
            QStringList parts = line.split(QRegExp("\\s+"));
            if (parts.size() > 1) {
                totalKb = parts[1].toLong();
                break;
            }
        }
    }

    double totalGb = totalKb / 1024.0 / 1024.0;
    return QString("%1 GB").arg(totalGb, 0, 'f', 2);
}

double SystemInfo::calculateCpuUsage()
{
    QString statContent = readFile("/proc/stat");
    QStringList lines = statContent.split('\n');

    if (lines.isEmpty()) return 0.0;

    QString cpuLine = lines[0];
    QStringList values = cpuLine.split(QRegExp("\\s+"));

    if (values.size() < 5) return 0.0;

    long long user = values[1].toLongLong();
    long long nice = values[2].toLongLong();
    long long system = values[3].toLongLong();
    long long idle = values[4].toLongLong();
    long long iowait = values[5].toLongLong();
    long long irq = values[6].toLongLong();
    long long softirq = values[7].toLongLong();

    long long currentIdle = idle + iowait;
    long long currentTotal = user + nice + system + idle + iowait + irq + softirq;

    if (previousTotal == 0) {
        previousIdle = currentIdle;
        previousTotal = currentTotal;
        return 0.0;
    }

    long long totalDiff = currentTotal - previousTotal;
    long long idleDiff = currentIdle - previousIdle;

    double usage = 100.0 * (totalDiff - idleDiff) / totalDiff;

    previousIdle = currentIdle;
    previousTotal = currentTotal;

    return usage;
}

double SystemInfo::getCpuUsage()
{
    return calculateCpuUsage();
}

double SystemInfo::getMemoryUsage()
{
    QString memInfo = readFile("/proc/meminfo");
    QStringList lines = memInfo.split('\n');

    long totalKb = 0, availableKb = 0;

    for (const QString &line : lines) {
        if (line.startsWith("MemTotal:")) {
            QStringList parts = line.split(QRegExp("\\s+"));
            if (parts.size() > 1) {
                totalKb = parts[1].toLong();
            }
        } else if (line.startsWith("MemAvailable:")) {
            QStringList parts = line.split(QRegExp("\\s+"));
            if (parts.size() > 1) {
                availableKb = parts[1].toLong();
            }
        }
    }

    if (totalKb > 0) {
        long usedKb = totalKb - availableKb;
        return (double)usedKb / totalKb * 100.0;
    }

    return 0.0;
}

void SystemInfo::updateStats()
{
    double cpu = getCpuUsage();
    double mem = getMemoryUsage();
    emit statsUpdated(cpu, mem);
}

#include "systeminfo.moc"
