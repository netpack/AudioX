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
#include <QtGui>
#include <phonon/audiodataoutput.h>

class AudioClass: public QWidget{
Q_OBJECT
    QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> > m_audiodata;

public  slots:
  void dataReceived(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >& data){
  //qDebug()<<"Hii"<<data[Phonon::AudioDataOutput::LeftChannel][512];
      m_audiodata = data;
      update();
 }
  void paintEvent(QPaintEvent *event){
//      qDebug()<<"In Paint Event";
      int w = size().width();
      int h = size().height();
      QPainter painter;
      painter.begin(this);
        painter.setBrush(Qt::white);
        //Draw A Background Rectangle
        painter.drawRect(0,0,w,h);


        //Draw the wave form
        QPainterPath leftChannelPath,rightChannelPath;

        int y = 0;
        int x = 0;
        int nSamples = m_audiodata[Phonon::AudioDataOutput::LeftChannel].size();
        for(int i =0; i < nSamples ;i++){
            //One for the left channel
            y = (m_audiodata[Phonon::AudioDataOutput::LeftChannel][i]*h)/131070 + h*0.25;
            x = (i*w)/nSamples;
            leftChannelPath.lineTo(x,y);
            //One for the right channel
            y= (m_audiodata[Phonon::AudioDataOutput::RightChannel][i]*h)/131070 + h*0.75;
            rightChannelPath.lineTo(x,y);
        }
        //Draw the Audio data
        painter.setBrush(Qt::transparent);
        painter.setPen(Qt::blue);
        painter.drawPath(leftChannelPath);
        painter.setPen(Qt::red);
        painter.drawPath(rightChannelPath);

        painter.setPen(Qt::black);
        //Draw the axes
        painter.drawLine(0,0,0,h);
        painter.drawLine(0,h*0.25,w,h*0.25);
        painter.drawLine(0,h*0.5,w,h*0.5);
        painter.drawLine(0,h*0.75,w,h*0.75);

      painter.end();
  }

};

