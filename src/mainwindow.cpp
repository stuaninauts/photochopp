#include "mainwindow.h"
#include "qboxlayout.h"
#include "qpushbutton.h"
#include "qspinbox.h"
#include "ui_mainwindow.h"

#include "histogram.h"
#include "originalimagewindow.h"

#include <QPixmap>
#include <QString>
#include <QImage>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <iostream>
#include <vector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    oiw(nullptr),
    h(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("PhotoChopp");

    oiw = new OriginalImageWindow(this, this);

    connect(this, &MainWindow::resized, this, &MainWindow::center_image_label);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete h;
}

QImage& MainWindow::get_img_original()
{
    return img_original;
}

QImage& MainWindow::get_img_modified()
{
    return img_modified;
}

void MainWindow::set_img_original(QImage& img)
{
    img_original = img;
    return;
}
void MainWindow::set_img_modified(QImage& img)
{
    img_modified = img;
    update_modified_img();
    return;
}

/*
 *
 * ------------------------------------------------ PROTECTED FUNCTIONS ------------------------------------------------
 *
*/

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    emit resized();
}

/*
 *
 * ------------------------------------------------ AUXILIAR FUNCTIONS ------------------------------------------------
 *
*/

void MainWindow::center_image_label() {
    QPoint center = rect().center() - ui->img_label_modified->rect().center();
    ui->img_label_modified->move(center);
}

void MainWindow::update_modified_img() {
    QPixmap pix_modified = QPixmap::fromImage(img_modified);
    int label_width = ui->img_label_modified->width();
    int label_height = ui->img_label_modified->height();

    if (pix_modified.width() > label_width || pix_modified.height() > label_height) {
        pix_modified = pix_modified.scaled(label_width, label_height, Qt::KeepAspectRatio);
    }

    ui->img_label_modified->setPixmap(pix_modified);
    ui->img_label_modified->setAlignment(Qt::AlignCenter);
}

bool MainWindow::img_is_null() {
    if (img_modified.isNull()) {
        QMessageBox::about(this, "Error!", "You have to import an image first.");
        return true;
    } else {
        return false;
    }
}

int MainWindow::create_spinbox_dialog(QString label_text, int min, int max) {
    QDialog dialog(this);

    QSpinBox spinBox(&dialog);
    spinBox.setRange(min, max);

    QLabel text(&dialog);
    text.setText(label_text);

    QPushButton btn_ok("OK", &dialog);
    QPushButton btn_cancel("Cancel", &dialog);

    QVBoxLayout layout(&dialog);
    layout.addWidget(&text);
    layout.addWidget(&spinBox);

    QHBoxLayout layout_btn;
    layout_btn.addWidget(&btn_cancel);
    layout_btn.addWidget(&btn_ok);

    layout.addLayout(&layout_btn);

    dialog.setLayout(&layout);

    connect(&btn_cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(&btn_ok, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        return spinBox.value();
    } else {
        return INT_MIN;
    }
}

bool MainWindow::img_is_grayscale(QImage img) {

    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            QRgb pixel = img.pixel(x, y);
            int red = qRed(pixel);
            int green = qGreen(pixel);
            int blue = qBlue(pixel);
            if (red != green || red != blue || green != blue) {
                return false;
            }
        }
    }

    return true;
}

void MainWindow::convert_to_grayscale(QImage& img)
{
    for (int y = 0; y < img.height(); ++y) {
        uchar* scanLine = img.scanLine(y);
        for (int x = 0; x < img.width(); ++x) {
            // convert the uchar pointer to QRgb
            QRgb pixel = *reinterpret_cast<QRgb*>(scanLine + x * sizeof(QRgb));

            // calculate the luminance and convert the pixel mantaining the alpha of the pixel
            int luminance = qRed(pixel) * 0.299 + qGreen(pixel) * 0.587 + qBlue(pixel) * 0.114;
            QRgb grayscalePixel = qRgba(luminance, luminance, luminance, qAlpha(pixel));

            memcpy(scanLine + x * sizeof(QRgb), &grayscalePixel, sizeof(QRgb));
        }
    }

    return;
}


/*
 *
 * ------------------------------------------------ MENU FILE OPERATIONS ------------------------------------------------
 *
*/

