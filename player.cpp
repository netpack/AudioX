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
#include "aex_main.h"
#include <QFileDialog>
#include <QFile>
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


/*

  TODO?: Try to send mediaObject to high priority tread

*/


player::player(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::player)
{



    /*main music*/
 audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
 mediaObject = new Phonon::MediaObject(this);

 metaInformationResolver = new Phonon::MediaObject(this);
 mediaObject->setTickInterval(25);
 //mediaObject->setTransitionTime(-3000);
/*Left and right speakers vumeter*/

 dataout = new Phonon::AudioDataOutput(this);
 Phonon::createPath(mediaObject, dataout);
 connect( dataout, SIGNAL(dataReady(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >)),
         this, SLOT(dataReceived(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >)));



    /*fast loading music*/
    music=new Phonon::MediaObject(this);
    Phonon::createPath(mediaObject,audioOutput);


    /*Main music signals and slots*/
    connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),this, SLOT(stateChanged(Phonon::State,Phonon::State)));
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),this, SLOT(sourceChanged(Phonon::MediaSource)));
    connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(aboutToFinish()));

    /*relogio - clock*/
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
    timer->start(1000);
    /*eof relogio*/

    /*Scheduler*/
    QTimer *schedulerTimer = new QTimer(this);
    connect(schedulerTimer,SIGNAL(timeout()),this,SLOT(run_scheduler()));
    schedulerTimer->start(60000);


    ui->setupUi(this); // setup the ui

    setWindowTitle("AudioX");

    setWindowIcon(QIcon("./48x48.png"));
    showTime();

    ui->seekSlider->setMediaObject(mediaObject);
    ui->volumeSlider->setAudioOutput(audioOutput);

    ui->webView->load(QUrl("http://youtube.com"));

    /*connect to db*/
    if(!adb.isOpen()){
        qDebug()<<"opening db form 88 in player.cpp";
            adb=QSqlDatabase::addDatabase("QSQLITE");
            adb.setDatabaseName("/home/fred/adb.db");
            adb.open();
    }
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



    /*Drag & Drop Set*/
    /*player*/
     this->setAcceptDrops(true);
    /*playlist (listWidget)*/
     ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
     ui->listWidget->setDragEnabled(true);
     ui->listWidget->viewport()->setAcceptDrops(true);
     ui->listWidget->setAcceptDrops(true);
     ui->listWidget->setDropIndicatorShown(true);
     ui->listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    /*Music list*/
     ui->musicView->setSelectionMode(QAbstractItemView::SingleSelection);
     ui->musicView->setDragEnabled(true);
     ui->musicView->viewport()->setAcceptDrops(false);
     ui->musicView->setAcceptDrops(false);
     ui->musicView->setDropIndicatorShown(false);
     ui->musicView->setDragDropMode(QAbstractItemView::DragOnly);
     ui->musicView->setSelectionBehavior(QAbstractItemView::SelectRows);
     ui->musicView->setSelectionMode(QAbstractItemView::SingleSelection);


    indexSourceChanged = 0; //index p correr sourceChanged apenas uma vez devido a bug vindo do connect p o vumeter
    indexcanal = 4; //index p atualização do volume de qvumeter
    indexJust3rdDropEvt = 0; //index p aceitar drop so ao 2º loop visto ser chamado 2 vezes sendo a ultima quando entramos em cima da listView

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

                qDebug()<<"Scheduler is comparing values: ";
                qDebug()<<ano1<<" should be equal to "<<ano2<<" -> ano";
                qDebug()<<mes1<<" should be equal to "<<mes2<<" -> mes";
                qDebug()<<dia1<<" should be equal to "<<dia2<<" -> dia";
                qDebug()<<hora1<<" should be equal to "<<hora2<<" -> hora";
                qDebug()<<min1<<" should be equal to "<<min2<<" -> minuto";

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
                            ui->listWidget->insertItem(0,pubPath);
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

                //add to playlist
        }

    }
}

}

