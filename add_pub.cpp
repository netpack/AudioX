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
#include <QFileDialog>
#include <QSqlTableModel>
#include <QDebug>
#include <QMessageBox>
#include "add_pub.h"
#include "ui_add_pub.h"
#include "connect.h"

add_pub::add_pub(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::add_pub)
{
    ui->setupUi(this);
    QSqlQuery qry;
    qry.prepare("insert into pub values(NULL,NULL,NULL)");
    qry.exec();
/*
    QString pub_id;
    qry.prepare("select id from pub order by id desc limit 0,1");
    qry.exec();
    while(qry.next()){
       pub_id = qry.value(0).toString();

    }

    QSqlQueryModel * model = new QSqlQueryModel();
        model->setQuery("select * from scheduler where id='"+pub_id+"'");
        ui->tableView->setModel(model);
*/
}

add_pub::~add_pub()
{
    delete ui;
}

void add_pub::on_pushButton_3_clicked()
{
    //browse for file

    QString file = QFileDialog::getOpenFileName(this,tr("Select file"));
    ui->txt_selected_file->setText(file);

}

void add_pub::on_pushButton_4_clicked()
{
    // add new date and time //

        //get the id of the current pub row
            QString pub_id;
            QSqlQuery qry;
            qry.prepare("select id from pub order by id desc limit 0,1");
            qry.exec();
            while(qry.next()){
               pub_id = qry.value(0).toString();

            }
            qDebug()<<"This pub id is: "<<pub_id;

        //get the date/time selected
            QString dateandtime1 = ui->dateTimeEdit->text();
            qDebug()<<"selected date and time: " << dateandtime1;

            QStringList array_dateandtime1 = dateandtime1.split(" ");

            QStringList array_date1 = array_dateandtime1[0].split("/");
            QString dia1 = array_date1[0];
            QString mes1 = array_date1[1];
            QString ano1 = array_date1[2];

            qDebug () << "dia1: " << dia1 << " mes1: " << mes1 << " ano1: "<<ano1;

            QStringList array_time1 = array_dateandtime1[1].split(":");
            QString hora1 = array_time1[0];
            QString min1 = array_time1[1];

            qDebug () << "hora1: " << hora1 << "min1: "<< min1;



        //prepare the query

            QSqlQuery qry_add;
            qry_add.prepare("insert into scheduler values ('"+pub_id+"','"+ano1+"','"+mes1+"','"+dia1+"','"+hora1+"','"+min1+"','1',NULL,NULL,NULL,NULL,NULL,NULL,NULL)");


        //add to scheduler

            if(qry_add.exec())
            {
                qDebug () << qry_add.lastQuery();
                //update the table view of the database with the events for this pub
                    //QSqlQueryModel * model = new QSqlQueryModel();
                    //model->setQuery("select * from scheduler where id='"+pub_id+"'");
                   // ui->tableView->setModel(model);

                QSqlQuery qss;
                qss.prepare("select * from scheduler where id='"+pub_id+"'");
                qss.exec();
                while(qss.next()){
                    QString thistype = qss.value(6).toString();
                    qDebug()<<"the type is: "<<thistype;
                    
                    //if type is 1 then show it like: 25/08/2014 at 21:30
                    
                    if(thistype == "1"){
                        QString thisyear = qss.value(1).toString();
                        QString thismonth = qss.value(2).toString();
                        QString thisday = qss.value(3).toString();

                        QString thishour = qss.value(4).toString();
                        QString thismin = qss.value(5).toString();

                        QString thisdateline = thisday+"/"+thismonth+"/"+thisyear+" at "+thishour+":"+thismin;
                        ui->listWidget->addItem(thisdateline);
                        qDebug()<<"134 add_pub.cpp Add This line: "<<thisdateline;

                    }
                    
                    //if type is 2 then show it like: Mondays at 21:30
                    
                    //if type is 3 then show it like: From 25/08/2014 To 26/08/2014 at 21:30
                    
                }

            } else {
                qDebug () << qry_add.lastQuery();
            }





}

void add_pub::updateScheduleTable()
{

}

void add_pub::on_pushButton_clicked()
{
//get the id of the current pub row
    QString pub_id;
    QSqlQuery qry;
    qry.prepare("select id from pub order by id desc limit 0,1");
    qry.exec();
    while(qry.next()){
       pub_id = qry.value(0).toString();

    }
//get the path
    QString thisPath = ui->txt_selected_file->text();
//get the name
    QString thisName = ui->txt_name->text();

    //QSqlQuery qry;
    qry.prepare("update pub set name='"+thisName+"', path='"+thisPath+"' where id="+pub_id+"");
    qry.exec();

   QMessageBox::information(this,"Add pub","Publicity saved!");

}
