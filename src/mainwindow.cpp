#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPixmap>
#include <QMessageBox>
#include <QDir>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#ifdef Q_OS_WIN
    // Decorate splitters for Windows
    decorateSplitter(ui->dirTreeSplitter, 1);
    decorateSplitter(ui->logBarSplitter, 1);
#endif

    ui->progressBar->setHidden(true);

    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);
    fileSystemModel->setRootPath(QDir::rootPath());
    ui->dirTree->setModel(fileSystemModel);
    for (int i = 1; i < fileSystemModel->columnCount(); ++i) {
        ui->dirTree->hideColumn(i);
    }
    ui->dirTree->header()->hide();
    ui->dirTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->dirTree->header()->setStretchLastSection(false);

    groupsTableModel = new GroupsTableModel(this);
    ui->groupsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->groupsView->setModel(groupsTableModel);

    logger = new Logger(this, ui->loggingBrowser);

    analyzer = new Analyzer();
    analyzer->moveToThread(analyzer);

    connect(this, &MainWindow::destroyed, analyzer, &Analyzer::quit);
    connect(analyzer, &Analyzer::finished, this, &MainWindow::onAnalysisThreadFinished);
    connect(analyzer, &Analyzer::analysisProgressTextSent, logger, &Logger::print);
    connect(analyzer, &Analyzer::analysisDone, this, &MainWindow::onAnalysisDone);

    logger->print("Started. Main thread: " + QString::number((long long)this->thread(), 16));
}

MainWindow::~MainWindow()
{
    analyzer->requestInterruption();
    // waiting for 2 seconds before exit
    analyzer->wait(2000);
    delete ui;
}

void MainWindow::onAnalysisThreadFinished()
{
    ui->analyzeButton->setText("Analyze");
    ui->analyzeButton->setEnabled(true);
    ui->progressBar->setHidden(true);
}

void MainWindow::onAnalysisDone(const AnalysisResult &result)
{
    groupsTableModel->update(result.getGroupsAsVector());
    ui->foldersCountText->setText(QString::number(result.subfoldersCount));
    ui->folderSize->setText(QString::number(result.size));
    ui->filesCountText->setText(QString::number(result.totalFilesCount));
    ui->groupsCountText->setText(QString::number(result.groups.size()));
}

void MainWindow::on_dirTree_clicked(const QModelIndex &index)
{
    QFileInfo fileInfo = fileSystemModel->fileInfo(index);
    if (fileInfo.isDir()) {
        ui->analysisPath->setText(fileInfo.absoluteFilePath());
    }
}

void MainWindow::on_analyzeButton_clicked()
{
    if (analyzer->isRunning()) {
        analyzer->requestInterruption();
        ui->analyzeButton->setEnabled(false);
    }
    else {
        analyzer->folderPath = ui->analysisPath->toPlainText();
        analyzer->start();
        ui->analyzeButton->setText("Cancel");
        ui->progressBar->setHidden(false);
    }
}

void MainWindow::decorateSplitter(QSplitter *splitter, int index)
{
    const int gripLength = 15;
    const int gripWidth = 1;
    const int grips = 3;

    splitter->setOpaqueResize(true);
    splitter->setChildrenCollapsible(true);

    splitter->setHandleWidth(7);
    QSplitterHandle* handle = splitter->handle(index);
    Qt::Orientation orientation = splitter->orientation();
    QHBoxLayout* layout = new QHBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    if (orientation == Qt::Horizontal)
    {
        for (int i = 0; i < grips; ++i)
        {
            QFrame* line = new QFrame(handle);
            line->setMinimumSize(gripWidth, gripLength);
            line->setMaximumSize(gripWidth, gripLength);
            line->setFrameShape(QFrame::StyledPanel);
            layout->addWidget(line);
        }
    }
    else
    {
        layout->addStretch();
        QVBoxLayout* vbox = new QVBoxLayout;
        for (int i = 0; i < grips; ++i)
        {
            QFrame* line = new QFrame(handle);
            line->setMinimumSize(gripLength, gripWidth);
            line->setMaximumSize(gripLength, gripWidth);
            line->setFrameShape(QFrame::StyledPanel);
            vbox->addWidget(line);
        }
        layout->addLayout(vbox);
        layout->addStretch();
    }
}
