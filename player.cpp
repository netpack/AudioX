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
#include "player.h"
#include "ui_player.h"
#include "optionsdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QList>
#include <QDebug>
#include <Phonon/MediaObject>
#include <QDateTime>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QtWebKit/QWebView>
#include <QtSql>
#include <audioclass.h>
#include "add_music_single.h"
#include "dialog.h"
#include "addgenre.h"
#include "addjingle.h"
#include "qvumeter.h"
#include "add_pub.h"
#include "youtubedownloader.h"
#include "commonFunctions.h"


/*

  Possible TODO ?: Try to send mediaObject to high priority tread

*/


player::player(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::player)
{

    qDebug() << "Starting AudioX v2...";
    AudioXversion();


/* Var for keeping track of playing state *
 *
 *  0 = stoped (default)
 *  1 = playing
 *  2 = paused
 *
 * */
 playing=0;

 autoMode=1; //autoMode will load new musics into the playlist so it never stops playing once started. On by default.



 /* get db settings */
 QFile settings ("../config/settings.conf");
 if (!settings.open(QIODevice::ReadOnly | QIODevice::Text)){
     qDebug() << "../config/settings.conf could not be loaded. Please check that it exists";
     return;
 }

     QTextStream in(&settings);
     qDebug() << "Opening ../config/settings.conf";
     while (!in.atEnd()) {
         QString line = in.readLine();
         QStringList results = line.split(" = ");

         if(results[0]=="Database"){
             txt_selected_db = results[1];
         }
         if(results[0]=="Disable_Seek_Bar"){
             disableSeekBar = results[1];
             qDebug() << "Diable Seek bar settings: " << disableSeekBar;
         }
         if(results[0]=="Normalize_Soft"){
             normalization_soft = results[1];
             qDebug() << "normalization_soft settings: " << normalization_soft;
         }
         if(results[0]=="Disable_Volume"){
             Disable_Volume = results[1];
             qDebug() << "Disable_Volume settings: " << Disable_Volume;
         }
         if(results[0]=="ask_normalize_new_files"){
             ask_normalize_new_files = results[1];
             qDebug() << "ask_normalize_new_files settings: " << ask_normalize_new_files;
         }



     }


/* main music objects */
 audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
 mediaObject = new Phonon::MediaObject(this);
 metaInformationResolver = new Phonon::MediaObject(this);

 /* Tick interval and transition time (not working yet under gnu/linux.. not sure on other os's) */
 //mediaObject->setTickInterval(0);
 //mediaObject->setTransitionTime(0);

/* Connect left and right audio data outpus to vumeter slot */
 dataout = new Phonon::AudioDataOutput(this);
 Phonon::createPath(mediaObject, dataout);
 connect( dataout, SIGNAL(dataReady(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >)),
         this, SLOT(dataReceived(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >)));



    /* Music object for holding on-the-fly music loading and playing on top of AudioX main playlist *
     *
     * I think this can be useful for manually playing smaller station ids for example
     * on top of starting or ending tracks
     *
     * */
    music=new Phonon::MediaObject(this);
    Phonon::createPath(mediaObject,audioOutput);


    /* Main music object signals and slots */
    connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),this, SLOT(stateChanged(Phonon::State,Phonon::State)));
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),this, SLOT(sourceChanged(Phonon::MediaSource)));
    connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(aboutToFinish()));

    /* main clock signals and slots */
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
    timer->start(1000);


    /*Scheduler*/
    /*
     *
     * This will run every minute to check if there is something on the database programed for this minute in 'time' and do something if so.
     *
     * */
    QTimer *schedulerTimer = new QTimer(this);
    connect(schedulerTimer,SIGNAL(timeout()),this,SLOT(run_scheduler()));
    schedulerTimer->start(60000);


    ui->setupUi(this); // setup the ui

    showTime();

    ui->seekSlider->setMediaObject(mediaObject);
    ui->seekSlider->setEnabled(false);
    if(disableSeekBar == "false"){
        ui->seekSlider->setEnabled(true);
    }

    ui->volumeSlider->setAudioOutput(audioOutput);
    ui->volumeSlider->setEnabled(false);
    if(Disable_Volume == "false"){
        ui->volumeSlider->setEnabled(true);
    }

    checkDbOpen();
    /*eof connect to db*/

    /*Populate Music Table*/
    /*
    QSqlQueryModel * model=new QSqlQueryModel();
    QSqlQuery* qry=new QSqlQuery();
    qry->prepare("select * from musics");
    qry->exec();
    model->setQuery(*qry);
    ui->musicView->setModel(model);
    */

    /*Populate music table with an editable table field on double-click*/

    QSqlTableModel * model = new QSqlTableModel(this);
    model->setTable("musics");
    model->select();
    ui->musicView->setModel(model);
    ui->musicView->setSortingEnabled(true);
    ui->musicView->hideColumn(0);
    ui->musicView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);


    /*Populate jingles table with an editable table field on double-click*/
    QSqlTableModel * jinglesmodel = new QSqlTableModel(this);
    jinglesmodel->setTable("jingles");
    jinglesmodel->select();
    ui->jinglesView->setModel(jinglesmodel);
    ui->jinglesView->setSortingEnabled(true);
    ui->jinglesView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    /*Populate Pub table*/
    QSqlTableModel *pubmodel = new QSqlTableModel(this);
    pubmodel->setTable("pub");
    pubmodel->select();
    ui->PubView->setModel(pubmodel);
    ui->PubView->setSortingEnabled(true);
    ui->PubView->hideColumn(0);
    ui->PubView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);


    /*Populate genre1 and 2 filters*/
    QSqlQueryModel * model_genre1=new QSqlQueryModel();
    QSqlQueryModel * model_genre2=new QSqlQueryModel();

    QSqlQuery* qry=new QSqlQuery();

    qry->prepare("select name from genres1");
    qry->exec();
    model_genre1->setQuery(*qry);
    ui->cBoxGenre1->setModel(model_genre1);

    qry->prepare("select name from genres1");
    qry->exec();
    model_genre2->setQuery(*qry);
    ui->cBoxGenre2->setModel(model_genre2);



    /*Drag & Drop Set*/
    /*player*/
     this->setAcceptDrops(true);
    /*playlist (list_playlist)*/
     ui->list_playlist->setSelectionMode(QAbstractItemView::SingleSelection);
     ui->list_playlist->setDragEnabled(true);
     ui->list_playlist->viewport()->setAcceptDrops(true);
     ui->list_playlist->setAcceptDrops(true);
     ui->list_playlist->setDropIndicatorShown(false);
     ui->list_playlist->setDragDropMode(QAbstractItemView::InternalMove);
    /*Music list*/
     ui->musicView->setSelectionMode(QAbstractItemView::SingleSelection);
     ui->musicView->setDragEnabled(true);
     ui->musicView->viewport()->setAcceptDrops(false);
     ui->musicView->setAcceptDrops(false);
     ui->musicView->setDropIndicatorShown(true);
     ui->musicView->setDragDropMode(QAbstractItemView::DragOnly);
     ui->musicView->setSelectionBehavior(QAbstractItemView::SelectRows);
     ui->musicView->setSelectionMode(QAbstractItemView::SingleSelection);


    indexSourceChanged = 0; //index p correr sourceChanged apenas uma vez devido a bug vindo do connect p o vumeter
    indexcanal = 4; //index p atualização do volume de qvumeter
    indexJust3rdDropEvt = 0; //index p aceitar drop so ao 2º loop visto ser chamado 2 vezes sendo a ultima quando entramos em cima da listView



    ui->musicView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->musicView, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(musicViewContextMenu(const QPoint&)));

    ui->list_playlist->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->list_playlist, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(list_playlistContextMenu(const QPoint&)));

}

