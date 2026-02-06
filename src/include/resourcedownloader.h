#pragma once

#include <QObject>

class ResourceDownloader : public QObject {
    Q_OBJECT
public:
    explicit ResourceDownloader(QObject *parent = nullptr);
    void ensureOfflineResources();

private:
    void createFallbackIcons();
};