void MainWindow::on_actionImport_Image_triggered()
{
    QString path_import = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(), tr("Images (*.png, *.jpg)"));

    if (!path_import.isEmpty()) {


        // load the original image window
        img_original = QImage(path_import);
        if (oiw) {
            oiw->load_img_original();
            oiw->show();
        }


        img_modified = QImage(img_original);




        update_modified_img();
    }

    return;
}

void MainWindow::on_actionSave_Image_As_triggered()
{
    if (!img_is_null()) {
        QString path_save = QFileDialog::getSaveFileName(this, tr("Salvar Imagem"), QDir::homePath(), tr("Imagens (*.png *.jpg)"));

        if (!path_save.isEmpty()) {
            if (img_modified.save(path_save)) {
                qDebug() << "Success!";
            } else {
                qDebug() << "Error to save image!";
            }
        }
    }

    return;
}

void MainWindow::on_actionReset_Original_Image_Size_triggered()
{
    int w = img_original.width();
    int h = img_original.height();

    oiw->resize(w, h);
    oiw->load_img_original();

    return;
}

void MainWindow::on_actionCopy_Image_to_Clipboard_triggered()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setImage(img_modified);
    return;
}


void MainWindow::on_actionReset_Image_to_Original_triggered()
{
    if (!img_is_null()) {
        img_modified = QImage(img_original);
        update_modified_img();
        return;
    }
}

/*
 *
 * ------------------------------------------------ MENU TRANSFORMATION OPERATIONS ------------------------------------------------
 *
*/

void MainWindow::on_actionMirror_Vertically_triggered()
{
    if (!img_is_null()) {
        int w = img_modified.width();
        int h = img_modified.height();
        int bytesPerLine = img_modified.bytesPerLine();

        uchar* dados = img_modified.bits();

        int line_size = w * 4; // 4 bytes por pixel (ARGB32)
        uchar* aux_line = new uchar[line_size];
        uchar* line1, *line2;

        int line;
        // iterates over the lines swapping lines using aux_line
        for (line = 0; line <= h / 2; line++) {
            line1 = dados + line * bytesPerLine;
            line2 = dados + (h - line - 1) * bytesPerLine;

            memcpy(aux_line, line1, line_size);
            memcpy(line1, line2, line_size);
            memcpy(line2, aux_line, line_size);
        }

        delete[] aux_line;

        update_modified_img();
    }

    return;
}


void MainWindow::on_actionMirror_Horizontally_triggered()
{
    if (!img_is_null()) {
        int h = img_modified.height();
        int bytesPerLine = img_modified.bytesPerLine();

        uchar* data = img_modified.bits();
        int channels = 4; // RGBA

        uint8_t aux_pxl[channels];
        uchar* pxl1, *pxl2;

        int line_begin;
        int pxl;
        int swap_pxl;

        // iterate the image line by line
        for (line_begin = 0; line_begin < h * bytesPerLine; line_begin += bytesPerLine) {
            // pxl start with the begin of the line and increment until the half of the line
            // swap_pxl start with the begin of the next line and decrement until the half of the line
            for (pxl = line_begin, swap_pxl = line_begin + bytesPerLine;
                 pxl <= line_begin + (bytesPerLine / 2);
                 pxl += channels, swap_pxl -= channels) {
                pxl1 = data + pxl;
                pxl2 = data + swap_pxl - channels;

                memcpy(aux_pxl, pxl1, channels);
                memcpy(pxl1, pxl2, channels);
                memcpy(pxl2, aux_pxl, channels);
            }
        }

        update_modified_img();
    }

    return;
}

void MainWindow::on_actionRotate_90_Clockwise_triggered()
{
    if (!img_is_null()) {
        QImage rotated_img(img_modified.height(), img_modified.width(), img_modified.format());

        for (int y = 0; y < img_modified.height(); ++y) {
            for (int x = 0; x < img_modified.width(); ++x) {
                QRgb pixel = img_modified.pixel(x, y);
                rotated_img.setPixel(img_modified.height() - y - 1, x, pixel);
            }
        }

        img_modified = rotated_img;
        update_modified_img();
    }

    return;
}


