#include "resourcedownloader.h"

// 本地离线资源准备
#include <QCoreApplication>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QStandardPaths>

ResourceDownloader::ResourceDownloader(QObject *parent) : QObject(parent) {}

void ResourceDownloader::ensureOfflineResources() {
    createFallbackIcons();
}

void ResourceDownloader::createFallbackIcons() {
    QDir dir(QCoreApplication::applicationDirPath() + "/assets/icons");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    const QList<QPair<QString, QColor>> icons = {
        {"whiteboard.png", QColor("#4F46E5")},
        {"attendance.png", QColor("#0EA5E9")},
        {"classisland.png", QColor("#22C55E")},
        {"random.png", QColor("#A855F7")},
        {"ai.png", QColor("#F97316")},
        {"settings.png", QColor("#64748B")},
        {"custom.png", QColor("#334155")}
    };

    for (const auto &entry : icons) {
        QFile f(dir.filePath(entry.first));
        if (f.exists()) continue;
        QPixmap pix(96, 96);
        pix.fill(Qt::transparent);

        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setBrush(entry.second);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(8, 8, 80, 80, 20, 20);
        p.end();

        pix.save(dir.filePath(entry.first));
    }
}
