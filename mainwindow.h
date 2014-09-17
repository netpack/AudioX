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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <phonon/MediaObject>
#include <phonon/MediaSource>

#include <QtSql>
#include <QtDebug>
#include <QFileInfo>
#include <aex_main.h>
#include <add_music.h>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QSqlDatabase adb;
    void connClose(){
        adb.close();
        adb.removeDatabase(QSqlDatabase::defaultConnection);
    }

    bool connOpen(){
        /*connect to db*/
            adb=QSqlDatabase::addDatabase("QSQLITE");
            adb.setDatabaseName("/home/fred/adb.db");
            MainWindow conn;
            if(conn.adb.open())
            {
               // ui->status->setText("Database is now loaded!");
                qDebug()<<("Database is now loaded!");
                return true;
            } else {
                //ui->status->setText("Error opening database..");
                qDebug()<<("Error loading Database!");
                return false;
            }
        /*eof connect to db*/
    }

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_play_clicked();

    void on_stop_clicked();



    void on_bt_login_clicked();

    void on_actionExit_triggered();

private:
    Ui::MainWindow *ui;
    Phonon::MediaObject *music;

};

#endif // MAINWINDOW_H
