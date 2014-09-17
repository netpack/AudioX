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
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
/*connect to db*/
    adb=QSqlDatabase::addDatabase("QSQLITE");
    adb.setDatabaseName("/home/fred/adb.db");
    if(adb.open())
    {
        ui->status->setText("Database is now loaded!");
    } else {
        ui->status->setText("Error opening database..");
    }
/*eof connect to db*/

    //QPixmap pix_logo("/home/fred/Documents/NETPACK/NetPackimgs/Logo_09_small_txt.png");
   // ui->logo->setPixmap(pix_logo);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_bt_login_clicked()
{
    QString user, pass;
    user = ui->txt_user->text();
    pass = ui->txt_pass->text();

    if(!adb.isOpen()){
        qDebug()<<("Failed to open db!");
        return;
    }
    connOpen();
    QSqlQuery sql;

    if(sql.exec("select * from users where user='"+user+"' and pass='"+pass+"'"))
    {
        int count=0;
        while(sql.next())
        {
            count++;
        }
        if(count==1)
        {
            ui->status->setText("Login was OK!");
            connClose();
            this->hide();
            Aex_main Aex_main;
            Aex_main.setModal(true);
            Aex_main.exec();

        } else
        {
            ui->status->setText("Login was NOT OK!");
        }
    }


}
void MainWindow::on_play_clicked()
{

}

void MainWindow::on_stop_clicked()
{

}

void MainWindow::on_actionExit_triggered()
{

}
