#include "custommodel.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QMessageBox>
#include <vector>
#include <thread>
#include <QThread>
#include <QDesktopServices>
#include <QDesktopServices>

main_window::main_window(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    dirModel  = new CustomModel(this);

    dirModel->setRootPath(QDir::rootPath());
    dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

    ui->cancelButton->hide();
    ui->deleteButton->hide();
    ui->markButton->hide();

    ui->treeView->setModel(dirModel);

    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);
    ui->treeView->setMaximumWidth(400);

    expand(dirModel->index(QDir::homePath()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->treeWidget->hideColumn(3);

    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    QCommonStyle style;
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    ui->progressBar->hide();

    connect(ui->scanButton, &QPushButton::clicked, this, &main_window::scan);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &main_window::show_about_dialog);
    connect(ui->cancelButton, &QPushButton::clicked, this, &main_window::cancel);
    connect(ui->deleteButton, &QPushButton::clicked, this, &main_window::deleteItems);
    connect(ui->markButton, &QPushButton::clicked, this, &main_window::markItems);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &main_window::openItemMenu);
    curr_dir = QDir::homePath();

    setWindowTitle(QString("Directory Content - %1").arg(curr_dir));
}

main_window::~main_window() {
    if (ui->cancelButton->isVisible()) {
        cancel();
    }
    delete dirModel;
}

void main_window::deleteItems() {
    ui->treeWidget->hide();
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        QString id = (*it)->text(3);
        QList<QTreeWidgetItem*> itemList;
        int count = 0;
        while (*it &&(*it)->text(3) == id) {
            ++count;
            if ((*it)->checkState(0) == Qt::Checked) {
                itemList.append(*it);
            }
            ++it;
        }
        if (itemList.size() == count) {
            QMessageBox::StandardButton alert;
            alert = QMessageBox::question(this,"Continue?","File group of size " + itemList.first()->text(1) + " will be deleted all.", QMessageBox::Yes | QMessageBox::No);
            if (alert == QMessageBox::No) {
                break;
            }
        }
        for (auto item:itemList) {
            QString path = item->text(2);
            QFile file(path);
            if (file.remove()) {
                delete item;
            } else {
                QMessageBox::StandardButton alert;
                alert = QMessageBox::question(this,"Continue?","File " + itemList.first()->text(2) + " can't be deleted.", QMessageBox::Yes | QMessageBox::No);
                if (alert == QMessageBox::No) {
                    break;
                }
            }
        }
        if (count - itemList.size() == 1) {
            if (*it) {
                --it;
                auto item = *it;
                ++it;
                delete item;
            } else {
                delete ui->treeWidget->topLevelItem(ui->treeWidget->topLevelItemCount()-1);
            }
        }
    }
    ui->treeWidget->show();
}

void main_window::markItems() {
    ui->treeWidget->hide();
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        QString id = (*it)->text(3);
        (*it)->setCheckState(0, Qt::Unchecked);
        ++it;
        while (*it && (*it)->text(3) == id) {
            (*it)->setCheckState(0, Qt::Checked);
            ++it;
        }
    }
    ui->treeWidget->show();
}
void main_window::openItemMenu(const QPoint &pos) {
    clickedItem = ui->treeWidget->itemAt(pos);

    QAction *openItemAction = new QAction(tr("Open file"),this);
    QAction *showFolderAction = new QAction(tr("Show containing folder"),this);
    QAction *deleteItemAction = new QAction(tr("Delete file"),this);
    QAction *markGroupAction = new QAction(tr("Mark equal files"),this);
    connect(openItemAction, &QAction::triggered, this, &main_window::openItem);
    connect(showFolderAction, &QAction::triggered, this, &main_window::showFolder);
    connect(deleteItemAction, &QAction::triggered, this, &main_window::deleteItem);
    connect(markGroupAction, &QAction::triggered, this, &main_window::markGroupItem);
    QMenu menu(this);
    menu.addAction(openItemAction);
    menu.addAction(showFolderAction);
    menu.addAction(deleteItemAction);
    menu.addAction(markGroupAction);
    menu.exec( ui->treeWidget->mapToGlobal(pos) );
    clearClickedItem();
}

void main_window::clearClickedItem() {
    clickedItem = nullptr;
}

void main_window::openItem() {
    if (clickedItem == nullptr) return;
    QString path = clickedItem->text(2);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void main_window::showFolder() {
    if (clickedItem == nullptr) return;
    QDir dir(clickedItem->text(2));
    dir.cdUp();
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.path()));
}

void main_window::markGroupItem() {
    if (clickedItem == nullptr) return;
    QString id = clickedItem->text(3);
    QTreeWidgetItem *tmp_item = ui->treeWidget->itemAbove(clickedItem);
    while (tmp_item != nullptr && tmp_item->text(3) == id) {
        tmp_item->setCheckState(0, Qt::Checked);
        tmp_item = ui->treeWidget->itemAbove(tmp_item);
    }
    tmp_item = ui->treeWidget->itemBelow(clickedItem);
    while (tmp_item != nullptr && tmp_item->text(3) == id) {
        tmp_item->setCheckState(0, Qt::Checked);
        tmp_item = ui->treeWidget->itemBelow(tmp_item);
    }
    clickedItem->setCheckState(0,Qt::Unchecked);
}

