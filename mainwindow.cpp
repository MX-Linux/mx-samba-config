/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2021 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
 *          Dolphin_Oracle
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <QDebug>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QInputDialog>
#include <QScreen>
#include <QScrollBar>
#include <QTextStream>

#include "about.h"
#include "cmd.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons

    QSize size = this->size();
    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
        if (this->isMaximized()) { // add option to resize if maximized
            this->resize(size);
            centerWindow();
        }
    }
    setup();
    refreshUserList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::centerWindow()
{
    QRect screenGeometry = qApp->screens().first()->geometry();
    int x = (screenGeometry.width()-this->width()) / 2;
    int y = (screenGeometry.height()-this->height()) / 2;
    this->move(x, y);
}

// setup versious items first time program runs
void MainWindow::setup()
{
    connect(qApp, &QApplication::aboutToQuit, this, &MainWindow::cleanup);
    this->setWindowTitle("MX Samba Config");
    this->adjustSize();
    ui->buttonCancel->setEnabled(true);
}

// cleanup environment when window is closed
void MainWindow::cleanup()
{

}

void MainWindow::cmdStart()
{
    setCursor(QCursor(Qt::BusyCursor));
}


void MainWindow::cmdDone()
{
    ui->progressBar->setValue(ui->progressBar->maximum());
    setCursor(QCursor(Qt::ArrowCursor));
}

// set proc and timer connections
void MainWindow::setConnections()
{
    proc.disconnect();
    connect(&proc, &QProcess::readyReadStandardOutput, this, &MainWindow::updateOutput);
    connect(&proc, &QProcess::started, this, &MainWindow::cmdStart);
    connect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::cmdDone);
}

void MainWindow::updateOutput()
{
    QString out = proc.readAll();
    qDebug() << out;
    ui->outputBox->insertPlainText(out);
    QScrollBar *sb = ui->outputBox->verticalScrollBar();
    sb->setValue(sb->maximum());
    qApp->processEvents();
}

void MainWindow::progress(int counter, int duration) // processes tick emited by Cmd to be used by a progress bar
{
    ui->progressBar->setMaximum(duration);
    ui->progressBar->setValue(counter % (duration + 1));
}

QStringList MainWindow::listUsers()
{
    QString output;
    if (!cmd.run("pdbedit --list", output, true)) {
        qDebug() << "Error listing users";
        return QStringList();
    }
    QStringList list;
    for (const QString &item : output.split("\n"))
        list << item.section(":", 0, 0);
    list.sort();
    return list;
}

void MainWindow::refreshUserList()
{
    ui->listWidgetUsers->clear();
    ui->listWidgetUsers->addItems(listUsers());
}


// About button clicked
void MainWindow::on_buttonAbout_clicked()
{
    this->hide();
    displayAboutMsgBox( tr("About %1") + "MX Samba Config",
                       "<p align=\"center\"><b><h2>MX Samba Config</h2></b></p><p align=\"center\">" +
                       tr("Version: ") + qApp->applicationVersion() + "</p><p align=\"center\"><h3>" +
                       tr("Program for configuring Samba shares and users.") +
                       "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p><p align=\"center\">" +
                       tr("Copyright (c) MX Linux") + "<br /><br /></p>",
                        "/usr/share/doc/mx-samba-config/license.html", tr("%1 License").arg(this->windowTitle()));

    this->show();
}

// Help button clicked
void MainWindow::on_buttonHelp_clicked()
{
    QString url = "google.com";
    displayDoc(url, tr("%1 Help").arg(this->windowTitle()));
}

void MainWindow::on_pushButtonRemoveUser_clicked()
{
    if (!ui->listWidgetUsers->currentItem())
        return;
    const QString &user = ui->listWidgetUsers->currentItem()->text();
    if (!cmd.run("pdbedit --delete " +  user))
       qDebug() << "Cannot delete user" << user;
    refreshUserList();
}

void MainWindow::on_pushButtonAddUser_clicked()
{
    QDialog dialog(this);
    QFormLayout form(&dialog);
    form.addRow(new QLabel(tr("Enter the username and password:")));

    QLineEdit *username = new QLineEdit(&dialog);
    QLineEdit *password = new QLineEdit(&dialog);
    QLineEdit *password2 = new QLineEdit(&dialog);
    password->setEchoMode(QLineEdit::Password);
    password2->setEchoMode(QLineEdit::Password);
    form.addRow(tr("Username:"), username);
    form.addRow(tr("Password:"), password);
    form.addRow(tr("Confirm password:"), password2);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (username->text().isEmpty()) {
            QMessageBox::critical(this, tr("Error"), tr("Empty username, please enter a name."));
            return;
        }
        QString cmdstr = QString("grep -q '^%1:' /etc/passwd").arg(username->text());
        if (!cmd.run(cmdstr, true)) {
            QMessageBox::critical(this, tr("Error"), tr("Username not found on the system, make sure you enter a valid username."));
            return;
        }
        if (password->text() != password2->text()) {
            QMessageBox::critical(this, tr("Error"), tr("Passwords don't match, please enter again."));
            return;
        }
        cmdstr = QString("echo -ne '%1\n%1'|smbpasswd -as %2").arg(password->text()).arg(username->text());
        if (!cmd.run(cmdstr, true)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not add user."));
            return;
        }
    }
    refreshUserList();
}

void MainWindow::on_pushButtonUserPassword_clicked()
{
    if (!ui->listWidgetUsers->currentItem())
        return;
    QDialog dialog(this);
    QFormLayout form(&dialog);
    form.addRow(new QLabel(tr("Change the password for \'%1\'").arg(ui->listWidgetUsers->currentItem()->text())));

    QLineEdit *password = new QLineEdit(&dialog);
    QLineEdit *password2 = new QLineEdit(&dialog);
    password->setEchoMode(QLineEdit::Password);
    password2->setEchoMode(QLineEdit::Password);
    form.addRow(tr("Password:"), password);
    form.addRow(tr("Confirm password:"), password2);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (password->text() != password2->text()) {
            QMessageBox::critical(this, tr("Error"), tr("Passwords don't match, please enter again."));
            return;
        }
        const QString &cmdstr = QString("echo -ne '%1\n%1'|smbpasswd -U %2").arg(password->text()).arg(ui->listWidgetUsers->currentItem()->text());
        if (!cmd.run(cmdstr, true)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not change password."));
            return;
        }
    }
}

void MainWindow::on_pushButtonRestartSamba_clicked()
{
    if (!cmd.run("service smbd restart"))
        QMessageBox::critical(this, tr("Error"), tr("Could not restart Samba."));
}
