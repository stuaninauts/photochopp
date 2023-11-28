#ifndef ORIGINALIMAGEWINDOW_H
#define ORIGINALIMAGEWINDOW_H

#include <QDialog>

class MainWindow;

namespace Ui {
class OriginalImageWindow;
}

class OriginalImageWindow : public QDialog
{
    Q_OBJECT

public:
    explicit OriginalImageWindow(QWidget *parent = nullptr, MainWindow* mainWindow = nullptr);
    ~OriginalImageWindow();

    // load the original image into the window (first load)
    void load_img_original();

    // resize label and pixmap with the w and h of the window
    void resize_label_and_pixmap();
private:
    // windows pointers
    Ui::OriginalImageWindow *ui;
    MainWindow* mw_ptr;

    // pixmap of the image
    QPixmap pix_original;

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // ORIGINALIMAGEWINDOW_H
