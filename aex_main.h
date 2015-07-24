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
#ifndef AEX_MAIN_H
#define AEX_MAIN_H

#include <QDialog>
#include "player.h"
namespace Ui {
class Aex_main;
}

class Aex_main : public QDialog
{
    Q_OBJECT
    
public:
    explicit Aex_main(QWidget *parent = 0);
    ~Aex_main();
    
private slots:
    void on_bt_add_music_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();


    void on_bt_manage_db_clicked();

private:
    Ui::Aex_main *ui;

private:
   player *pl;
};

#endif // AEX_MAIN_H