void MainWindow::on_actionRotate_90_Counterclockwise_triggered()
{
    if (!img_is_null()) {
        QImage rotated_img(img_modified.height(), img_modified.width(), img_modified.format());

        for (int y = 0; y < img_modified.height(); ++y) {
            for (int x = 0; x < img_modified.width(); ++x) {
                QRgb pixel = img_modified.pixel(x, y);
                rotated_img.setPixel(y, img_modified.width() - x - 1, pixel);
            }
        }

        img_modified = rotated_img;
        update_modified_img();
    }

    return;
}

void MainWindow::on_actionZoom_in_triggered()
{
    int new_h = img_modified.height() * 2 - 1;
    int new_w = img_modified.width() * 2 - 1;
    QImage aux_img = QImage(new_w, new_h, img_modified.format());

    // fill the previous values
    for (int y = 0; y < img_modified.height(); ++y) {
        for (int x = 0; x < img_modified.width(); ++x) {
            const QRgb pixel = img_modified.pixel(x, y);
            aux_img.setPixel(x * 2, y * 2, pixel);
        }
    }

    // fill the new cols created
    for (int y = 0; y < new_h; y += 2) {
        for (int x = 1; x < new_w - 1; x += 2) {
            const QRgb left_pixel = aux_img.pixel(x-1, y);
            const QRgb right_pixel = aux_img.pixel(x+1, y);
            const QRgb interpolated_pixel = qRgb((qRed(left_pixel) + qRed(right_pixel)) / 2,
                                                (qGreen(left_pixel) + qGreen(right_pixel)) / 2,
                                                (qBlue(left_pixel) + qBlue(right_pixel)) / 2);
            aux_img.setPixel(x, y, interpolated_pixel);
        }
    }


    // fill the new lines created
    for (int x = 0; x < new_w; x++) {
        for (int y = 1; y < new_h - 1; y += 2) {
            const QRgb top_pixel = aux_img.pixel(x, y-1);
            const QRgb bottom_pixel = aux_img.pixel(x, y+1);
            const QRgb interpolated_pixel = qRgb((qRed(top_pixel) + qRed(bottom_pixel)) / 2,
                                                 (qGreen(top_pixel) + qGreen(bottom_pixel)) / 2,
                                                 (qBlue(top_pixel) + qBlue(bottom_pixel)) / 2);
            aux_img.setPixel(x, y, interpolated_pixel);
        }
    }

    img_modified = aux_img;
    update_modified_img();

    return;
}

void MainWindow::on_actionZoom_out_triggered() {
    int sx = create_spinbox_dialog("Sx:", 1, img_modified.width());
    int sy = create_spinbox_dialog("Sy:", 1, img_modified.height());

    QImage new_img(img_modified.width() / sx, img_modified.height() / sy, img_modified.format());

    for (int y = 0; y < new_img.height(); y++) {
        for (int x = 0; x < new_img.width(); x++) {
            int total_pixels = 0;
            int total_red = 0, total_green = 0, total_blue = 0;

            for (int j = 0; j < sy; j++) {
                for (int i = 0; i < sx; i++) {
                    int img_x = x * sx + i;
                    int img_y = y * sy + j;

                    if (img_x < img_modified.width() && img_y < img_modified.height()) {
                            QRgb pixel = img_modified.pixel(img_x, img_y);
                            total_red += qRed(pixel);
                            total_green += qGreen(pixel);
                            total_blue += qBlue(pixel);
                            total_pixels++;
                    }
                }
            }

            int avg_red = total_red / total_pixels;
            int avg_green = total_green / total_pixels;
            int avg_blue = total_blue / total_pixels;

            new_img.setPixel(x, y, qRgb(avg_red, avg_green, avg_blue));
        }
    }

    img_modified = new_img;
    update_modified_img();

    return;
}

/*
 *
 * ------------------------------------------------ MENU ADJUSTMENT OPERATIONS ---------------------------------------------------
 *
*/

void MainWindow::on_actionShow_Histogram_triggered()
{
    h = new Histogram(this);
    h->set_mw_ptr(this);
    h->show();
}