void::player::list_playlistContextMenu(const QPoint& pos){
    QPoint globalPos = ui->list_playlist->mapToGlobal(pos);
    QMenu thisMenu;
    QString remove = "Remove this track from the playlist";
    QString moveToTop = "Send this track to the top of the playlist";
    QString moveToBottom = "Send this track to the bottom of the playlist";

    thisMenu.addAction(remove);
    thisMenu.addAction(moveToTop);
    thisMenu.addAction(moveToBottom);

    QAction* selectedItem = thisMenu.exec(globalPos);
    if(selectedItem){
        QString selectedListItem = selectedItem->text();
        int rowidx = ui->list_playlist->selectionModel()->currentIndex().row();
        estevalor = ui->list_playlist->model()->data(ui->list_playlist->model()->index(rowidx,0)).toString();

        if(selectedListItem==remove){
            delete ui->list_playlist->item(rowidx);
        }
        if(selectedListItem==moveToTop){
            delete ui->list_playlist->item(rowidx);
            ui->list_playlist->insertItem(0,estevalor);
        }
        if(selectedListItem==moveToBottom){
            delete ui->list_playlist->item(rowidx);
            ui->list_playlist->addItem(estevalor);
        }

    }
}

void::player::musicViewContextMenu(const QPoint& pos){
    QPoint globalPos = ui->musicView->mapToGlobal(pos);
    QMenu thisMenu;
    QString addToBottomOfPlaylist = "Add to the bottom of playlist";
    QString addtoTopOfPlaylist = "Add to the top of the playlist";
    QString deleteThisFromDB = "Delete this track from database";
    QString openWithAudacity = "Open this in Audacity";


    thisMenu.addAction(addToBottomOfPlaylist);
    thisMenu.addAction(addtoTopOfPlaylist);
    thisMenu.addAction(deleteThisFromDB);
    thisMenu.addAction(openWithAudacity);

    QAction* selectedItem = thisMenu.exec(globalPos);
    if (selectedItem)
    {
        //qDebug()<<"selected item in context menu was: "<<selectedItem->text();
        QString selectedMenuItem = selectedItem->text();
        int rowidx = ui->musicView->selectionModel()->currentIndex().row();
        estevalor = ui->musicView->model()->data(ui->musicView->model()->index(rowidx,7)).toString();

        if(selectedMenuItem==addToBottomOfPlaylist){
            qDebug()<<"Launch add this to bottom of playlist";
            ui->list_playlist->addItem(estevalor);
        }
        if(selectedMenuItem==addtoTopOfPlaylist){
            qDebug()<<"Launch add this to top of playlist";
         ui->list_playlist->insertItem(0,estevalor);
        }
        if(selectedMenuItem==deleteThisFromDB){

            QMessageBox::StandardButton go;
            go = QMessageBox::question(this,"Sure boss?","Are you sure you want to delete the track from the database?", QMessageBox::Yes|QMessageBox::No);
            if(go==QMessageBox::Yes){
                QSqlQuery sql;
                sql.prepare("delete from musics where path=:path");
                sql.bindValue(":path",estevalor);
                if(sql.exec()){
                    QMessageBox::information(this,tr("Track removed"),tr("The track was removed from the database!"));
                    update_music_table();
                } else {
                    QMessageBox::critical(this,tr("Error"),sql.lastError().text());
                    qDebug() << "last sql: " << sql.lastQuery();
                }
            }
           }

       if(selectedMenuItem==openWithAudacity){
           QProcess sh;
           sh.startDetached("sh",QStringList()<<"-c"<<"audacity \""+estevalor+"\"");
       }


    }
    else
    {
        // nothing was chosen
    }
}

void::player::run_scheduler(){
    qDebug () << "Scheluder running...";
    checkDbOpen();
    /*check if there is any thing to go on now*/

QSqlQuery sched_qry;
sched_qry.prepare("select * from scheduler");
if(sched_qry.exec()){
    while(sched_qry.next()){
        QString tipo = sched_qry.value(6).toString();
        qDebug() << "Scheduler got a type "<<tipo<<" event rule.";

        /*
         *
         * Var tipo determines the type of shedule
         * 1 = an event that is to be played only once at a specific minute in time
         *
         *
         *
         * */

        if(tipo=="1"){
            QDateTime now = QDateTime::currentDateTime();

            //agora ano
                QString ano1 = now.toString("yyyy");
            //agora mes
                QString mes1 = now.toString("M");
            //agora dia
                QString dia1 = now.toString("d");
            //agora hora
                QString hora1 = now.toString("h");
            //agora minuto
                QString min1 = now.toString("m");


            //este qry ano
                QString ano2 = sched_qry.value(1).toString();
            //este qry mes
                QString mes2 = sched_qry.value(2).toString();
            //este qry dia
                QString dia2 = sched_qry.value(3).toString();
            //este qry hora
                QString hora2 = sched_qry.value(4).toString();
            //este qry minuto
                QString min2 = sched_qry.value(5).toString();

                /*
                qDebug()<<"Scheduler is comparing values: ";
                qDebug()<<ano1<<" should be equal to "<<ano2<<" -> ano";
                qDebug()<<mes1<<" should be equal to "<<mes2<<" -> mes";
                qDebug()<<dia1<<" should be equal to "<<dia2<<" -> dia";
                qDebug()<<hora1<<" should be equal to "<<hora2<<" -> hora";
                qDebug()<<min1<<" should be equal to "<<min2<<" -> minuto";
                 */


            //if agora values == este qry values
                if((ano1==ano2) && (mes1==mes2) && (dia1==dia2) && (hora1==hora2) && (min1==min2)){
                    qDebug() << "Scheduled event now fired!";

                    QString schId = sched_qry.value(0).toString();

                    //add to playlist
                    QSqlQuery getPath;
                    getPath.prepare("select path from pub where id='"+schId+"'");
                    if(getPath.exec()){
                        while(getPath.next()){
                            QString pubPath = getPath.value(0).toString();
                            ui->list_playlist->insertItem(0,pubPath);
                            qDebug()<<"Scheduled event added to the top of the playlist: "<<pubPath;
                        }

                    }

                    //delete scheduler row cause its a type 1

                    QSqlQuery del_qry;
                    del_qry.prepare("delete from scheduler where id='"+schId+"'");
                    if(del_qry.exec()){
                        qDebug () << "Scheduled rule was deleted!";
                    } else {
                        qDebug()<<"exeption deleting scheduled rule with qry: "<<del_qry.lastQuery()<<" we got: "<<del_qry.lastError();
                    }

                    //check if pub still has other scheduler rules and delete from pub if not


                }

                //
                //TODO.. other types..
                //




        }

    }
}

}

