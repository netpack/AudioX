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
#include "add_music_single.h"
#include "ui_add_music_single.h"
#include <QFileDialog>
#include "mainwindow.h"
#include "add_music.h"
//#include "connect.h"
#include <QMessageBox>
#include <QtSql>
add_music_single::add_music_single(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::add_music_single)
{
    ui->setupUi(this);

    if(!adb.isOpen()){
        //connect to db
        qDebug()<<"opening db form 16 in add_music_single.cpp";
            adb=QSqlDatabase::addDatabase("QSQLITE");
            adb.setDatabaseName("/home/fred/adb.db");
            adb.open();
        //eof connect to db
    }

        QSqlQueryModel * model=new QSqlQueryModel();
        QSqlQueryModel * model2=new QSqlQueryModel();

        QSqlQuery* qry=new QSqlQuery();

        qry->prepare("select name from genres1");
        qry->exec();
        model->setQuery(*qry);
        ui->cbox_g1->setModel(model);

        qry->prepare("select name from genres1");
        qry->exec();
        model2->setQuery(*qry);
        ui->cbox_g2->setModel(model2);


       // connClose();
}

add_music_single::~add_music_single()
{
    delete ui;
}

void add_music_single::on_toolButton_clicked()
{

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "",
                                                     tr("Files (*.*)"));
    if(fileName!="")
    ui->txt_file->setText(fileName);
}

void add_music_single::on_pushButton_2_clicked()
{
    this->hide();
    add_music add_music;
    add_music.setModal(true);
    add_music.exec();
}

void add_music_single::on_pushButton_clicked()
{
    QString file, artist, song, g1, g2, country, pub_date;
    file = ui->txt_file->text();
    artist = ui->txt_artist->text();
    song = ui->txt_song->text();
    g1 = ui->cbox_g1->currentText();
    g2= ui->cbox_g2->currentText();
    country = ui->cbox_country->currentText();
    pub_date = ui->txt_pub_date->text();

   // QString str = ui->cbox_g1->currentText();

    qDebug()<<"g1 is "<< g1 << " g2 is "<< g2<<" contry is "<<country;


    if(!adb.isOpen()){

        qDebug()<<"opening db form 83 in add_music_single.cpp";
            adb=QSqlDatabase::addDatabase("QSQLITE");
            adb.setDatabaseName("/home/fred/adb.db");
            adb.open();

    }



    QSqlQuery sql;
    sql.prepare("insert into musics values(NULL,:artist,:song,:g1,:g2,:country,:pub_date,:file)");
    sql.bindValue(":artist",artist);
    sql.bindValue(":song",song);
    sql.bindValue(":g1",g1);
    sql.bindValue(":g2",g2);
    sql.bindValue(":country",country);
    sql.bindValue(":pub_date",pub_date);
    sql.bindValue(":file",file);

    if(sql.exec())
    {
        qDebug() << "last sql: " << sql.lastQuery();
        QMessageBox::information(this,tr("Save"),tr("Music Added!"));
        connClose();
        this->hide();
    } else {
        QMessageBox::critical(this,tr("Error"),sql.lastError().text());
        qDebug() << "last sql: " << sql.lastQuery();
    }


}
