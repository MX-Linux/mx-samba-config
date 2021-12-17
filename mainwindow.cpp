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
#include <QGroupBox>
#include <QInputDialog>
#include <QRadioButton>
#include <QScreen>
#include <QScrollBar>
#include <QTextStream>

#include "about.h"
#include "cmd.h"
#include "mainwindow.h"
#include "ui_editshare.h"
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
    refreshUserList();
    refreshShareList();
}

MainWindow::~MainWindow()
{
    settings.setValue("geometry", saveGeometry());
    delete ui;
}

void MainWindow::centerWindow()
{
    QRect screenGeometry = qApp->primaryScreen()->geometry();
    int x = (screenGeometry.width()-this->width()) / 2;
    int y = (screenGeometry.height()-this->height()) / 2;
    this->move(x, y);
}

void MainWindow::addEditShares(EditShare *editshare)
{
    if (editshare->exec() == QDialog::Accepted) {
        if (editshare->ui->textShareName->text().isEmpty() || !QFileInfo::exists(editshare->ui->textSharePath->text())) {
            QMessageBox::critical(this, tr("Error"), tr("Error, could not add share. Empty share name"));
            return;
        }
        if (!QFileInfo::exists(editshare->ui->textSharePath->text())) {
            QMessageBox::critical(this, tr("Error"), QString("Path: %1 doesn't exist.").arg(editshare->ui->textSharePath->text()));
            return;
        }
        QStringList userList = cmd.getCmdOut("getent group users | cut -d: -f4").split(",");
        userList.insert(0, tr("Everyone"));
        QString permissions;
        for (const QString &user : userList) {
            if (!permissions.isEmpty())
                permissions += ",";
            if (editshare->findChild<QRadioButton *>("*Deny*" + user)->isChecked())
                permissions += user + ":D";
            else if (editshare->findChild<QRadioButton *>("*ReadOnly*" + user)->isChecked())
                permissions += user + ":R";
            else if (editshare->findChild<QRadioButton *>("*FullAccess*" + user)->isChecked())
                permissions += user + ":F";
            permissions.remove(QRegularExpression(",$"));
        }

        cmd.run("runuser -u $(logname) net usershare add \"" + editshare->ui->textShareName->text() + "\" "
            + "\"" + editshare->ui->textSharePath->text() + "\" "
            + "\"" + (editshare->ui->textComment->text().isEmpty() ? "" : editshare->ui->textComment->text()) + "\""
            + " " + permissions + " guest_ok=" + (editshare->ui->comboGuestOK->currentText() == tr("Yes") ? "y" : "n"));
        refreshShareList();
    }
}


QStringList MainWindow::listUsers()
{
    QString output;
    if (!cmd.run("pdbedit --list", output, true)) {
        QMessageBox::critical(this, tr("Error"), tr("Error listing users"));
        return QStringList();
    }
    QStringList list;
    if (output.isEmpty())
        return QStringList();
    for (const QString &item : output.split("\n"))
        list << item.section(":", 0, 0);
    list.sort();
    return list;
}

void MainWindow::buildUserList(EditShare *editshare)
{
    QLayout *layout = editshare->ui->frameUsers->layout();
    QStringList userList = cmd.getCmdOut("getent group users | cut -d: -f4").split(",");
    userList.insert(0, tr("Everyone"));
    for (const QString &user : userList) {
        QGroupBox *groupBox = new QGroupBox(user);
        groupBox->setObjectName(user);
        QHBoxLayout *hbox = new QHBoxLayout;

        QRadioButton *radio = new QRadioButton(tr("&Deny"));
        radio->setObjectName("*Deny*" + user);
        hbox->addWidget(radio);
        connect(radio, &QRadioButton::pressed, radio, [radio](){if(radio->isChecked()) radio->setAutoExclusive(false); else radio->setAutoExclusive(true);});

        radio = new QRadioButton(tr("&Read Only"));
        radio->setObjectName("*ReadOnly*" + user);
        hbox->addWidget(radio);
        connect(radio, &QRadioButton::pressed, radio, [radio](){if(radio->isChecked()) radio->setAutoExclusive(false); else radio->setAutoExclusive(true);});

        radio = new QRadioButton(tr("&Full Access"));
        radio->setObjectName("*FullAccess*" + user);
        hbox->addWidget(radio);
        connect(radio, &QRadioButton::pressed, radio, [radio](){if(radio->isChecked()) radio->setAutoExclusive(false); else radio->setAutoExclusive(true);});

        hbox->addStretch(1);
        groupBox->setLayout(hbox);
        layout->addWidget(groupBox);
    }
    layout->addItem(new QSpacerItem(0, 10, QSizePolicy::Ignored, QSizePolicy::Expanding));
}

