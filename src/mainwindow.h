#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QFileSystemModel>

#include "logger.h"
#include "analyzer.h"
#include "groupstablemodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onAnalysisThreadFinished();
    void onAnalysisDone(const AnalysisResult& result);

private slots:
    void on_dirTree_clicked(const QModelIndex &index);
    void on_analyzeButton_clicked();

private:
    Ui::MainWindow *ui;
    QFileSystemModel *fileSystemModel;
    GroupsTableModel *groupsTableModel;
    Logger *logger;
    Analyzer *analyzer;

    void decorateSplitter(QSplitter* splitter, int index);
};

#endif // MAINWINDOW_H