void player::checkDbOpen(){
    /*connect to db*/
        if(!adb.isOpen()){
            qDebug()<<"correct: opening db from checkDbOpen in player.cpp";
                adb=QSqlDatabase::addDatabase("QSQLITE");
                adb.setDatabaseName("../config/adb.db");
                adb.open();
        }
        /*eof connect to db*/
}

void player::update_music_table(){
    checkDbOpen();
    /*Populate music table with an editable table field on double-click*/

    QSqlTableModel * model = new QSqlTableModel(this);
    model->setTable("musics");
    model->select();
    ui->musicView->setModel(model);


    /*Populate jingles table with an editable table field on double-click*/
    QSqlTableModel * jinglesmodel = new QSqlTableModel(this);
    jinglesmodel->setTable("jingles");
    jinglesmodel->select();
    ui->jinglesView->setModel(jinglesmodel);


    /*Populate Pub table*/
    QSqlTableModel *pubmodel = new QSqlTableModel(this);
    pubmodel->setTable("pub");
    pubmodel->select();
    ui->PubView->setModel(pubmodel);


    /*Populate genre1 and 2 filters*/
    QSqlQueryModel * model_genre1=new QSqlQueryModel();
    QSqlQueryModel * model_genre2=new QSqlQueryModel();

    QSqlQuery* qry=new QSqlQuery();

    qry->prepare("select name from genres1");
    qry->exec();
    model_genre1->setQuery(*qry);
    ui->cBoxGenre1->setModel(model_genre1);

    qry->prepare("select name from genres1");
    qry->exec();
    model_genre2->setQuery(*qry);
    ui->cBoxGenre2->setModel(model_genre2);
}