void MainWindow::on_actionGrayscale_triggered()
{
    if (!img_is_null()) {
        convert_to_grayscale(img_modified);
        update_modified_img();
    }
}


void MainWindow::on_actionNegative_triggered()
{
    if (!img_is_null()) {
        for (int y = 0; y < img_modified.height(); ++y) {
            QRgb* scanLine = reinterpret_cast<QRgb*>(img_modified.scanLine(y));
            for (int x = 0; x < img_modified.width(); ++x) {
                int red = 255 - qRed(scanLine[x]);
                int green = 255 - qGreen(scanLine[x]);
                int blue = 255 - qBlue(scanLine[x]);

                red = qBound(0, red, 255);
                green = qBound(0, green, 255);
                blue = qBound(0, blue, 255);

                scanLine[x] = qRgb(red, green, blue);
            }
        }
        update_modified_img();
    }
    return;
}

void MainWindow::on_actionAdjust_Brightness_triggered()
{
    int result = create_spinbox_dialog("Adjust Brightness", -255, 255);

    if (result != INT_MIN) {
        if (!img_is_null()) {
            int b = result;
            for (int y = 0; y < img_modified.height(); ++y) {
                QRgb* scanLine = reinterpret_cast<QRgb*>(img_modified.scanLine(y));
                for (int x = 0; x < img_modified.width(); ++x) {
                    int red = qRed(scanLine[x]) + b;
                    int green = qGreen(scanLine[x]) + b;
                    int blue = qBlue(scanLine[x]) + b;

                    red = qBound(0, red, 255);
                    green = qBound(0, green, 255);
                    blue = qBound(0, blue, 255);

                    scanLine[x] = qRgb(red, green, blue);
                }
            }
            update_modified_img();
        }
    }

    return;
}


void MainWindow::on_actionAdjust_Contrast_triggered()
{
    int result = create_spinbox_dialog("Adjust Contrast", 0, 255);

    if (result != INT_MIN) {
        if (!img_is_null()) {
            int a = result;
            for (int y = 0; y < img_modified.height(); ++y) {
                QRgb* scanLine = reinterpret_cast<QRgb*>(img_modified.scanLine(y));
                for (int x = 0; x < img_modified.width(); ++x) {
                    int red = qRed(scanLine[x]) * a;
                    int green = qGreen(scanLine[x]) * a;
                    int blue = qBlue(scanLine[x]) * a;

                    red = qBound(0, red, 255);
                    green = qBound(0, green, 255);
                    blue = qBound(0, blue, 255);

                    scanLine[x] = qRgb(red, green, blue);
                }
            }
            update_modified_img();
        }
    }

    return;
}


void MainWindow::on_actionQuantize_triggered()
{
    int result = create_spinbox_dialog("Set number of shades:", 1, 255);

    if (result != INT_MIN) {
        if (!img_is_null()) {
            int n = result; // number of shades
            if (n > 0) {
                int w = img_modified.width();
                int h = img_modified.height();
                int bytesPerLine = img_modified.bytesPerLine();
                uchar* data = img_modified.bits();

                int t1 = 255; // initialize with high value
                int t2 = 0;   // initialize with low value

                // get the min and max intensity values
                for (int y = 0; y < h; y++) {
                    QRgb* linha = reinterpret_cast<QRgb*>(data + y * bytesPerLine);
                    for (int x = 0; x < w; x++) {
                        int intensity = qRed(linha[x]); // for grayscale images
                        t1 = qMin(t1, intensity);
                        t2 = qMax(t2, intensity);
                    }
                }

                int tam_int = t2 - t1 + 1;

                if (n >= tam_int) {
                    return; // it is not necessary quantize
                } else {
                    int tb = tam_int / n;
                    for (int y = 0; y < h; y++) {
                        QRgb* line = reinterpret_cast<QRgb*>(data + y * bytesPerLine);
                        for (int x = 0; x < w; x++) {
                            int intensity = qRed(line[x]); // for grayscale images
                            int bin = (intensity - t1) / tb;
                            int new_t = t1 + bin * tb + tb / 2; // get the center value
                            new_t = qBound(0, new_t, 255); // guarantees the value is in the range [0, 255]
                            line[x] = qRgb(new_t, new_t, new_t);
                        }
                    }
                }

                update_modified_img();
            }
        }
    }

    return;
}

