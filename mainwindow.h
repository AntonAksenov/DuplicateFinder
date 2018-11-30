#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileInfo>
#include <QMainWindow>
#include <QMap>
#include <QProgressDialog>
#include <memory>
#include <thread>

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
    void scan_directory(QString const& dir);
    void show_about_dialog();
    void deleteFiles();

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QMap<qint64, QSet<QString>> sizeMap;
    QMap<QString, QSet<QString>> hashMap; // there is no hash for QFileInfo?
    QVector<QMap<QString, QSet<QString>>::iterator> equalFiles;
    QVector<QMap<qint64, QSet<QString>>::iterator> equalFiles1;

    int countAllDirFiles(const QDir& dir, QProgressDialog& progress);
    void hashAllDirFiles(const QDir& dir, QProgressDialog& progress);
    void updateDeleteButton();
};

#endif // MAINWINDOW_H
