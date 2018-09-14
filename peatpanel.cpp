#include "peatpanel.h"
#include "ui_peatpanel.h"

PEATPanel::PEATPanel(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PEATPanel)
{
    ui->setupUi(this);
    this->setWindowTitle("UW Trace Center Photosensitive Epilepsy Analysis Tool (PEAT)");
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    this->resize(QSize(900,580));

    player = new QMediaPlayer;
    playlist = new QMediaPlaylist;
    
}

PEATPanel::~PEATPanel()
{
    delete ui;
}

void mainFrame::on_folderButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open a Video File", "", "Video File (*.*)");
    if(!filename.isEmpty())
    {
        playlist->addMedia(QUrl::fromLocalFile(filename));
        playlist->setCurrentIndex(1);

        player->setPlaylist(playlist);
        player->setVideoOutput(ui->videoWidget);
        ui->videoWidget->show();
        player->play();
        ui->rewindButton->setStyleSheet("#rewindButton { background-image: url(:/images/Res/FastBackUp.bmp); border-image: url(:/images/Res/FastBackUp.bmp); } #rewindButton:hover { border-image: url(:/images/Res/FastBackDownHilite.bmp); } #rewindButton:pressed { border-image: url(:/images/Res/FastBackPressed.bmp); }");
        ui->playButton->setStyleSheet("#playButton { background-image: url(:/images/Res/PlayUp.bmp); border-image: url(:/images/Res/PlayUp.bmp); } #playButton:hover { border-image: url(:/images/Res/PlayDownHilite.bmp); } #playButton:pressed { border-image: url(:/images/Res/PlayPressed.bmp); }");
        ui->pauseButton->setStyleSheet("#pauseButton { background-image: url(:/images/Res/StopUp.bmp); border-image: url(:/images/Res/StopUp.bmp); } #pauseButton:hover { border-image: url(:/images/Res/StopDownHilite.bmp); } #pauseButton:pressed { border-image: url(:/images/Res/StopPressed.bmp); }");
        ui->forwardButton->setStyleSheet("#forwardButton { background-image: url(:/images/Res/FFwdUp.bmp); border-image: url(:/images/Res/FFwdUp.bmp); } #forwardButton:hover { border-image: url(:/images/Res/FFwdDownHilite.bmp); } #forwardButton:pressed { border-image: url(:/images/Res/FFwdPressed.bmp); }");

    }
    else
    {
        QMessageBox messageBox;
        messageBox.warning(this, "Error", "Uploading file went wrong !");
        messageBox.setFixedSize(500,200);
    }
}

void mainFrame::on_playButton_clicked()
{
    player->play();
}

void mainFrame::on_pauseButton_clicked()
{
    player->pause();
}

void mainFrame::on_forwardButton_clicked()
{

    if (x < 32.0)
    {
        x = 2.0*x;
        rate = x;
        player->setPlaybackRate(rate);
    }
}

void mainFrame::on_rewindButton_clicked()
{
    if (x > 0.03125)
    {
        x = 0.5*x;
        player->setPlaybackRate(x);
    }
}