void MainWindow::refreshShareList()
{
    ui->treeWidgetShares->clear();
    QString output;
    if (!cmd.run("runuser -u $(logname) net usershare info", output)) {
        QMessageBox::critical(this, tr("Error"), tr("Error listing shares"));
        return;
    }
    if (output.isEmpty())
        return;
    QStringList listShares{output.split("\n\n")};
    qDebug() << listShares;
    for (const QString &share : listShares) {
        QStringList list = share.split("\n");
        if (list.isEmpty())
            continue;
        QStringList fixed_list;
        list.first().remove(QRegularExpression("^\\[")).remove(QRegularExpression("]$"));
        if (!list.at(1).isEmpty())
            list[1].remove(QRegularExpression("^path="));
        if (!list.at(2).isEmpty())
            list[2].remove(QRegularExpression("^comment="));
        if (!list.at(3).isEmpty()) {
            list[3].remove(QRegularExpression("^usershare_acl="));
            list[3].remove(QRegularExpression(",$"));
        }
        if (!list.at(4).isEmpty())
            list[4].remove(QRegularExpression("^guest_ok="));
        ui->treeWidgetShares->insertTopLevelItem(0, new QTreeWidgetItem(list));
    }
    for (auto i = 0; ui->treeWidgetShares->columnCount() < i; ++i)
        ui->treeWidgetShares->resizeColumnToContents(i);
}

void MainWindow::refreshUserList()
{
    ui->listWidgetUsers->clear();
    QStringList users = listUsers();
    if (!users.isEmpty())
        ui->listWidgetUsers->addItems(users);
}

void MainWindow::on_pushAbout_clicked()
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

void MainWindow::on_pushHelp_clicked()
{
    QString url = "google.com";
    displayDoc(url, tr("%1 Help").arg(this->windowTitle()));
}

void MainWindow::on_pushRemoveUser_clicked()
{
    if (!ui->listWidgetUsers->currentItem())
        return;
    const QString &user = ui->listWidgetUsers->currentItem()->text();
    if (!cmd.run("pdbedit --delete " +  user))
        QMessageBox::critical(this, tr("Error"), tr("Cannot delete user: ") + user);
    refreshUserList();
}

void MainWindow::on_pushAddUser_clicked()
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

void MainWindow::on_pushUserPassword_clicked()
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

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

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

void MainWindow::on_pushRestartSamba_clicked()
{
    if (!cmd.run("service smbd restart"))
        QMessageBox::critical(this, tr("Error"), tr("Could not restart Samba."));
    else
        QMessageBox::information(this, tr("Success"), tr("Samba restarted successfully."));
}

void MainWindow::on_pushRemoveShare_clicked()
{
    if (!ui->treeWidgetShares->currentItem())
        return;
    QString share = ui->treeWidgetShares->selectedItems().at(0)->text(0);
    if (share.isEmpty())
        return;
    if (!cmd.run("runuser -u $(logname) net usershare delete " +  share))
        QMessageBox::critical(this, tr("Error"), tr("Cannot delete share: ") + share);
    refreshShareList();
}

void MainWindow::on_pushEditShare_clicked()
{
    if (!ui->treeWidgetShares->currentItem())
        return;
    EditShare *editshare = new EditShare;
    buildUserList(editshare);
    editshare->ui->textShareName->setText(ui->treeWidgetShares->selectedItems().at(0)->text(0));
    editshare->ui->textSharePath->setText(ui->treeWidgetShares->selectedItems().at(0)->text(1));
    editshare->ui->textComment->setText(ui->treeWidgetShares->selectedItems().at(0)->text(2));
    editshare->ui->comboGuestOK->setCurrentIndex(ui->treeWidgetShares->selectedItems().at(0)->text(4) == "y" ? 0 : 1);
    QString permissions = ui->treeWidgetShares->selectedItems().at(0)->text(3);
    QStringList permission_list;
    if (!permissions.isEmpty())
        permission_list = permissions.split(",");

    for (const QString &item : permission_list) {
        if (item.isEmpty())
            continue;
        QString user = item.section(":", 0, 0);
        if (user.contains("\\"))
            user = user.section("\\", 1, 1);
        QString permission = item.section(":", 1, 1);
        QRadioButton *button;
        if (permission.toUpper() == "D") {
            button = editshare->findChild<QRadioButton *>("*Deny*" + user);
        } else if (permission.toUpper() == "R") {
            button = editshare->findChild<QRadioButton *>("*ReadOnly*" + user);
        } else if (permission.toUpper() == "F") {
            button = editshare->findChild<QRadioButton *>("*FullAccess*" + user);
        } else {
            QMessageBox::critical(this, tr("Error"), "Error: trying to process permissions: "  + item);
            return;
        }
        if (button) button->setChecked(true);
    }
    addEditShares(editshare);
}

void MainWindow::on_pushAddShare_clicked()
{
    EditShare *editshare = new EditShare;
    buildUserList(editshare);
    addEditShares(editshare);
}
