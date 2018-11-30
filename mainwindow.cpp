#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCommonStyle>
#include <QCryptographicHash>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QStringList>

main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , hashMap()
{
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->setColumnCount(2);
    ui->treeWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    QCommonStyle style;
    ui->actionScan_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));
    ui->actionDelete->setIcon(style.standardIcon(QCommonStyle::SP_TrashIcon));
    ui->actionDelete->setVisible(false);

    connect(ui->actionScan_Directory, &QAction::triggered, this, &main_window::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &main_window::show_about_dialog);
    connect(ui->actionDelete, &QAction::triggered, this, &main_window::deleteFiles);
    connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, this, &main_window::updateDeleteButton);
   // scan_directory(QDir::homePath());
}

main_window::~main_window()
{}

void main_window::select_directory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    scan_directory(dir);
}

int main_window::countAllDirFiles(const QDir& dir, QProgressDialog& progress)
{
    int ans = 0;
    for (QFileInfo file_info: dir.entryInfoList(QDir::Files)) {
        auto it = sizeMap.find(file_info.size());
        if (it == sizeMap.end()) {
            sizeMap.insert(file_info.size(), QSet<QString>());
            it = sizeMap.find(file_info.size());
        }
        it->insert(file_info.absoluteFilePath());
        if(it->size() == 2) {
            equalFiles1.append(it);
            ans++;
        }
        if(it->size() >= 2) ans++;
        if (progress.wasCanceled()) return 0;
    }
    for (QString dirName : dir.entryList(QDir::Dirs)) {
        if (dirName != "." && dirName != "..") {
            QDir curDir = dir;
            curDir.cd(dirName);
            ans += countAllDirFiles(curDir, progress);
            if (progress.wasCanceled()) return 0;
        }
    }
    return ans;
}
/*
void main_window::hashAllDirFiles(const QDir& dir, QProgressDialog& progress)
{
    qDebug(dir.absolutePath().toStdString().c_str());

    for(QFileInfo file_info: dir.entryInfoList(QDir::Files)) {
        QFile file(file_info.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            qFatal(QString("File %1 can't be openned").arg(file_info.absoluteFilePath()).toStdString().c_str());
        } else {
            QCryptographicHash hash(QCryptographicHash::Sha256);
                if (hash.addData(&file)) {
                    progress.setValue(progress.value() + 1);

                    QString fileHash = hash.result().append(QString::number(file_info.size()));

                    // for checking non hash files comparing
                    //fileHash = "0";

                    auto it = hashMap.find(fileHash);
                    if (it == hashMap.end()) {
                        hashMap.insert(fileHash, QSet<QString>());
                        it = hashMap.find(fileHash);
                    }
                    it->insert(file_info.absoluteFilePath());
                    if (it->size() == 2) {
                        equalFiles.append(it);
                    }
                }
        }
        if(progress.wasCanceled()) {
            return;
        }
    }
    for (QString dirName : dir.entryList(QDir::Dirs)) {
        if (dirName != "." && dirName != "..") {
            QDir curDir = dir;
            curDir.cd(dirName);
            hashAllDirFiles(curDir, progress);
            if(progress.wasCanceled()) {
                return;
            }
        }
    }
}

bool compareFiles(QString fileName1, QString fileName2) {
    QFile file1(fileName1);
    QFile file2(fileName2);
    if (!file1.open(QIODevice::ReadOnly)) {
        qFatal(QString("File %1 can't be openned").arg(fileName1).toStdString().c_str());
        return false;
    } else if (!file2.open(QIODevice::ReadOnly)) {
        qFatal(QString("File %1 can't be openned").arg(fileName2).toStdString().c_str());
        return false;
    } else {
        while (!file1.atEnd() && !file2.atEnd()) {
            if(file1.readLine() != file2.readLine()) {
                return false;
            }
        }
        return file1.atEnd() && file2.atEnd();
    }
}
*/
void main_window::scan_directory(QString const& dir)
{
    ui->treeWidget->clear();
    sizeMap.clear();
    hashMap.clear();
    equalFiles.clear();
    equalFiles1.clear();

    setWindowTitle(QString("Directory Content - %1").arg(dir));
    QDir d(dir);

    QProgressDialog progress("counting files...", "Abort", 0, 100, this);
    progress.show();
    int cnt = countAllDirFiles(d, progress);
    //progress.setMaximum(cnt + 1);
    progress.setLabelText("reading files...");
    progress.setWindowModality(Qt::WindowModal);
    QCoreApplication::processEvents();

    //hashAllDirFiles(d, progress);

    qDebug(QString(QString("%1 files")).arg(cnt).toStdString().c_str());

    int i = 0;
    for (auto set : equalFiles1) {
        if (set->size() > 1) {

            for(QFileInfo file_info : *set) {
                qDebug(QString(QString("%1 %2")).arg(QString::number(i), file_info.absoluteFilePath()).toStdString().c_str());
                QFile file(file_info.absoluteFilePath());
                if (!file.open(QIODevice::ReadOnly)) {
                    qFatal(QString("File %1 can't be openned").arg(file_info.absoluteFilePath()).toStdString().c_str());
                } else {
                    QCryptographicHash hash(QCryptographicHash::Sha256);
                    hash.addData(&file);

                    QString fileHash = hash.result().append(QString::number(file_info.size()));

                    // for checking non hash files comparing
                    //fileHash = "0";

                    auto it = hashMap.find(fileHash);
                    if (it == hashMap.end()) {
                        hashMap.insert(fileHash, QSet<QString>());
                        it = hashMap.find(fileHash);
                    }
                    it->insert(file_info.absoluteFilePath());
                    if (it->size() == 2) {
                        equalFiles.append(it);
                    }
                    progress.setValue(((++i)*100) / cnt);
                }
                if (progress.wasCanceled()) return;
            }
        }
    }

    if(progress.wasCanceled()) {
        return;
    }

    qDebug(QString(QString("%1 equal files sets")).arg(equalFiles.size()).toStdString().c_str());
    /*
    QVector<QSet<QString>> newEqualFiles; //here files are certanly equal in one set

    for(auto set : equalFiles) {
        QVector<QSet<QString>> newSets;
        for(QString fileName : *set) {
            if(fileName != *set->begin()) {
                if(!compareFiles(fileName, *set->begin())) {
                    bool unique = true;
                    for (auto newSet : newSets) {
                        if(compareFiles(fileName, *newSet.begin())) {
                            newSet.insert(fileName);
                            unique = false;
                            break;
                        }
                        if (progress.wasCanceled()) { return; }
                    }
                    if(unique) {
                        QSet<QString> newSet;
                        newSet.insert(fileName);
                        newSets.append(newSet);
                    }
                }
            }
        }
        for (auto newSet : newSets) {
            for (QString fileName : newSet) {
                set->remove(fileName);
            }
            if(newSet.size() > 1) {
                newEqualFiles.append(newSet);
            }
            if (progress.wasCanceled()) { return; }
        }
    }
    */
    i = 0;
    for (auto set : equalFiles) {
        if(set->size() > 1) {
            QTreeWidgetItem* groupItem = new QTreeWidgetItem(ui->treeWidget);
            groupItem->setText(0, QString("group %1").arg(++i));
            groupItem->setText(1, QString::number(QFileInfo(*set->begin()).size()));
            for (auto fileName : *set) {
                QTreeWidgetItem* fileItem = new QTreeWidgetItem();
                fileItem->setText(0, fileName);
                groupItem->addChild(fileItem);
                if(progress.wasCanceled()) {
                    //ui->treeWidget->clear();
                    return;
                }
            }
            ui->treeWidget->addTopLevelItem(groupItem);
        }
    }
    /*
    for (auto set : newEqualFiles) {
        QTreeWidgetItem* groupItem = new QTreeWidgetItem(ui->treeWidget);
        groupItem->setText(0, QString("group %1").arg(++i));
        groupItem->setText(1, QString::number(QFileInfo(*set.begin()).size()));
        for (auto fileName : set) {
            QTreeWidgetItem* fileItem = new QTreeWidgetItem();
            fileItem->setText(0, fileName);
            groupItem->addChild(fileItem);
            if(progress.wasCanceled()) {
                //ui->treeWidget->clear();
                return;
            }
        }
        ui->treeWidget->addTopLevelItem(groupItem);
    }
    */
    if(ui->treeWidget->topLevelItemCount() == 0) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, "Nothing to show");
        ui->treeWidget->addTopLevelItem(item);
    }

    progress.setValue(progress.value() + 1);
}

void main_window::show_about_dialog()
{
    QMessageBox::aboutQt(this);
}

void main_window::updateDeleteButton(){
    qDebug("updateDelete");
    ui->actionDelete->setVisible(ui->treeWidget->selectedItems().size() > 0);
}

void main_window::deleteFiles() {
    qDebug("delete");
    for(auto item : ui->treeWidget->selectedItems()) {
        qDebug(item->text(0).toStdString().c_str());
        if (item->parent()) {
            qDebug(QString(item->text(0)).toStdString().c_str());
            QFile(item->text(0)).remove();
            item->parent()->removeChild(item);
        } else {
            while(item->childCount()) {
                qDebug(QString("_" + item->child(0)->text(0)).toStdString().c_str());
                QFile(item->child(0)->text(0)).remove();
                item->removeChild(item->child(0));
            }
            delete item;
        }
    }
}
