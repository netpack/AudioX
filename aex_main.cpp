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
#include "aex_main.h"
#include "ui_aex_main.h"
#include <QtSql>
#include <QtDebug>
#include <QFileInfo>
#include <QFile>
#include <aex_main.h>
#include <add_music.h>
#include "mainwindow.h"
#include "player.h"

Aex_main::Aex_main(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Aex_main)
{
    ui->setupUi(this);

}

Aex_main::~Aex_main()
{
    delete ui;
}

void Aex_main::on_bt_add_music_clicked()
{
    //this->hide();
    add_music add_music;
    add_music.setModal(true);
    add_music.exec();
}

void Aex_main::on_pushButton_clicked()
{
  MainWindow conn;
  QSqlQueryModel * model=new QSqlQueryModel();
  conn.connOpen();
  QSqlQuery* qry=new QSqlQuery(conn.adb);
  qry->prepare("select * from musics");
  qry->exec();
  model->setQuery(*qry);
  ui->tableView->setModel(model);

  conn.connClose();

  qDebug()<<model->rowCount();
}

void Aex_main::on_pushButton_2_clicked()
{
    MainWindow conn;
    QSqlQueryModel * model=new QSqlQueryModel();
    conn.connOpen();
    QSqlQuery* qry=new QSqlQuery(conn.adb);
    qry->prepare("select distinct artist from musics");
    qry->exec();
    model->setQuery(*qry);
    ui->listView->setModel(model);

    conn.connClose();

    qDebug()<<model->rowCount();
}


