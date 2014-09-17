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
#include "add_music.h"
#include "ui_add_music.h"
#include "connect.h"
#include "add_music_single.h"
add_music::add_music(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::add_music)
{
    ui->setupUi(this);


}

add_music::~add_music()
{
    delete ui;

}

void add_music::on_pushButton_clicked()
{
    this->hide();
    add_music_single add_music_single;
    add_music_single.setModal(true);
    add_music_single.exec();
}