/*
 *
 * ------------------------------------------------ MENU FILTER & CONVOLUTION OPERATIONS ------------------------------------------------
 *
*/

void MainWindow::on_actionGaussian_lowpass_triggered()
{
    std::vector<std::vector<float>> kernel = {{0.0625, 0.125, 0.0625},
                                              {0.125,  0.25,  0.125},
                                              {0.0625, 0.125, 0.0625}};

    int times = create_spinbox_dialog("Apply Gaussian Filter how many times?", 1, 1000);
    for (int i = 0; i < times; i++)
        conv(kernel, FILTER::GAUSSIAN);
}

void MainWindow::on_actionLaplacian_High_Pass_triggered()
{
    std::vector<std::vector<float>> kernel = {{ 0, -1,  0},
                                              {-1,  4, -1},
                                              { 0, -1 , 0}};
    conv(kernel, FILTER::LAPLACIAN);
}


void MainWindow::on_actionGeneric_High_Pass_triggered()
{
    std::vector<std::vector<float>> kernel = {{-1, -1, -1},
                                              {-1,  8, -1},
                                              {-1, -1, -1}};
    conv(kernel, FILTER::GENERIC_HIGH_PASS);
}


void MainWindow::on_actionPrewitt_Hx_triggered()
{
    std::vector<std::vector<float>> kernel = {{-1, 0, 1},
                                              {-1, 0, 1},
                                              {-1, 0, 1}};
    conv(kernel, FILTER::PREWITT_HX);
}


void MainWindow::on_actionPrewitt_Hy_triggered()
{
    std::vector<std::vector<float>> kernel = {{-1, -1, -1},
                                              { 0,  0,  0},
                                              { 1,  1,  1}};
    conv(kernel, FILTER::PREWITT_HY);
}


void MainWindow::on_actionSobel_Hx_triggered()
{
    std::vector<std::vector<float>> kernel = {{-1, 0, 1},
                                              {-2, 0, 2},
                                              {-1, 0, 1}};
    conv(kernel, FILTER::SOBEL_HX);
}


void MainWindow::on_actionSobel_Hy_triggered()
{
    std::vector<std::vector<float>> kernel = {{-1, -2, -1},
                                              { 0,  0,  0},
                                              { 1,  2,  1}};
    conv(kernel, FILTER::SOBEL_HY);
}

void MainWindow::on_actionCustom_3x3_Kernel_triggered()
{
    QDialog dialog(this);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QGridLayout* gridLayout = new QGridLayout;
    std::vector<std::vector<QDoubleSpinBox*>> spinBoxes;

    // create 3x3 matrix with double spinbox
    for (int i = 0; i < 3; ++i) {
        std::vector<QDoubleSpinBox*> row;
        for (int j = 0; j < 3; ++j) {
            QDoubleSpinBox* spinBox = new QDoubleSpinBox;
            spinBox->setRange(-1000.0, 1000.0); // Define um intervalo de valores permitidos
            gridLayout->addWidget(spinBox, i, j);
            row.push_back(spinBox);
        }
        spinBoxes.push_back(row);
    }

    layout->addLayout(gridLayout);

    QPushButton* btnGetValues = new QPushButton("Run Kernel");
    layout->addWidget(btnGetValues);

    // store the values in the kernel and calls conv
    std::vector<std::vector<float>> kernel(3, std::vector<float>(3, 0.0));
    connect(btnGetValues, &QPushButton::clicked, [&spinBoxes, &kernel, this, &dialog] () {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                kernel[i][j] = spinBoxes[i][j]->value();
            }
        }
        conv(kernel, FILTER::CUSTOM);
        dialog.close();
    });

    dialog.setLayout(layout);
    dialog.exec();
}

