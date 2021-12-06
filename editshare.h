/**********************************************************************
 *  editshare.h
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

#ifndef EDITSHARE_H
#define EDITSHARE_H

#include <QDialog>

namespace Ui {
class editshare;
}

class editshare : public QDialog
{
    Q_OBJECT

public:
    explicit editshare(QWidget *parent = nullptr);
    ~editshare();

private:
    Ui::editshare *ui;
};

#endif // EDITSHARE_H
