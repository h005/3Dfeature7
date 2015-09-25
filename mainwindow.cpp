#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "fea.hh"

#include <QFileDialog>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    fea = NULL;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_open_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                      tr("Open"),".",
                      tr("matrix Files(*.matrix)"));
    if(fileName == NULL)
        return;
    QFileInfo fileInfo(fileName);
    QString path = fileInfo.absoluteDir().absolutePath().append("/");

    fea = new Fea(fileName,path);

    ui->label->setText(QString("path: ").append(fileName));

}

void MainWindow::on_process_clicked()
{
    fea->setFeature();
}

void MainWindow::on_modelPath_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                      tr("Open"),".",
                      tr("matrix Files(*.dae *.obj *.off)"));
    if(fileName == NULL)
        return;

    QString tmp = ui->label->text();
    tmp.append("\nmodel file:").append(fileName);
    ui->label->setText(tmp);

    fea->setModelPath(fileName);
}