void player::dataReceived(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> > map){

   if(indexcanal>=1){

       indexcanal --;

       if(indexcanal==1){
           double valorCanalDireito = map[Phonon::AudioDataOutput::RightChannel][0] / 10;
         //  qDebug () << "Valor do canal direito: " << abs(valorCanalDireito);

           double valorCanalEsquerdo = map[Phonon::AudioDataOutput::LeftChannel][0] / 10;
         //  qDebug () << "Valor do canal esquerdo: " << abs(valorCanalEsquerdo);



           //Valor da coluna da direita | Right speaker value
            int valorPercentualDireito = abs(((valorCanalDireito*100)/1500));
           if(valorPercentualDireito >= 90 ){
                ui->qvumeter->setRightValue(100);
                if(normalization_soft=="1"){
                                audioOutput->setVolume(1);
                                qDebug() << "Normalizing softly... 1.00";
                }
           } else if(valorPercentualDireito >= 80 && valorPercentualDireito <=89){
               ui->qvumeter->setRightValue(90);
                if(normalization_soft=="1"){
                               audioOutput->setVolume(1.2);
                               ui->qvumeter->setRightValue(92);
                               qDebug() << "Normalizing softly... 1.00";
                }
           } else if(valorPercentualDireito >= 70 && valorPercentualDireito <=79){
               ui->qvumeter->setRightValue(80);
                if(normalization_soft=="1"){
                   audioOutput->setVolume(1.04);
                   ui->qvumeter->setRightValue(90);
                    qDebug() << "Normalizing softly... 1.01";
                }
           } else if(valorPercentualDireito >= 60 && valorPercentualDireito <=69){
               ui->qvumeter->setRightValue(70);

               if(normalization_soft=="1"){
                    audioOutput->setVolume(1.05);
                    ui->qvumeter->setRightValue(80);
                    qDebug() << "Normalizing softly... 1.02";
               }

           } else if(valorPercentualDireito >= 50 && valorPercentualDireito <=59){
               ui->qvumeter->setRightValue(60);

               if(normalization_soft=="1"){
                    audioOutput->setVolume(1.06);
                    ui->qvumeter->setRightValue(70);
                    qDebug() << "Normalizing softly... 1.03";
                }

           } else if(valorPercentualDireito >= 40 && valorPercentualDireito <=49){
               ui->qvumeter->setRightValue(50);

               if(normalization_soft=="1"){
                    audioOutput->setVolume(1.07);
                    ui->qvumeter->setRightValue(60);
                    qDebug() << "Normalizing softly... 1.04";
               }

           } else if(valorPercentualDireito >= 30 && valorPercentualDireito <=39){
               ui->qvumeter->setRightValue(40);

                if(normalization_soft=="1"){
                    audioOutput->setVolume(1.08);
                    ui->qvumeter->setRightValue(50);
                     qDebug() << "Normalizing softly... 1.05";
                }

           } else if(valorPercentualDireito >= 20 && valorPercentualDireito <=29){
               ui->qvumeter->setRightValue(30);

               if(normalization_soft=="1"){
                    audioOutput->setVolume(1.09);
                    ui->qvumeter->setRightValue(40);
                     qDebug() << "Normalizing softly... 1.06";
                }
           } else if(valorPercentualDireito >= 10 && valorPercentualDireito <=19){
               ui->qvumeter->setRightValue(20);

               if(normalization_soft=="1"){
                    audioOutput->setVolume(1.1);
                    ui->qvumeter->setRightValue(30);
                     qDebug() << "Normalizing softly... 1.08";
               }
           } else if(valorPercentualDireito >= 1 && valorPercentualDireito <=9){
               ui->qvumeter->setRightValue(10);

               if(normalization_soft=="1"){
                    audioOutput->setVolume(1.11);
                    ui->qvumeter->setRightValue(20);
                     qDebug() << "Normalizing softly... 1.09";
               }

           } else if(valorPercentualDireito == 0){
               ui->qvumeter->setRightValue(0);
           }





           //Valor da coluna da esquerda | Left speaker value
                      int valorPercentualEsquerdo = abs(((valorCanalEsquerdo*100)/1500));
                     if(valorPercentualEsquerdo >= 90 ){
                          ui->qvumeter->setLeftValue(100);
                          if(normalization_soft=="1"){
                                          audioOutput->setVolume(1);
                                          qDebug() << "Normalizing (left) softly... 1.00";
                          }
                     } else if(valorPercentualEsquerdo >= 80 && valorPercentualEsquerdo <=89){
                         ui->qvumeter->setLeftValue(90);
                          if(normalization_soft=="1"){
                                         audioOutput->setVolume(1.2);
                                         ui->qvumeter->setLeftValue(92);
                                         qDebug() << "Normalizing (left) softly... 1.00";
                          }
                     } else if(valorPercentualEsquerdo >= 70 && valorPercentualEsquerdo <=79){
                         ui->qvumeter->setLeftValue(80);
                          if(normalization_soft=="1"){
                             audioOutput->setVolume(1.04);
                             ui->qvumeter->setLeftValue(90);
                              qDebug() << "Normalizing (left) softly... 1.01";
                          }
                     } else if(valorPercentualEsquerdo >= 60 && valorPercentualEsquerdo <=69){
                         ui->qvumeter->setLeftValue(70);

                         if(normalization_soft=="1"){
                              audioOutput->setVolume(1.05);
                              ui->qvumeter->setLeftValue(80);
                              qDebug() << "Normalizing (left) softly... 1.02";
                         }

                     } else if(valorPercentualEsquerdo >= 50 && valorPercentualEsquerdo <=59){
                         ui->qvumeter->setLeftValue(60);

                         if(normalization_soft=="1"){
                              audioOutput->setVolume(1.06);
                              ui->qvumeter->setLeftValue(70);
                              qDebug() << "Normalizing (left) softly... 1.03";
                          }

                     } else if(valorPercentualEsquerdo >= 40 && valorPercentualEsquerdo <=49){
                         ui->qvumeter->setLeftValue(50);

                         if(normalization_soft=="1"){
                              audioOutput->setVolume(1.07);
                              ui->qvumeter->setLeftValue(60);
                              qDebug() << "Normalizing (left) softly... 1.04";
                         }

                     } else if(valorPercentualEsquerdo >= 30 && valorPercentualEsquerdo <=39){
                         ui->qvumeter->setLeftValue(40);

                          if(normalization_soft=="1"){
                              audioOutput->setVolume(1.08);
                              ui->qvumeter->setLeftValue(50);
                               qDebug() << "Normalizing (left) softly... 1.05";
                          }

                     } else if(valorPercentualEsquerdo >= 20 && valorPercentualEsquerdo <=29){
                         ui->qvumeter->setLeftValue(30);

                         if(normalization_soft=="1"){
                              audioOutput->setVolume(1.09);
                              ui->qvumeter->setLeftValue(40);
                               qDebug() << "Normalizing (left) softly... 1.06";
                          }
                     } else if(valorPercentualEsquerdo >= 10 && valorPercentualEsquerdo <=19){
                         ui->qvumeter->setLeftValue(20);

                         if(normalization_soft=="1"){
                              audioOutput->setVolume(1.1);
                              ui->qvumeter->setLeftValue(30);
                               qDebug() << "Normalizing (left) softly... 1.08";
                         }
                     } else if(valorPercentualEsquerdo >= 1 && valorPercentualEsquerdo <=9){
                         ui->qvumeter->setLeftValue(10);

                         if(normalization_soft=="1"){
                              audioOutput->setVolume(1.11);
                              ui->qvumeter->setLeftValue(20);
                               qDebug() << "Normalizing (left) softly... 1.09";
                         }

                     } else if(valorPercentualEsquerdo == 0){
                         ui->qvumeter->setLeftValue(0);
                     }






           // indexLastDbValue = valorPercentualEsquedo;
          indexcanal = 4;

       }


   }

}


void player::showTime()
{
    QTime time = QTime::currentTime();
    QString text = time.toString("hh:mm");
    QString segundos = time.toString(":ss");
    if ((time.second() % 2) == 0)
        segundos[0] = ' ';
   // display(text);
    //qDebug() << "text is: " << text;
    ui->txt_horas->display(text+segundos);


    //ui->txt_segundos->display(segundos);
}

player::~player()
{
    delete ui;
}

void player::on_pushButton_clicked()
{
    /*Play on Top*/
    QString file = QFileDialog::getOpenFileName(this,tr("Select file to play"));
    music=Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource(file));
    ui->fast_label->setText(file);
}

void player::on_pushButton_2_clicked()
{
    /*Atualizar fast play lable*/
      QString title = music->currentSource().fileName();


        QFileInfo file(title);
        QString baseName = file.baseName();

        ui->fast_label->setText(baseName);
   /*EOF Atualizar fast play lable*/

    music->play();
}

void player::on_pushButton_3_clicked()
{
    music->stop();
}


