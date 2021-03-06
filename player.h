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
#ifndef PLAYER_H
#define PLAYER_H
#include <QMainWindow>
#include <QtSql>
#include <QtDebug>
#include <QList>
#include <QFileDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <phonon>
#include <phonon/audiodataoutput.h>





namespace Ui {
class player;

}

class player : public QMainWindow
{
    Q_OBJECT

public:
    explicit player(QWidget *parent = 0);
    ~player();


    qint32 transitionTime() const;
    void setTransitionTime(qint32 msec);
    void update_music_table();
    void checkDbOpen();


private slots:
    void musicViewContextMenu(const QPoint&);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void addFiles();
    void nextFile();
    void finished();
    void playPause();
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void tick(qint64 time);
    void sourceChanged(const Phonon::MediaSource &source);
    void aboutToFinish();
    void on_pushButtonPlay_clicked();
    void stopAction();
    void on_actionConfigurations_triggered();
    void on_pushButton_4_clicked();
    void showTime();
    void on_actionAdd_Music_triggered();
    void dropEvent(QDropEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void on_musicView_pressed(const QModelIndex &index);
    void on_actionBuy_me_a_Coffe_triggered();
    void on_actionManage_Genres_triggered();
    void on_actionAdd_Jingle_triggered();
    void dataReceived(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >);
    void on_list_playlist_clicked(const QModelIndex &index);
    void on_actionExit_triggered();
    void on_pushButton_5_clicked();
    void on_jinglesView_pressed(const QModelIndex &index);
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_actionAdd_Publicity_triggered();
    void run_scheduler();
    void on_actionSet_active_database_triggered();
    void on_bt_video_download_clicked();
    void on_stopAction_clicked();
    void on_bt_autoMode_clicked();
    void autoModeGetMoreSongs();
    void actionAdd_a_directory();
    void on_addFiles_clicked();
    void on_actionAdd_a_directory_triggered();
    void list_playlistContextMenu(const QPoint&);
    void on_actionSave_Playlist_triggered();
    void on_actionClear_Playlist_triggered();
    void on_actionLoad_Playlist_triggered();

private:
    QSqlDatabase adb;
    Ui::player *ui;
    Phonon::MediaObject *music;
    QString *file;
    Phonon::SeekSlider *seekSlider;
    Phonon::MediaObject *mediaObject;
    Phonon::MediaObject *metaInformationResolver;
    Phonon::AudioOutput *audioOutput;
    Phonon::AudioDataOutput *dataout;
    Phonon::VolumeSlider *volumeSlider;
    QList<Phonon::MediaSource> sources;
    QMimeData *mimeData;
    QString text;
    QString path;
    QString estevalor;
    QString xaction;
    QMap<int, QWidget *> map;
    QString lastupdated;
    int playing;
    int indexSourceChanged;
    int indexcanal;
    int indexJust3rdDropEvt;
    QString txt_selected_db;
    QString disableSeekBar;
    QString normalization_soft;
    QString Disable_Volume;
    QString ask_normalize_new_files;
    int autoMode;
};

#endif // PLAYER_H