void main_window::deleteItem() {
    if (clickedItem == nullptr) return;
    QString path = clickedItem->text(2);
    QFile file(path);
    file.remove();
    int count = 0;
    QString id = clickedItem->text(3);
    auto curr_item = ui->treeWidget->itemAbove(clickedItem);
    QTreeWidgetItem* last_item = nullptr;
    for (int i = 0;i < 2;i++) {
        if (curr_item == nullptr || curr_item->text(3) == nullptr) {
            break;
        }
        if (curr_item->text(3) == id) {
            last_item = curr_item;
            count++;
        }
        curr_item = ui->treeWidget->itemAbove(curr_item);
    }
    curr_item = ui->treeWidget->itemBelow(clickedItem);
    for (int i = 0;i < 2;i++) {
        if (curr_item == nullptr || curr_item->text(3) == nullptr) {
            break;
        }
        if (curr_item->text(3) == id) {
            last_item = curr_item;
            count++;
        }
        curr_item = ui->treeWidget->itemBelow(curr_item);
    }
    if (count == 1) {
        delete last_item;
    }
    delete clickedItem;
    clickedItem = nullptr;
}


void main_window::select_directory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
   ui->treeView->setRootIndex(dirModel->setRootPath(dir));
}

void main_window::cancel() {
    if (thread != nullptr) {
        thread->requestInterruption();
    }
}

QString getSizeString(size_t size) {
    std::vector<std::string> suffixes = {"B", "KB", "MB", "GB", "TB"};
    double d_size = size;
    size_t index = 0;
    while (d_size >= 512 && index < suffixes.size()) {
        d_size /= 1024.0;
        index++;
    }
    return QString::number(d_size, 'f', (index > 0)).append(suffixes[index].c_str());
}

void main_window::expand(QModelIndex index) {
    while (index.parent()!=index) {
        ui->treeView->expand(index);
        index = index.parent();
    }
}

void main_window::setProgressBar(int progress) {
    ui->progressBar->setValue(progress);
}

void main_window::unblock() {
    ui->scanButton->setEnabled(true);
    ui->treeView->blockSignals(false);
    ui->treeWidget->blockSignals(false);
    ui->cancelButton->hide();
    ui->scanButton->show();
    dirModel->setLock(false);
    ui->progressBar->hide();
    if (ui->treeWidget->itemAt(0,0) != nullptr) {
        ui->deleteButton->show();
        ui->markButton->show();
    }
}

void main_window::block() {
    ui->treeWidget->blockSignals(true);
    ui->scanButton->hide();
    ui->cancelButton->show();
    ui->treeView->blockSignals(true);
    ui->progressBar->show();
    ui->progressBar->setValue(0);
    dirModel->setLock(true);
    ui->markButton->hide();
    ui->deleteButton->hide();
}

void main_window::scan() {
    block();
    ui->treeWidget->clear();
    Scanner *scanner = new Scanner(dirModel, dirModel->index(dirModel->rootPath(), 0));
    thread = new QThread;
    qRegisterMetaType<QVector<QList<QString>>>("QVector<QList<QString>>");
    connect(thread, &QThread::started, scanner, &Scanner::scan);
    connect(thread, &QThread::finished, scanner, &Scanner::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(scanner, &Scanner::finished, thread, &QThread::quit);
    connect(scanner, &Scanner::finished, scanner, &Scanner::deleteLater);
    connect(scanner, &Scanner::addTreeItems, this, &main_window::addScannedFiles);
    connect(scanner, &Scanner::setProgressBar, this, &main_window::setProgressBar);
    connect(scanner, &Scanner::finished, this, &main_window::unblock);
    scanner->moveToThread(thread);
    thread->start();
}

void main_window::addScannedFiles(QVector<QList<QString> > files) {
    size_t n = files.size();
    QFileIconProvider iconProvider;
    for (size_t i = 0; i < n; i++) {

        QColor color(rand() % 256, rand() % 256, rand() % 256);
        QColor textColor("black");
        QBrush brush(color);
        if (color.black() > 100) {
            textColor = QColor("white");
        }

        int id = 0;
        if (ui->treeWidget->topLevelItemCount() != 0) {
             id = ui->treeWidget->topLevelItem(ui->treeWidget->topLevelItemCount() - 2)->text(3).toInt() + 1;
        }
        for (QString &path: files[i]) {
            QFileInfo file_info(path);
            QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
            item->setFlags(item->flags()| Qt::ItemIsUserCheckable);
            item->setCheckState(0, Qt::Unchecked);
            item->setText(3, QString('0'+id));
            for (int i = 0;i < 3; i++) {
                item->setBackground(i,brush);
                item->setTextColor(i,textColor);
            }
            item->setIcon(0, iconProvider.icon(file_info));
            item->setText(0, file_info.fileName());
            item->setText(1, getSizeString(file_info.size()));
            item->setText(2, file_info.filePath());
            ui->treeWidget->addTopLevelItem(item);
        }
        ui->treeWidget->topLevelItem(ui->treeWidget->topLevelItemCount() - 1)->setCheckState(0, Qt::Unchecked);
    }
}

void main_window::show_about_dialog() {
    QMessageBox::aboutQt(this);
}
