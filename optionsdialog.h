#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QtSql>
#include <player.h>

namespace Ui {
class optionsDialog;
}

class optionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit optionsDialog(QWidget *parent = 0);
    ~optionsDialog();
    QSqlDatabase adb;


private slots:


    void on_checkBox_disableSeekBar_toggled(bool checked);
    void saveSettings2Db();
    void on_bt_save_settings_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_bt_pwd_clicked();

    void on_bt_uname_clicked();

    void on_bt_edit_settings_clicked();

    void on_bt_free_clicked();

    void on_bt_df_clicked();

    void on_bt_update_youtubedl_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::optionsDialog *ui;
    QString txt_selected_db;
    QString disableSeekBar;
    QString Normalize_Soft;
    QString Disable_Volume;

};

#endif // OPTIONSDIALOG_H
