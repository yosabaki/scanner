#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "scanner.h"

#include <QFileSystemModel>
#include <QMainWindow>
#include <QPoint>
#include <QTreeWidget>
#include <memory>

namespace Ui {
class MainWindow;
}

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit main_window(QWidget *parent = 0);
    ~main_window();

private slots:
    void select_directory();
    void show_about_dialog();
    void expand(QModelIndex index);
    void scan();
    void cancel();
    void deleteItems();
    void markItems();
    void openItem();
    void openItemMenu(const QPoint &pos);
    void showFolder();
    void deleteItem();
    void markGroupItem();
    void clearClickedItem();
public slots:
    void addScannedFiles(QVector<QList<QString>> files);
    void unblock();
    void setProgressBar(int progress);
private:

    QTreeWidgetItem *clickedItem;
    void block();
    QThread *thread;
    CustomModel *dirModel;
    QString curr_dir;
    std::unique_ptr<Ui::MainWindow> ui;
};

#endif // MAINWINDOW_H
