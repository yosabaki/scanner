#include "custommodel.h"
#include "scanner.h"

#include <QCryptographicHash>
#include <QMessageBox>
#include <QTextStream>
#include <QtConcurrent/QtConcurrent>
#include <iostream>

Scanner::Scanner(CustomModel *model, QModelIndex const &index) : scannedCount(0) {
    addFiles(model, index);
}

Scanner::~Scanner() {

}

void Scanner::addFiles(CustomModel *model, QModelIndex const &index) {
    QString path = model->filePath(index);
    if (model->isChecked(index) == Qt::Checked) {
        addFile(path);
    } else if (model->isChecked(index) == Qt::PartiallyChecked) {
        for (int i = 0; i < model->rowCount(index); i++) {
            addFiles(model, model->index(i, 0, index));
        }
    }
}

void Scanner::addFile(QString path) {
    files.push_back(path);
}

QByteArray Scanner::calcHash(const QString &path) {
    QFile file(path);

    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }

    QCryptographicHash::Algorithm hashAlgo = QCryptographicHash::Sha512;
    QCryptographicHash hash(hashAlgo);
    if (hash.addData(&file)) {
        return hash.result();
    }
    return QByteArray();
}

void Scanner::indexFiles() {
    for (auto &dir: files) {
        if (QThread::currentThread()->isInterruptionRequested()) break;
        QDirIterator it(dir, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            if (QThread::currentThread()->isInterruptionRequested()) break;
            QFileInfo file_info = it.fileInfo();
            if (file_info.isFile()) {
                fileSizes.insertMulti(file_info.size(), file_info.filePath());
            }
            it.next();
        }
    }
    curr_pos = fileSizes.begin();
}


void Scanner::scan() {
    indexFiles();
    for (auto const &key: fileSizes.uniqueKeys()) {
        if (QThread::currentThread()->isInterruptionRequested()) break;
        if (fileSizes.count(key) == 1) { ++scannedCount; }
        else {
            QFuture<QVector<QList<QString>>> fut = QtConcurrent::run(this, &Scanner::scanNextFileGroup, key);
            emit addTreeItems(fut.result());
            emit setProgressBar(getProgress());
        }
    }
    emit finished();
}

bool Scanner::atEnd() const {
    return curr_pos == fileSizes.end();
}

QVector<QList<QString>> Scanner::scanNextFileGroup(int key) {
    curr_pos = fileSizes.find(key);
    if (fileSizes.count(curr_pos.key()) == 1) {
        ++scannedCount;
        return QVector<QList<QString>>();
    }
    QVector<QList<QString>> equalFiles;
    if (curr_pos != fileSizes.end() && fileSizes.count(curr_pos.key()) != 1) {
        QVector<std::tuple<QByteArray, QString>> key_hashes;
        while (curr_pos != fileSizes.end() && curr_pos.key() == key) {
            QByteArray hash = calcHash(curr_pos.value());
            if (!hash.isEmpty()) {
                key_hashes.push_back(std::make_tuple(hash, curr_pos.value()));
            }
            ++scannedCount;
            ++curr_pos;
        }
        std::sort(key_hashes.begin(), key_hashes.end());
        QByteArray prevHash = std::get<0>(key_hashes[0]);
        for (int j = 1; j < key_hashes.size(); ++j) {
            if (prevHash  == std::get<0>(key_hashes[j])) {
                QList<QString> equalFileList;
                equalFileList.append(std::get<1>(key_hashes[j - 1]));
                while (j < key_hashes.size() && std::get<0>(key_hashes[j]) == prevHash) {
                    equalFileList.append(std::get<1>(key_hashes[j]));
                    ++j;
                }
                equalFiles.push_back(equalFileList);
            }
            if (j < key_hashes.size()) {
                prevHash = std::get<0>(key_hashes[j]);
            }
        }
    }
    return equalFiles;
}

int Scanner::getProgress() {
    if (fileSizes.empty()) return 100;
    return double(scannedCount * 100) / fileSizes.size();
}