void player::playPause()
{

    switch (mediaObject->state()){
        case Phonon::PlayingState:
            mediaObject->pause();
            ui->pushButtonPlay->setChecked(false);
            break;
        case Phonon::PausedState:
            mediaObject->play();
            break;
        case Phonon::StoppedState:
            mediaObject->play();
            break;
        case Phonon::LoadingState:
            ui->pushButtonPlay->setChecked(false);
            break;
        case Phonon::BufferingState:
            qDebug() << "Buffering...";
            break;
        case Phonon::ErrorState:
        qDebug() << "Error in Phonon... got ErrorState in PlayPause switch";
            break;

    }

}
void player::addFiles()
{

//browse for files and add them to the playlist without adding to db
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select Music Files"),
             QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

         if (files.isEmpty())
             return;

         QMessageBox::StandardButton reply;
         QString normalizeIt = "no";
         if(ask_normalize_new_files=="true"){
             reply = QMessageBox::question(this,"Normalize also?","Do you want to normalize the audio of the imported tracks? (this ensures a great music quality and a constant volume output level, but it's much slower.)",QMessageBox::Yes|QMessageBox::No);
             if(reply==QMessageBox::Yes){
                 normalizeIt = "yes";
             }
         }


         //int index = sources.size();
         foreach (QString string, files) {
             if(normalizeIt=="yes"){
                 qDebug()<<"SoX is working...";
                 QString tmpname = "/home/fred/cpp/music/tmp.mp3";
                 QProcess sh;
                 qDebug()<<"RUNNING CMD: sox "<<string<<" "<<tmpname;
                 //sh.start("sh",QStringList()<<"-c"<<"sox "+string+" "+tmpname);
                 sh.start("sh",QStringList()<<"sox "+string+" "+tmpname);
                 sh.waitForFinished();
                 qDebug()<<"File was normalized: "<<string;
              }
            ui->list_playlist->addItem(string);

         }

}
void player::nextFile()
{
    int numOfItemsInPlaylist = ui->list_playlist->count();
    if(numOfItemsInPlaylist>0){
        qDebug()<<"850 asking for next file..";
        QString filepath = ui->list_playlist->item(0)->text();
        Phonon::MediaSource source(filepath);
        sources.append(source);
        int index = sources.indexOf(mediaObject->currentSource()) + 1;
        mediaObject->enqueue(sources.at(index));



        ui->timeLcd->display("00:00");

        indexSourceChanged=0;
        mediaObject->setCurrentSource(sources.at(index));
        mediaObject->play();

        QDateTime now = QDateTime::currentDateTime();
        QString text = now.toString("yyyy-MM-dd || hh:mm:ss ||");
        QString title = mediaObject->currentSource().fileName();
        QFileInfo file(title);
        QString baseName = file.fileName();
        ui->nowPlaying->setText(baseName);

        QString linewithdate = text + " " + baseName;
        qDebug () << "line with date for history is: " << linewithdate;
        ui->historylist->addItem(linewithdate); //adiciona à historylist
        delete ui->list_playlist->item(0); //apagar da playlist


    } else {
        qDebug()<<"871 asking for next file but nothing to play..";
      }
}

void player::aboutToFinish()
{

    qDebug()<<("Launched aboutToFinish function");

    int numNaPlaylist = ui->list_playlist->count();

    if(numNaPlaylist>=1){
        QString filepath = ui->list_playlist->item(0)->text();
        Phonon::MediaSource source(filepath);
        sources.append(source);
        int index = sources.indexOf(mediaObject->currentSource()) + 1;
        mediaObject->enqueue(sources.at(index));

        QDateTime now = QDateTime::currentDateTime();
        QString text = now.toString("yyyy-MM-dd || hh:mm:ss ||");

        ui->timeLcd->display("00:00");

        QString title = mediaObject->currentSource().fileName();
        QFileInfo file(title);
        QString baseName = file.fileName();
        ui->nowPlaying->setText(baseName);

        QString linewithdate = text + " " + baseName;
        qDebug () << "line with date for history is: " << linewithdate;
        ui->historylist->addItem(linewithdate); //adiciona à historylist
        delete ui->list_playlist->item(0); //apagar da playlist

        indexSourceChanged=0;
    }
     else {
        qDebug()<<"No more files in queue to play! (aboutToFinish function)";

        if(autoMode==1){
            qDebug()<<"autoMode is ON, seeking for more musics to play...";

            autoModeGetMoreSongs(); //function that deals with choosing music for autoMode

        } else {
            qDebug()<<"autoMode is OFF, so stopping...";
            ui->pushButtonPlay->setChecked(false);
        }


    }




}

void player::finished()
{

    if(autoMode == 0){
         ui->pushButtonPlay->setChecked(false);
         playing=0;
         mediaObject->stop();
         qDebug()<<"Player Stopped. No more files in queue to play. autoMode is OFF.";
    } else {
                int index = sources.indexOf(mediaObject->currentSource())+1;
                mediaObject->setCurrentSource(sources.at(index));
                mediaObject->play();
                qDebug() << "Let's autoRock!";
    }

}
void player::stateChanged(Phonon::State newState, Phonon::State /* oldState */)
 {
     switch (newState) {
         case Phonon::ErrorState:
             if (mediaObject->errorType() == Phonon::FatalError) {
                 QMessageBox::warning(this, tr("State: Fatal Error 201503271649"),
                 mediaObject->errorString());
             } else {
                 QMessageBox::warning(this, tr("State: Error 201503271650"),
                 mediaObject->errorString());
             }
             break;
           case Phonon::PlayingState:
                        qDebug()<<"State: Now Playing!!";
                        playing=1;
                        ui->pushButtonPlay->setText("Pause");
                     break;
             case Phonon::StoppedState:
                        qDebug()<<("State: Stopped!!");
                        playing=0;
                        ui->timeLcd->display("00:00");
                        ui->pushButtonPlay->setText("Play");
                        ui->qvumeter->setRightValue(0);
                        ui->qvumeter->setLeftValue(0);

                     break;
                 case Phonon::PausedState:
                        qDebug()<<"Paused!!";
                        playing=2;
                        ui->pushButtonPlay->setText("Play");
                     break;
     case Phonon::BufferingState:
         qDebug() << "Filling up buffers...";
         break;
     case Phonon::LoadingState:
         qDebug() << "Loading...";
         break;
     }
}

void player::tick(qint64 time)
{
    QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);

    ui->timeLcd->display(displayTime.toString("mm:ss"));
    //setVolume();

}


void player::sourceChanged(const Phonon::MediaSource &source)
{
    //bool wasPlaying = mediaObject->state() == Phonon::PlayingState;
    qDebug() << "SOURCE CHANGED: " << &source;

if(indexSourceChanged==1){

    QDateTime now = QDateTime::currentDateTime();
    QString text = now.toString("yyyy-MM-dd || hh:mm:ss ||");

    ui->timeLcd->display("00:00");

    QString title = mediaObject->currentSource().fileName();
    QFileInfo file(title);
    QString baseName = file.fileName();
    ui->nowPlaying->setText(baseName);

    QString linewithdate = text + " " + baseName;
    qDebug () << "line with date for history is: " << linewithdate;
    ui->historylist->addItem(linewithdate); //adiciona à historylist
    delete ui->list_playlist->item(0); //apagar da playlist

    indexSourceChanged=0;
} else {

    indexSourceChanged++;
}


}

