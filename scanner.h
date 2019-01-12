#ifndef SCANNER_H
#define SCANNER_H

#include "custommodel.h"

#include <QList>
#include <QObject>



class Scanner: public QObject {
    Q_OBJECT

public:

    Scanner(CustomModel *model, QModelIndex const &index);

    ~Scanner();

    QByteArray calcHash(QString const& path);

    int getProgress();

    bool atEnd() const;

public slots:

    void scan();

signals:
    void addTreeItems(QVector<QList<QString>> items);

    void setProgressBar(int percent);

    void finished();

    void error(QString err);

private:
    void indexFiles();

    QVector<QList<QString>> scanNextFileGroup(int key);

    void addFiles(CustomModel *model, QModelIndex const& index);

    void addFile(QString path);

    int scannedCount;
    QMap<int, QString>::iterator curr_pos;
    QMap<int, QString> fileSizes;
    QVector<QString> files;
};

#endif // SCANNER_H