void player::checkDbOpen(){
    /*connect to db*/
        if(!adb.isOpen()){
            qDebug()<<"opening db form checkDbOpen in player.cpp";
                adb=QSqlDatabase::addDatabase("QSQLITE");
                adb.setDatabaseName("/home/fred/adb.db");
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
          // qDebug () << "Valor do canal direito: " << abs(valorCanalDireito);

           double valorCanalEsquerdo = map[Phonon::AudioDataOutput::LeftChannel][0] / 10;
         //  qDebug () << "Valor do canal esquerdo: " << abs(valorCanalEsquerdo);



           //Valor da coluna da direita | Right speaker value
            int valorPercentualDireito = abs(((valorCanalDireito*100)/1500));
           if(valorPercentualDireito >= 90 ){
                ui->qvumeter->setRightValue(100);
           } else if(valorPercentualDireito >= 80 && valorPercentualDireito <=89){
               ui->qvumeter->setRightValue(90);
           } else if(valorPercentualDireito >= 70 && valorPercentualDireito <=79){
               ui->qvumeter->setRightValue(80);
           } else if(valorPercentualDireito >= 60 && valorPercentualDireito <=69){
               ui->qvumeter->setRightValue(70);
           } else if(valorPercentualDireito >= 50 && valorPercentualDireito <=59){
               ui->qvumeter->setRightValue(60);
           } else if(valorPercentualDireito >= 40 && valorPercentualDireito <=49){
               ui->qvumeter->setRightValue(50);
           } else if(valorPercentualDireito >= 30 && valorPercentualDireito <=39){
               ui->qvumeter->setRightValue(40);
           } else if(valorPercentualDireito >= 20 && valorPercentualDireito <=29){
               ui->qvumeter->setRightValue(30);
           } else if(valorPercentualDireito >= 10 && valorPercentualDireito <=19){
               ui->qvumeter->setRightValue(20);
           } else if(valorPercentualDireito >= 1 && valorPercentualDireito <=9){
               ui->qvumeter->setRightValue(10);
           } else if(valorPercentualDireito == 0){
               ui->qvumeter->setRightValue(0);
           }


           //Valor da coluna da esquerda | Left speaker value

                int valorPercentualEsquedo = abs(((valorCanalEsquerdo*100)/1500));
               // qDebug () << "Valor P-E : "<<valorPercentualEsquedo;
                if(valorPercentualEsquedo >= 90 ){
                     ui->qvumeter->setLeftValue(100);
                } else if(valorPercentualEsquedo >= 80 && valorPercentualEsquedo <=89){
                    ui->qvumeter->setLeftValue(90);
                } else if(valorPercentualEsquedo >= 70 && valorPercentualEsquedo <=79){
                    ui->qvumeter->setLeftValue(80);
                } else if(valorPercentualEsquedo >= 60 && valorPercentualEsquedo <=69){
                    ui->qvumeter->setLeftValue(70);
                } else if(valorPercentualEsquedo >= 50 && valorPercentualEsquedo <=59){
                    ui->qvumeter->setLeftValue(60);
                } else if(valorPercentualEsquedo >= 40 && valorPercentualEsquedo <=49){
                    ui->qvumeter->setLeftValue(50);
                } else if(valorPercentualEsquedo >= 30 && valorPercentualEsquedo <=39){
                    ui->qvumeter->setLeftValue(40);
                } else if(valorPercentualEsquedo >= 20 && valorPercentualEsquedo <=29){
                    ui->qvumeter->setLeftValue(30);
                } else if(valorPercentualEsquedo >= 10 && valorPercentualEsquedo <=19){
                    ui->qvumeter->setLeftValue(20);
                } else if(valorPercentualEsquedo >= 1 && valorPercentualEsquedo <=9){
                    ui->qvumeter->setLeftValue(10);
                } else if(valorPercentualEsquedo == 0){
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
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select Music Files"),
             QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

         if (files.isEmpty())
             return;

         //int index = sources.size();
         foreach (QString string, files) {

            ui->listWidget->addItem(string);

         }

}
void player::nextFile()
{
    QString filepath = ui->listWidget->item(0)->text();
    Phonon::MediaSource source(filepath);
    sources.append(source);

    int index = sources.indexOf(mediaObject->currentSource()) + 1;

    if (sources.size() > index) {
         mediaObject->stop();
         mediaObject->setCurrentSource(sources.at(index));
         mediaObject->play();

     } else {
        qDebug() << "sources.size is less or equal to the index of mediaObject+1 (at 395)";
        QString filepath = ui->listWidget->item(0)->text();
        Phonon::MediaSource source(filepath);
        sources.append(source);
        int index = sources.indexOf(mediaObject->currentSource())+1;
        mediaObject->setCurrentSource(sources.at(index));
        mediaObject->play();
    }
}

void player::aboutToFinish()
{

    qDebug()<<("Launched aboutToFinish function");

    int numNaPlaylist = ui->listWidget->count();

    if(numNaPlaylist>=1){
        QString filepath = ui->listWidget->item(0)->text();
        Phonon::MediaSource source(filepath);
        sources.append(source);
        int index = sources.indexOf(mediaObject->currentSource()) + 1;
        mediaObject->enqueue(sources.at(index));
    }
     else {
        qDebug()<<"No more files in queue to play!.. so Stoping..(aboutToFinish function)";
        ui->pushButtonPlay->setChecked(false);
    }




}

void player::finished()
{

         ui->pushButtonPlay->setChecked(false);
         qDebug()<<"Player Stoped. No more files in queue to play.";

}
void player::stateChanged(Phonon::State newState, Phonon::State /* oldState */)
 {
     switch (newState) {
         case Phonon::ErrorState:
             if (mediaObject->errorType() == Phonon::FatalError) {
                 QMessageBox::warning(this, tr("Fatal Error"),
                 mediaObject->errorString());
             } else {
                 QMessageBox::warning(this, tr("Error"),
                 mediaObject->errorString());
             }
             break;
           case Phonon::PlayingState:
                        qDebug()<<"Now Playing!!";
                        ui->pushButtonPlay->setText("Pause");
                     break;
             case Phonon::StoppedState:
                        qDebug()<<("Stoped!!");
                        ui->timeLcd->display("00:00");
                        ui->pushButtonPlay->setText("Play");
                        ui->qvumeter->setRightValue(0);
                        ui->qvumeter->setLeftValue(0);
                     break;
                 case Phonon::PausedState:
                        qDebug()<<"Paused!!";
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
    qDebug() << "SOURCE CHANGED !";// and is now at: " << &source << ". wasPlaying bool is: " << wasPlaying;

if(indexSourceChanged==1){

    QDateTime now = QDateTime::currentDateTime();
    QString text = now.toString("yyyy-MM-dd || hh:mm:ss ||");

    ui->timeLcd->display("00:00");

    QString title = mediaObject->currentSource().fileName();
    QFileInfo file(title);
    QString baseName = file.baseName();
    ui->nowPlaying->setText(baseName);

    QString linewithdate = text + " " + baseName;
    qDebug () << "line with date for history is: " << linewithdate;
    ui->historylist->addItem(linewithdate); //adiciona à historylist
    delete ui->listWidget->item(0); //apagar da playlist

    indexSourceChanged=0;
} else {

    indexSourceChanged++;
}


}

void player::on_pushButtonPlay_clicked()
{  
    //main play bt

    int index = sources.indexOf(mediaObject->currentSource())+1;

    //qDebug() << "index is: " << index;
   // qDebug() << "Current source is: " << cursource;
    if (sources.size() > index) {
         mediaObject->stop();
         mediaObject->setCurrentSource(sources.at(index));
         mediaObject->play();

     } else {
        qDebug() << "sources.size is less or equal to the index of mediaObject+1 // or the file does not exist but is in db..";


        QString filepath = ui->listWidget->item(0)->text();
        Phonon::MediaSource source(filepath);
        sources.append(source);
        int index = sources.indexOf(mediaObject->currentSource())+1;
        mediaObject->setCurrentSource(sources.at(index));
        mediaObject->play();
    }
}

void player::stopAction()
{
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
            ui->listWidget->addItem(estevalor);

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

    QSqlTableModel * model = new QSqlTableModel(this);
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

}

void player::on_actionConfigurations_triggered()
{
    Aex_main Aex_main;
    Aex_main.setModal(true);
    Aex_main.exec();
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

void player::on_listWidget_clicked(const QModelIndex &index)
{
    //ui->musicView->selectRow(index.row());
   // QVariant valor = ui->musicView->model()->data(index,0);
    qDebug() << "Pressed row in MUSIC PLAYLIST (listWidget) is: " << index.row();
}

void player::on_actionExit_triggered()
{
 exit(0);
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
   //qDebug () << g1_checked << " : " << g2_checked;

    if(g1_checked == true){
        QString selectedGenre1 = ui->cBoxGenre1->currentText();
        addG1 = " genre1='"+selectedGenre1+"' ";
    }
    if(g2_checked == true){
        QString selectedGenre2 = ui->cBoxGenre2->currentText();
        if(g1_checked==true){
             addG2 = "and genre2='"+selectedGenre2+"' ";
        } else{
            addG2 = " genre2='"+selectedGenre2+"' ";
        }

    }

    QSqlQueryModel * model = new QSqlQueryModel();
    model->setQuery("select * from musics where "+addG1+addG2);
    ui->musicView->setModel(model);


}

void player::on_actionAdd_Publicity_triggered()
{
    add_pub add_pub;
    add_pub.setModal(true);
    add_pub.exec();
}