void player::on_pushButtonPlay_clicked()
{  
    //main play bt


    //if stopped
    if(playing==0){
        int numOfItemsInPlaylist = ui->list_playlist->count();
        if(numOfItemsInPlaylist>0){
        int index = sources.indexOf(mediaObject->currentSource())+1;

        //qDebug() << "index is: " << index;
       // qDebug() << "Current source is: " << cursource;
        if (sources.size() > index) {
            //se houver faixas pra tocar na lista de reprodução...
             mediaObject->stop();
             mediaObject->setCurrentSource(sources.at(index));
             mediaObject->play();
             playing=1;
             qDebug() << "I RAN FOR A 100 MILES TO GET TO YOU AND STILL ...";

         } else {
            qDebug() << "sources.size is less or equal to the index of mediaObject+1";

            int num_of_rows = ui->list_playlist->count();

            qDebug() << "num_of_rows: " << num_of_rows;

            if (num_of_rows>0){

                QString filepath = ui->list_playlist->item(0)->text();
                qDebug() << "201527031639 Dealing with file: " << filepath;

                Phonon::MediaSource source(filepath);
                sources.append(source);
                int index = sources.indexOf(mediaObject->currentSource())+1;
                mediaObject->setCurrentSource(sources.at(index));
                mediaObject->play();

                QDateTime now = QDateTime::currentDateTime();
                QString text = now.toString("yyyy-MM-dd || hh:mm:ss ||");

                ui->timeLcd->display("00:00");

                QString title = mediaObject->currentSource().fileName();
                QFileInfo file(title);
                QString baseName = file.fileName();
                ui->nowPlaying->setText(baseName);

                QString linewithdate = text + " " + baseName;
                qDebug () << "line with date for history is: " << linewithdate;
                ui->historylist->addItem(linewithdate); //adiciona à historylist
                delete ui->list_playlist->item(0); //apagar da playlist

                indexSourceChanged=0;


            }


        }

    } else {


        if(playing==1){
            qDebug()<<"Play_bt if playing==1";
            playing=2;
            mediaObject->pause();
        } else {

                //if paused
                qDebug()<<"Play_bt if playing==2";
                if(playing==2){
                    playing=1;
                    mediaObject->play();
                }

            }

    }

    }

}

void player::stopAction()
{
    playing=0;
    mediaObject->stop();
}


void player::on_pushButton_4_clicked()
{
    /*
    audioRecorder = new QAudioRecorder;

    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/mp3");
    audioSettings.setQuality(QMultimedia::HighQuality);

    audioRecorder->setEncodingSettings(audioSettings);

    audioRecorder->setOutputLocation(QUrl::fromLocalFile("test.mp3"));

    audioProbe = new QAudioProbe(this);
    if (audioProbe->setSource(audioRecorder)) {
        // Probing succeeded, audioProbe->isValid() should be true.
        connect(audioProbe, SIGNAL(audioBufferProbed(QAudioBuffer)),
                this, SLOT(calculateLevel(QAudioBuffer)));

    }

    audioRecorder->record();
    // Now audio buffers being recorded should be signaled
    // by the probe, so we can do things like calculating the
    // audio power level, or performing a frequency transform
    */
}

void player::on_actionAdd_Music_triggered()
{
    add_music_single add_music_single;
    add_music_single.setModal(true);
    add_music_single.exec();
    update_music_table();
}



void player::dropEvent(QDropEvent *event)
{

    player *source =
            qobject_cast<player *>(event->source());

        if(xaction == "drag_to_music_playlist"){
            qDebug() << "drop MUSIC event! " << estevalor << " xaction: " << xaction;// << "evnt source: " << source->objectName();
            //qDebug () << "event mime data" << event->mimeData();
            if(source->objectName()=="player"){
            //sources.append(estevalor);
            ui->list_playlist->addItem(estevalor);

            }
            xaction = "";

        } else {
            //qDebug () << "xaction is not defined or is not 'drag_to_music_playlist' .. its content is: " << xaction;
        }


       event->acceptProposedAction();


}

void player::dragEnterEvent(QDragEnterEvent *event)
 {
    //qDebug() << "drag enter event " << event << event->mimeData();
    if(indexJust3rdDropEvt==1){
        //qDebug () << "This is the 2nd interaction and we now accepted the proposed action.";
        event->acceptProposedAction();
        indexJust3rdDropEvt = 0;
    } else{
        indexJust3rdDropEvt++;
    }




 }


void player::on_musicView_pressed(const QModelIndex &index)
{

indexJust3rdDropEvt=0;

    //int thisid = index.row()+1;
    ui->musicView->selectRow(index.row());
    int rowidx = ui->musicView->selectionModel()->currentIndex().row();

    //QSqlTableModel * model = new QSqlTableModel(this);
    //model->setTable("musics");
   // model->select();
    //QString sqlTableId = model->index(rowidx , 0).data().toString();
    //qDebug()<< sqlTableId;

    //TO
    //aqui poderiamos evitar o query via estevalor = model->index(rowidx,7).data().toString();



    //QSqlQuery qry;
    //qry.prepare("select path from musics where id=:id");
    //qry.bindValue(":id",sqlTableId);

   // if(qry.exec()){
       // while(qry.next()){
            //qDebug() << qry.value(0).toString();
            //estevalor = qry.value(0).toString();
            //estevalor = model->index(rowidx,7).data().toString();
    estevalor = ui->musicView->model()->data(ui->musicView->model()->index(rowidx,7)).toString();
    qDebug () << "este valor: [" << estevalor << "]";

            xaction = "drag_to_music_playlist";
            //check if file exists and avoid adding if it does not
            bool ha = QFile::exists (estevalor);
            if(!ha){

                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, "The file does NOT exist?", "It seams like the file does NOT exist on the hard drive... Should it be deleted from the database?",
                                              QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                  qDebug() << "the file should be deleted from the database cause it does not exist in the hd (or path should change)";
                      QSqlQuery* qry=new QSqlQuery();
                      qry->prepare("delete from musics where path = :thpath");
                      qry->bindValue(":thpath",estevalor);

                     if(qry->exec()){
                          qDebug() << "Music Deleted form database! last query was:"<< qry->lastQuery();
                          update_music_table();
                     } else {
                         qDebug() << "There was an error deleting the music from the database"<< qry->lastError() << qry->lastQuery();
                     }
                } else {
                  qDebug() << "keeping invalid record in db... please fix path..";
                }

            }

     //   }
  //  } else {
   //     qDebug() << "erro: " << qry.lastError();
   // }


    //qDebug() << "Pressed row is: " << index.row() << " and QVariant valor is: " << valor.toString();
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setData(text, "drag_to_music_playlist");
    drag->setMimeData(mimeData);
    Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
    qDebug() << "DropAction near 1081 has: " << dropAction;
}

