#ifndef CUSTOMMODEL_H
#define CUSTOMMODEL_H

#include <QFileSystemModel>

class CustomModel : public QFileSystemModel {
public:
    CustomModel(QObject *parent) : QFileSystemModel(parent){
        connect(this,&CustomModel::directoryLoaded, this, &CustomModel::checkLoadedDirectory);
    }
    QMap<QString, int> checkedNumber;

    int isChecked(QModelIndex const& index) const;

    void setLock(bool lock);

private slots:

    void checkLoadedDirectory(QString const &path);

private:

    QVariant data(const QModelIndex& index, int role) const;

    void setChildrenCheck(QModelIndex index, bool check);

    void setParentCheck(QModelIndex index, int prevValue, int currValue);

    Qt::ItemFlags flags(const QModelIndex& index) const;

    bool setData(const QModelIndex& index, const QVariant& value, int role);

    bool lock = false;
};

#endif // CUSTOMMODEL_H
