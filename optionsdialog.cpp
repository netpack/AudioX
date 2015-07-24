#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "QFile"
#include "QDebug"
#include "QTextStream"
#include "QProcess"
#include <QMessageBox>
#include <QtSql>
#include <commonFunctions.h>
#include <QDebug>
#include <player.h>
optionsDialog::optionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::optionsDialog)
{
    ui->setupUi(this);
   //ui->txt_selected_db->setText("Default DB adb.db selected");

    AudioXversion();
    //update_music_table();

    /* get db settings */

    QFile settings ("../config/settings.conf");
    if (!settings.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "../config/settings.conf could not be opened for read only ..???.. does it exist?";
        return;
    }


        QTextStream in(&settings);
        qDebug() << "Opening settings.conf";
        while (!in.atEnd()) {
            QString line = in.readLine();
            //txt_selected_db = line;

            /*
             * settings are in the format setting=value
             * so we split the curent line into the results array
             *
             * */

            QStringList results = line.split(" = ");
            //qDebug() << "showing results: " << results[1];

            if(results[0]=="Database"){
                txt_selected_db = results[1];
                ui->txt_selected_db->setText(txt_selected_db);
            }
            if(results[0]=="Disable_Seek_Bar"){
                disableSeekBar = results[1];
                qDebug() << "Diable Seek bar settings: " << disableSeekBar;

                if(disableSeekBar=="true"){
                    ui->checkBox_disableSeekBar->setChecked(true);
                } else {
                    ui->checkBox_disableSeekBar->setChecked(false);
                }
            }

            if(results[0]=="Normalize_Soft"){
                Normalize_Soft = results[1];
                qDebug() << "Normalize_Soft settings: " << Normalize_Soft;

                if(Normalize_Soft=="true"){
                    ui->checkBox_Normalize_Soft->setChecked(true);
                } else {
                    ui->checkBox_Normalize_Soft->setChecked(false);
                }
            }

            if(results[0]=="Disable_Volume"){
                Disable_Volume = results[1];
                qDebug() << "Diable Seek bar settings: " << Disable_Volume;

                if(Disable_Volume=="true"){
                    ui->checkBox_disableVolume->setChecked(true);
                } else {
                    ui->checkBox_disableVolume->setChecked(false);
                }
            }

        }

}



optionsDialog::~optionsDialog()
{
    delete ui;
}



void optionsDialog::on_checkBox_disableSeekBar_toggled(bool checked)
{
   qDebug() << "Disable the seek bar: " << checked;
}

void optionsDialog::saveSettings2Db()
{
    qDebug() << "Saving... ";

    QFile settings_file("../config/settings.conf");
    if(!settings_file.open(QIODevice::WriteOnly)){
        qDebug() << "../config/settings.conf is NOT writable... check the file permitions.";
        return;
    }
    QTextStream out(&settings_file);


    //check the ui and save

    out << "Database = " << ui->txt_selected_db->text() << "\n";

    if(ui->checkBox_disableSeekBar->isChecked()){
        out << "Disable_Seek_Bar = true\n";
    } else {
        out << "Disable_Seek_Bar = false\n";
    }

    if(ui->checkBox_Normalize_Soft->isChecked()){
        out << "Normalize_Soft = true\n";
    } else {
        out << "Normalize_Soft = false\n";
    }

    if(ui->checkBox_disableVolume->isChecked()){
        out << "Disable_Volume = true\n";
    } else {
        out << "Disable_Volume = false\n";
    }

    settings_file.close();

}

void optionsDialog::on_bt_save_settings_clicked()
{
    saveSettings2Db();

}

void optionsDialog::on_pushButton_clicked()
{
    saveSettings2Db();

    this->hide();
}

void optionsDialog::on_pushButton_2_clicked()
{
    //update_music_table();
    this->hide();
}

void optionsDialog::on_bt_pwd_clicked()
{
    /*
    QProcess process;
    process.start("pwd");
    qDebug () << "pid: " << process.pid();
    process.waitForFinished(-1);
    if(process.exitCode()!=0){
        qDebug () << " Error " << process.exitCode();
    }
    else{
        qDebug () << " Ok " << process.pid();
    }
    */


    QProcess sh;
    sh.start("sh", QStringList() << "-c" << "pwd");

    sh.waitForFinished();
    QByteArray output = sh.readAll();
    qDebug() << output;
    ui->txt_terminal->appendPlainText(output);
    sh.close();

}

void optionsDialog::on_bt_uname_clicked()
{
    QProcess sh;
    sh.start("sh", QStringList() << "-c" << "uname -a");

    sh.waitForFinished();
    QByteArray output = sh.readAll();
    ui->txt_terminal->appendPlainText(output);
    sh.close();
}

void optionsDialog::on_bt_edit_settings_clicked()
{
    QProcess process;
    process.start("gedit", QStringList() << "../config/settings.conf");
    process.waitForFinished(-1);
}

void optionsDialog::on_bt_free_clicked()
{
    QProcess sh;
    sh.start("sh", QStringList() << "-c" << "free -mt");
    sh.waitForFinished();
    QByteArray output = sh.readAll();
    ui->txt_terminal->appendPlainText(output);
    sh.close();
}

void optionsDialog::on_bt_df_clicked()
{
    QProcess sh;
    sh.start("sh", QStringList() << "-c" << "df -hT");
    sh.waitForFinished();
    QByteArray output = sh.readAll();
    ui->txt_terminal->appendPlainText(output);
    sh.close();
}

void optionsDialog::on_bt_update_youtubedl_clicked()
{

    ui->txt_terminal->appendPlainText("If you are having problems downloading from youtube try this on a terminal:");
   ui->txt_terminal->appendPlainText("sudo easy_install -U youtube-dl");
}

void optionsDialog::on_pushButton_3_clicked()
{
    if(!adb.isOpen()){
        qDebug()<<"opening db from optionsdialog.cpp";
            adb=QSqlDatabase::addDatabase("QSQLITE");
            adb.setDatabaseName("../config/adb.db");
            adb.open();
    }
    //delete all records in the music table
    QMessageBox::StandardButton go;
    go = QMessageBox::question(this,"Sure root?","Are you sure you want to delete ALL the tracks from the music table in the database?", QMessageBox::Yes|QMessageBox::No);
    if(go==QMessageBox::Yes){
        QSqlQuery sql;
        sql.prepare("delete from musics where 1");
        if(sql.exec()){
           QMessageBox::information(this,tr("Tracks removed"),tr("All the tracks were removed from the database!"));
           //update_music_table();
        } else {
            QMessageBox::critical(this,tr("Error"),sql.lastError().text());
            qDebug() << "last sql: " << sql.lastQuery();
        }
    }
}