void player::on_actionConfigurations_triggered()
{

    optionsDialog optionsDialog;
    optionsDialog.setModal(true);
    optionsDialog.exec();

}


void player::on_actionBuy_me_a_Coffe_triggered()
{
    Dialog Dialog;
    Dialog.setModal(true);
    Dialog.exec();

}

void player::on_actionManage_Genres_triggered()
{
    addgenre addgenre;
    addgenre.setModal(true);
    addgenre.exec();
}

void player::on_actionAdd_Jingle_triggered()
{
    addJingle addJingle;
    addJingle.setModal(true);
    addJingle.exec();
}

void player::on_list_playlist_clicked(const QModelIndex &index)
{
    //ui->musicView->selectRow(index.row());
   // QVariant valor = ui->musicView->model()->data(index,0);
    qDebug() << "Pressed row in MUSIC PLAYLIST (list_playlist) is: " << index.row();
}

void player::on_actionExit_triggered()
{
   QMessageBox::StandardButton reply;
   reply = QMessageBox::question(this,"Sure?","Are you sure you want to exit?",QMessageBox::Yes|QMessageBox::No);
   if(reply==QMessageBox::Yes){
        exit(0);
   }

}

void player::on_pushButton_5_clicked()
{
    update_music_table();
}

void player::on_jinglesView_pressed(const QModelIndex &index)
{
    indexJust3rdDropEvt=0;

        //int thisid = index.row()+1;
        ui->jinglesView->selectRow(index.row());
        int rowidx = ui->jinglesView->selectionModel()->currentIndex().row();

        QSqlTableModel * model = new QSqlTableModel(this);
        model->setTable("jingles");
        model->select();
        QString sqlPath = model->index(rowidx , 1).data().toString();
        qDebug()<< sqlPath;
        xaction = "drag_to_music_playlist";
        estevalor = sqlPath;
                //check if file exists and avoid adding if it does not
                bool ha = QFile::exists (sqlPath);
                if(!ha){

                    QMessageBox::StandardButton reply;
                    reply = QMessageBox::question(this, "The file does NOT exist?", "It seams like the file does NOT exist on the hard drive... Should it be deleted from the database?",
                                                  QMessageBox::Yes|QMessageBox::No);
                    if (reply == QMessageBox::Yes) {
                      qDebug() << "the file should be deleted from the database cause it does not exist in the hd (or path should change)";
                          QSqlQuery* qry=new QSqlQuery();
                          qry->prepare("delete from jingles where path = :thpath");
                          qry->bindValue(":thpath",sqlPath);

                         if(qry->exec()){
                              qDebug() << "Music Deleted form database! last query was:"<< qry->lastQuery();
                              update_music_table();
                         } else {
                             qDebug() << "There was an error deleting the music from the database"<< qry->lastError() << qry->lastQuery();
                         }
                    } else {
                      qDebug() << "keeping invalid record in db... please fix path..";
                    }

                }





        //qDebug() << "Pressed row is: " << index.row() << " and QVariant valor is: " << valor.toString();
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setData(text, "drag_to_music_playlist");
        drag->setMimeData(mimeData);
        Qt::DropAction dropAction = drag->exec(Qt::CopyAction);

}

void player::on_pushButton_6_clicked()
{
    //search
    qDebug()<<"Staring a new search!";
    QString term = ui->txt_search->text();

    QSqlQueryModel * model = new QSqlQueryModel();
    model->setQuery("select * from musics where artist like '%"+term+"%' or song like '%"+term+"%'");
    ui->musicView->setModel(model);


}

void player::on_pushButton_7_clicked()
{
    //filter by genres

    QString addG1 = "";
    QString addG2 = "";

    bool g1_checked = ui->checkBox_filter_genre1->checkState(); //true or false
    bool g2_checked = ui->checkBox_filter_genre2->checkState();

    qDebug () << "132426032015 " << g1_checked << " : " << g2_checked;

    if(g1_checked == true){
        qDebug()<<"g1 is checked";
        QString selectedGenre1 = ui->cBoxGenre1->currentText();
        addG1 = " genre1='"+selectedGenre1+"' ";
    }
    if(g2_checked == true){
        qDebug()<<"g2 is checked";
        QString selectedGenre2 = ui->cBoxGenre2->currentText();
        if(g1_checked==true){
             qDebug()<<"both are checked...";
             addG2 = "and genre2='"+selectedGenre2+"' ";
        } else{
            qDebug()<<"Only g2 is checked";
            addG2 = " genre2='"+selectedGenre2+"' ";
        }

    }
    qDebug()<<"addG1 is "<<addG1<<" and addG2 is "<<addG2;
    if(addG1 != "" || addG2 != ""){
        qDebug() << "making a new table to show with the results...";
        QSqlQueryModel * model = new QSqlQueryModel();
        model->setQuery("select * from musics where "+addG1+addG2);
        ui->musicView->setModel(model);
    }




}

void player::on_actionAdd_Publicity_triggered()
{
    add_pub add_pub;
    add_pub.setModal(true);
    add_pub.exec();
}


void player::on_actionSet_active_database_triggered()
{

   // QString file = QFileDialog::getOpenFileName(this, tr("Select database..."), " ", tr("Database files(*.db)"));
/*
    QFile settings ("/home/fred/cpp/AudioEx/settings.conf");
    if (!settings.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

        QTextStream in(&settings);
        while (!in.atEnd()) {
            QString line = in.readLine();
            qDebug() << line;
        }
        */
}

void player::on_bt_video_download_clicked()
{

    youtubedownloader* widget = new youtubedownloader;
    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->show();
}

void player::on_bt_autoMode_clicked()
{
    if(autoMode == 0){
        autoMode = 1;
        qDebug()<<"autoMode is ON";
        ui->bt_autoMode->setStyleSheet("background-color: rgb(175, 227, 59)");
    } else {
        autoMode = 0;
        qDebug()<<"autoMode is OFF";
        ui->bt_autoMode->setStyleSheet("");
    }
}

void player::autoModeGetMoreSongs()
{
    //randomly select music from db
    int numMusics = 1;
    QSqlQuery query;
    query.prepare("select path from musics order by random() limit :numMusics");
    query.bindValue(":numMusics", numMusics);
    if(query.exec())
    {
        qDebug() << "SQL query executed: " << query.lastQuery();

        while(query.next()){
            QString path = query.value(0).toString();
            ui->list_playlist->addItem(path);
            qDebug() << "autoMode random music chooser adding: " << path;
            int numNaPlaylist = ui->list_playlist->count();

            if(numNaPlaylist>=1){
                QString filepath = ui->list_playlist->item(0)->text();
                Phonon::MediaSource source(filepath);
                sources.append(source);
                int index = sources.indexOf(mediaObject->currentSource()) + 1;
                mediaObject->enqueue(sources.at(index));
            }
        }

    } else {
        qDebug() << "SQL ERROR: " << query.lastError();
        qDebug() << "SQL was: " << query.lastQuery();

        //todo ... what should I do now? this should at least go to a log or something...

    }

}

