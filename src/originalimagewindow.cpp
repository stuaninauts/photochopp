#include "originalimagewindow.h"
#include "ui_originalimagewindow.h"
#include "mainwindow.h"
#include <QMoveEvent>

OriginalImageWindow::OriginalImageWindow(QWidget *parent, MainWindow* mainWindow) :
    QDialog(parent),
    ui(new Ui::OriginalImageWindow),
    mw_ptr(mainWindow)
{
    ui->setupUi(this);
}

OriginalImageWindow::~OriginalImageWindow()
{
    delete ui;
}

void OriginalImageWindow::load_img_original()
{
    // fix get img original
    pix_original = QPixmap::fromImage(mw_ptr->get_img_original());
    int w = mw_ptr->get_img_original().width();
    int h = mw_ptr->get_img_original().height();

    // resize window
    this->resize(w, h);

    // set the w and h with the original image
    resize_label_and_pixmap();

    return;
}

void OriginalImageWindow::resize_label_and_pixmap()
{
    int w = this->width();
    int h = this->height();

    ui->label_img_original->setFixedWidth(w);
    ui->label_img_original->setFixedHeight(h);

    ui->label_img_original->setPixmap(pix_original.scaled(w,h));
    ui->label_img_original->setScaledContents(true);

    return;
}

void OriginalImageWindow::resizeEvent(QResizeEvent *event) {
    QDialog::resizeEvent(event);
    resize_label_and_pixmap();
}



