#include "youtubedownloader.h"
#include "ui_youtubedownloader.h"
#include "QProcess"
#include <QMessageBox>
#include <QtSql>
#include <QDir>


youtubedownloader::youtubedownloader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::youtubedownloader)
{
    ui->setupUi(this);
    ui->frame_loading->hide();

    if(!adb.isOpen()){
        //connect to db
        qDebug()<<"opening db from youtubedownloader.cpp";
            adb=QSqlDatabase::addDatabase("QSQLITE");
            adb.setDatabaseName("../config/adb.db");
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

}

youtubedownloader::~youtubedownloader()
{
    delete ui;
}

void::youtubedownloader::showLoadingFrame(){
    ui->frame_loading->show();
    //this down here allows us to show the splash sreen
    QTime dieTime= QTime::currentTime().addSecs(1);
       while( QTime::currentTime() < dieTime )
       QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    getFile();
}

void::youtubedownloader::getFile(){

/*download the song, normalize, add to db*/

    ui->bt_youtube_getIt->blockSignals(true);
    ui->txt_teminal_yd1->appendPlainText("Youtube downloader starting...");

    QString ylink;
    ylink = ui->txt_videoLink->text();

    QString yartist = ui->txt_artist->text();
    QString ysong = ui->txt_song->text();

    QString cmd;
    cmd = "youtube-dl --extract-audio --audio-format mp3 --audio-quality 192k -o \"../music/"+yartist+" - "+ysong+".mp3\" " + ylink;

    ui->txt_teminal_yd1->appendPlainText(cmd);
    ui->txt_teminal_yd1->appendPlainText("Putting hamsters on the job... hold on...");

    QProcess sh;
    sh.setProcessChannelMode(QProcess::SeparateChannels);
    sh.start("sh", QStringList() << "-c" << cmd);
    //sh.waitForBytesWritten();
    QByteArray output = sh.readAll();
    //ui->txt_teminal_yd1->appendPlainText(output);
    sh.waitForFinished(9000000);
    //output = sh.readAll();
    ui->txt_teminal_yd1->appendPlainText(output);
    sh.close();



    /*
     * Normalize and move to music folder
     *
     * Depends on SoX with all format libraries
     *
     * */
    //QProcess shn;
    //shn.start("sh", QStringList() << "-c" << "for file in ./tmp/*.mp3; do echo 'Normalizing (SoX) '$file; filename=${file##*/}; echo 'Filename: '$filename; sox --i \"$file\"; sox --norm -S \"$file\" \"../music/$filename\"; rm \"$file\"; echo 'AudioX depends on SoX with all format libraries. If your having problems try to install this: https://apps.ubuntu.com/cat/applications/libsox-fmt-all/'; done");

    //shn.waitForFinished();
    //QByteArray outputn = shn.readAll();
    //qDebug() << output;
    //ui->txt_teminal_yd1->appendPlainText(outputn);
    //shn.close();
    ui->txt_teminal_yd1->appendHtml("<p style='color:green'>Finished calibrating THC and CBD levels!</p>");

    /*
     * EOF normalize and move
     *
     * */


    /*
     *
     * add to db
     *
     */


    if(!adb.isOpen()){
        //connect to db
        qDebug()<<"opening db from youtubedownloader.cpp";
            adb=QSqlDatabase::addDatabase("QSQLITE");
            adb.setDatabaseName("../config/adb.db");
            adb.open();
        //eof connect to db
    }



    QDir path("../music/");
    QStringList files = path.entryList(QDir::Files);

    qDebug()<<"Add to db has file: "<<files;

    qDebug()<<"So assuming it's an array this should be the 1st one: "<<files[0];

    for (int i=0; i<files.size(); i++) {
        qDebug()<<"in for loooop: "<<files[i];
        QString thisfilename = files[i];

        QStringList pieces = thisfilename.split( "-" );
        QString artist = pieces.value(0).trimmed();
        QString songwext = pieces.value(1).trimmed();
        QStringList splitext = songwext.split(".");
        QString song = splitext.value(0).trimmed();

        if(song.isEmpty()){
            song="-";
        }

        qDebug()<<"artist name should be: "<<artist;
        qDebug()<<"name of the song should be: "<<song;

        QString g1 = ui->cbox_g1->currentText();
        QString g2 = ui->cbox_g2->currentText();

        QString country;

        if(ui->checkBox_cplp->isChecked()){
            country = "PT";
        } else {
            country = "Other country / language";
        }


        QString pub_date = ui->dateEdit_publishedDate->text();
        QString file = "../music/"+thisfilename;

        qDebug()<<"path for file is: "<<file;


        /*check if it's already there..*/
        int dbhasmusic=0;
        QSqlQuery query;
        query.prepare("SELECT path FROM musics WHERE path=:path");
        query.bindValue(":path",file);
        query.exec();
        while (query.next()) {
               QString path = query.value(0).toString();
               qDebug() << "Skipping: "<<path;
               dbhasmusic=1;
           }
        qDebug()<<"dbhasmusic value is: "<<dbhasmusic;

        if(dbhasmusic==0){
            //add to db

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
            } else {
                QMessageBox::critical(this,tr("Error"),sql.lastError().text());
                qDebug() << "last sql: " << sql.lastQuery();
            }

            QMessageBox::information(this,tr("Youtube Downloader"),tr("Video downloaded, converted to mp3, moved to the music folder and added to database!\nSweet isn't it?"));
            //this->hide();
        }

    } //eof for loop

    /*
     *
     * EOF add to db
     *
     * */



    ui->txt_teminal_yd1->appendPlainText("Developers NOTE: Won't work if executed from Qt run/debug!");
    ui->txt_teminal_yd1->appendPlainText("Developers NOTE: Please open AudioX executable from the source folder after build without running from Qt!");
    ui->txt_teminal_yd1->appendPlainText("Developers NOTE: This is going to ./tmp/ (if your having problems open the AudioX/tmp folder in a terminal and do: 'sudo chmod 7777 ./' )");


    ui->txt_teminal_yd1->appendPlainText("All Done!");
    ui->bt_youtube_getIt->blockSignals(false);

    ui->frame_loading->hide();
}

void youtubedownloader::on_bt_youtube_getIt_clicked()
{
    QString ylink = ui->txt_videoLink->text();
    QString yartist = ui->txt_artist->text();
    QString ysong = ui->txt_song->text();

    if(ylink.isEmpty() || yartist.isEmpty() || ysong.isEmpty()){
        QMessageBox::information(this,tr("Youtube Downloader"),tr("The link, the artist name and the song title are mandatory..."));
    } else {
        showLoadingFrame();
    }



}
void youtubedownloader::on_pushButton_clicked()
{


}

void youtubedownloader::on_bt_close_clicked()
{
    this->hide();
}