void player::on_stopAction_clicked()
{

}
void player::actionAdd_a_directory()
{

}
void player::on_addFiles_clicked()
{

}

void player::on_actionAdd_a_directory_triggered()
{
     qDebug()<<"201503272128 add a full dir";

     QMessageBox::StandardButton reply;
     QString normalizeIt = "no";
     if(ask_normalize_new_files=="true"){
         reply = QMessageBox::question(this,"Normalize also?","Do you want to normalize the audio of the imported tracks? (this ensures a great music quality and a constant volume output level, but it's much slower.)",QMessageBox::Yes|QMessageBox::No);
         if(reply==QMessageBox::Yes){
             normalizeIt = "yes";
         }
     }

     QString dir = QFileDialog::getExistingDirectory(this, tr("Select a directory to import"));

     qDebug()<<"Selected a dir to import: "<<dir;

      QDirIterator it(dir, QStringList() << "*.mp3"<<"*.mp4"<<"*.ogg"<<"*.wav"<<"*.flac", QDir::Files, QDirIterator::NoIteratorFlags);
      while (it.hasNext()) {

          QString filewpath = it.next();
          qDebug() << "201503281351: " << filewpath;

          if(normalizeIt=="yes"){
              qDebug()<<"SoX is working...";
              QProcess sh;
              sh.start("sh",QStringList()<<"-c"<<"sox --norm -S "+filewpath+" "+filewpath);
              sh.waitForFinished();
              qDebug()<<"File was normalized: "<<filewpath;
           }

          QString nomeficheiro = QDir(dir).relativeFilePath(filewpath);

          //nomeficheiro.replace("_"," ");

          QStringList divide_nome = nomeficheiro.split( "-" );
          QString artist = divide_nome.value(0).replace("_"," ").replace(".mp3","").replace(".mp4","").replace(".ogg","").replace(".wav","").replace(".flac","").trimmed();
          QString song = divide_nome.value(1).replace("_"," ").replace(".mp3","").replace(".mp4","").replace(".ogg","").replace(".wav","").replace(".flac","").trimmed();
          if(song.isEmpty()){
              song="-";
          }
       QString g1 = "Default";
       QString g2 = "Default";

       QString country = "Other country / language";

       QString pub_date = "-";

       qDebug() << "Got! artist: "<<artist<<" and song: "<<song<<" from file: "<<nomeficheiro;



        //check if it's already in db

       int dbhasmusic=0;
       QSqlQuery query;
       query.prepare("SELECT path FROM musics WHERE path=:path");
       query.bindValue(":path",filewpath);
       query.exec();
       while (query.next()){
           QString thispath = query.value(0).toString();
           qDebug() << "Skipping: "<<thispath;
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
           sql.bindValue(":file",filewpath);

           if(sql.exec())
           {
               qDebug() << "last sql: " << sql.lastQuery();
           } else {
               QMessageBox::critical(this,tr("Error"),sql.lastError().text());
               qDebug() << "last sql: " << sql.lastQuery();
           }


           //this->hide();
       }






      }
 QMessageBox::information(this,tr("Add directory"),tr("All done! Have a nice day!"));
 update_music_table();
}


void player::on_actionSave_Playlist_triggered()
{
    qDebug()<<"Saving the playlist...";

    QString filename = QFileDialog::getSaveFileName(this,"Save playlist","../playlists/","XML files (*.xml)");

    if(!filename.isEmpty()){
        qDebug()<<"saving "<<filename;


        QFile file(filename);
        file.open(QIODevice::WriteOnly);

        QXmlStreamWriter xmlWriter(&file);
        xmlWriter.setAutoFormatting(true);
        xmlWriter.writeStartDocument();

        xmlWriter.writeStartElement("AudioXPlaylist");
        xmlWriter.writeStartElement("www.netpack.pt");


        //loop playlist and save every line into xml
            int numItems = ui->list_playlist->count();
            for(int i=0;i<numItems;i++){
               //qDebug()<<"i is: "<<i;
               QString txtItem = ui->list_playlist->item(i)->text();
               qDebug()<<"Xml adding file "<<txtItem;

               xmlWriter.writeTextElement("track",txtItem);
            }

        xmlWriter.writeEndElement();
        xmlWriter.writeEndDocument();
        file.close();
        QMessageBox::information(this,"Playlist Saved","The playlist was saved!");
    }


}

void player::on_actionClear_Playlist_triggered()
{
    QMessageBox::StandardButton reply;
    reply=QMessageBox::question(this,"Sure?","Are you sure? This will clear all tracks listed in the playlist.",QMessageBox::Yes|QMessageBox::No);
    if(reply==QMessageBox::Yes){
        ui->list_playlist->clear();
    }
}

void player::on_actionLoad_Playlist_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,"Load Playlist","../playlists/",tr("Xml files(*.xml)"));
    QFile file(filename);
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        QMessageBox::information(this,"Bad input","There was an error importing the file.. sorry.");
    }

    QXmlStreamReader Rxml;
    Rxml.setDevice(&file);
    Rxml.readNext();

    while(!Rxml.atEnd()){

        if (Rxml.isStartElement()) {

                    if (Rxml.name() == "AudioXPlaylist") {
                        qDebug()<<"Valid AudioX Playlist Found!";
                        Rxml.readNext();

                        if(Rxml.isEndElement()){
                            qDebug()<<"Found the last element of the XML file after StarElement, leaving the while loop";
                            Rxml.readNext();
                            break;
                        }

                    } else {
                        Rxml.raiseError(QObject::tr("Not an AudioX playlist file"));
                    }
                } else {
                    Rxml.readNext();

                    //qDebug()<<"This Rxml.name() is "<<Rxml.name();

                    if(Rxml.name()=="www.netpack.pt"){
                        qDebug()<<"Token element: "<<Rxml.name();
                        Rxml.readNext();
                    }

                    if(Rxml.name()=="track"){
                        QString track = Rxml.readElementText();
                        qDebug()<<"Rxml.readElementText(): "<<track;
                        ui->list_playlist->addItem(track);
                    }



                }
    }
    file.close();

}