void MainWindow::conv(std::vector<std::vector<float>>& kernel, FILTER filter)
{
    if (!img_is_null()) {
        rotate_180_kernel(kernel); // rotate 180 degrees the kernel
        QImage aux_img = img_modified; // aux img for calculations

        // only performs convolution for
        // gaussian kernel or
        // grayscale images
        if (filter == FILTER::GAUSSIAN || img_is_grayscale(img_modified)) {
            for (int x = 1; x < img_modified.width() - 1; x++) {
                for (int y = 1; y < img_modified.height() - 1; y++) {
                    int red_conv = 0, green_conv = 0, blue_conv = 0;

                    // apply the kernel for each channel

                    red_conv = kernel[0][0] * qRed(aux_img.pixel(x-1, y-1)) +
                               kernel[1][0] * qRed(aux_img.pixel(x, y-1)) +
                               kernel[2][0] * qRed(aux_img.pixel(x+1, y-1)) +
                               kernel[0][1] * qRed(aux_img.pixel(x-1, y)) +
                               kernel[1][1] * qRed(aux_img.pixel(x, y)) +
                               kernel[2][1] * qRed(aux_img.pixel(x+1, y)) +
                               kernel[0][2] * qRed(aux_img.pixel(x-1,y+1)) +
                               kernel[1][2] * qRed(aux_img.pixel(x,y+1)) +
                               kernel[2][2] * qRed(aux_img.pixel(x+1,y+1));

                    green_conv = kernel[0][0] * qBlue(aux_img.pixel(x-1, y-1)) +
                                 kernel[1][0] * qBlue(aux_img.pixel(x, y-1)) +
                                 kernel[2][0] * qBlue(aux_img.pixel(x+1, y-1)) +
                                 kernel[0][1] * qBlue(aux_img.pixel(x-1, y)) +
                                 kernel[1][1] * qBlue(aux_img.pixel(x, y)) +
                                 kernel[2][1] * qBlue(aux_img.pixel(x+1, y)) +
                                 kernel[0][2] * qBlue(aux_img.pixel(x-1,y+1)) +
                                 kernel[1][2] * qBlue(aux_img.pixel(x,y+1)) +
                                 kernel[2][2] * qBlue(aux_img.pixel(x+1,y+1));

                    blue_conv = kernel[0][0] * qGreen(aux_img.pixel(x-1, y-1)) +
                                kernel[1][0] * qGreen(aux_img.pixel(x, y-1)) +
                                kernel[2][0] * qGreen(aux_img.pixel(x+1, y-1)) +
                                kernel[0][1] * qGreen(aux_img.pixel(x-1, y)) +
                                kernel[1][1] * qGreen(aux_img.pixel(x, y)) +
                                kernel[2][1] * qGreen(aux_img.pixel(x+1, y)) +
                                kernel[0][2] * qGreen(aux_img.pixel(x-1,y+1)) +
                                kernel[1][2] * qGreen(aux_img.pixel(x,y+1)) +
                                kernel[2][2] * qGreen(aux_img.pixel(x+1,y+1));


                    // apply rules for these kernels
                    switch (filter) {
                    case FILTER::PREWITT_HX:
                    case FILTER::PREWITT_HY:
                    case FILTER::SOBEL_HX:
                    case FILTER::SOBEL_HY:
                        red_conv += 127;
                        green_conv += 127;
                        blue_conv += 127;
                        break;
                    default:
                        break;
                    }

                    // garantees the that the values are in the range
                    red_conv = qBound(0, red_conv, 255);
                    green_conv = qBound(0, green_conv, 255);
                    blue_conv = qBound(0, blue_conv, 255);

                    img_modified.setPixel(x, y, qRgb(red_conv, green_conv, blue_conv));
                }
            }

            update_modified_img();

        }  else {
            QMessageBox::about(this, "Error!", "You can't apply this filter for images which are not in gray scale");
        }
    }

    return;
}

void MainWindow::rotate_180_kernel(std::vector<std::vector<float>>& kernel)
{
    int n = kernel.size();

    // invert lines
    for (int i = 0; i < n / 2; i++) {
        for (int j = 0; j < n; j++) {
            std::swap(kernel[i][j], kernel[n - i - 1][j]);
        }
    }

    // invert cols
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n / 2; j++) {
            std::swap(kernel[i][j], kernel[i][n - j - 1]);
        }
    }

    // print the kernel (informational reasons)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j<n; j++) {
            std::cout << kernel[i][j] << " ";
        }
        std::cout << std::endl;
    }

    return;
}


