/*
 * This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    For more info contact: info@netpack.pt

    */
#include "add_pub_select_date_time.h"
#include "ui_add_pub_select_date_time.h"

add_pub_select_date_time::add_pub_select_date_time(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::add_pub_select_date_time)
{
    ui->setupUi(this);
}

add_pub_select_date_time::~add_pub_select_date_time()
{
    delete ui;
}
