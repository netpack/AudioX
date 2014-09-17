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
#ifndef MAINWINDOWPLAYER_H
#define MAINWINDOWPLAYER_H

#include <QMainWindow>

namespace Ui {
class MainWindowPlayer;
}

class MainWindowPlayer : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindowPlayer(QWidget *parent = 0);
    ~MainWindowPlayer();
    
private:
    Ui::MainWindowPlayer *ui;
};

#endif // MAINWINDOWPLAYER_H
