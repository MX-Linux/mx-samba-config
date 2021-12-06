/**********************************************************************
 *  cmd.cpp
 **********************************************************************
 * Copyright (C) 2021 MX Authors
 *
 * Authors: MX Linux <http://mxlinux.org>
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

#include "cmd.h"

#include <QDebug>
#include <QEventLoop>

Cmd::Cmd(QObject *parent)
    : QProcess(parent)
{
}

void Cmd::halt()
{
    if (state() != QProcess::NotRunning) {
        terminate();
        waitForFinished(5000);
        kill();
        waitForFinished(1000);
    }
}

bool Cmd::run(const QString &cmd, bool quiet)
{
    QString output;
    return run(cmd, output, quiet);
}

// util function for getting bash command output
QString Cmd::getCmdOut(const QString &cmd, bool quiet)
{
    QString output;
    run(cmd, output, quiet);
    return output;
}

bool Cmd::run(const QString &cmd, QString &output, bool quiet)
{
    if (this->state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << this->program() << this->arguments();
        return false;
    }
    if (!quiet) qDebug().noquote() << cmd;
    QEventLoop loop;
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    start("/bin/bash", QStringList() << "-c" << cmd);
    loop.exec();
    output = readAll().trimmed();
    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

