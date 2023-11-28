#include "histogram.h"
#include "ui_histogram.h"
#include "mainwindow.h"
#include <iostream>
#include <vector>

Histogram::Histogram(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Histogram),
    mw_ptr(nullptr),
    hist(256, 0),
    cum_norm_hist(256, 0),
    equal_hist(256, 0),
    equal_cum_norm_hist(256, 0)
{
    ui->setupUi(this);
    ui->hist_plot->addGraph(0)->setScatterStyle(QCPScatterStyle::ssDot);
    ui->hist_plot->addGraph(0)->setLineStyle(QCPGraph::lsLine);
    ui->hist_plot->xAxis->setLabel("Gray Tone");
    ui->hist_plot->yAxis->setLabel("Frequency");
    ui->hist_plot->xAxis->setRange(0, 255);
    ui->hist_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    set_plot(hist, 0);
}

Histogram::~Histogram()
{
    delete ui;
}

void Histogram::set_mw_ptr(MainWindow *ptr)
{
    mw_ptr = ptr;
    load_first_plot();
    return;
}

void Histogram::load_first_plot()
{
    init_hists(mw_ptr->get_img_modified(), hist, cum_norm_hist);
    set_plot(hist, 0);
    return;
}

void Histogram::init_hists(const QImage& img, std::vector<int>& histogram, std::vector<int>& cum_norm_histogram)
{
    histogram_calculate(img, histogram);
    histogram_accumulate_normalize(histogram, cum_norm_histogram);
    return;
}

void Histogram::clear_hists(std::vector<int>& histogram, std::vector<int>& cum_norm_histogram)
{
    histogram.clear();
    cum_norm_histogram.clear();
    return;
}

void Histogram::set_plot(const std::vector<int>& hist, int axis)
{
    // set data
    QVector<double> xData, yData;
    for (size_t i = 0; i < hist.size(); ++i) {
        xData.push_back(i);
        yData.push_back(hist[i]);
    }

    // set color
    QColor lineColor = (axis == 0) ? Qt::blue : Qt::red;
    QPen pen(lineColor);
    ui->hist_plot->graph(axis)->setPen(pen);

    // set params
    int max_y_value = *std::max_element(hist.begin(), hist.end());
    ui->hist_plot->yAxis->setRange(0, max_y_value);
    ui->hist_plot->graph(axis)->setData(xData, yData);

    // update plot
    ui->hist_plot->replot();

    return;
}


void Histogram::histogram_calculate(const QImage& img, std::vector<int>& histogram)
{
    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            QRgb pixel = img.pixel(x, y);
            int gray_pixel = qGray(pixel);
            ++histogram[gray_pixel];
        }
    }

    return;
}

void Histogram::histogram_accumulate_normalize(const std::vector<int>& histogram, std::vector<int>& cum_norm_histogram)
{
    int n_pixels = std::accumulate(histogram.begin(), histogram.end(), 0);
    float alpha = 255.0 / n_pixels;

    cum_norm_histogram[0] = alpha * histogram[0];
    for (size_t i = 1; i < histogram.size(); i++) {
        cum_norm_histogram[i] = cum_norm_histogram[i - 1] + alpha * histogram[i];
    }

    return;
}

void Histogram::on_checkbox_show_cum_hist_toggled(bool checked)
{
    if (checked) {
        set_plot(cum_norm_hist, 0);
        if (equal_clicked) {
            set_plot(equal_cum_norm_hist, 1);
        }
    } else {
        set_plot(hist, 0);
        if (equal_clicked) {
           set_plot(equal_hist, 1);
        }
    }

    return;
}

void Histogram::on_btn_hist_equalize_clicked()
{
    equal_clicked = true;

    QImage new_img = mw_ptr->get_img_modified();

    for (int x = 0; x < new_img.width(); x++) {
        for (int y = 0; y < new_img.height(); y++) {
            QRgb pixel = mw_ptr->get_img_modified().pixel(x, y);
            int gray_value = cum_norm_hist[qGray(pixel)];
            new_img.setPixel(x, y, qRgb(gray_value, gray_value, gray_value));
        }
    }

    init_hists(new_img, equal_hist, equal_cum_norm_hist);
    set_plot(equal_hist, 1);

    // add option button to alter img
    mw_ptr->set_img_modified(new_img);


    return;
}


void Histogram::on_btn_hist_match_clicked()
{

    QString path_import = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(), tr("Images (*.png, *jpg)"));

    if (!path_import.isEmpty()) {
        QImage target_image = QImage(path_import);
        std::vector<int> target_hist(256, 0);
        std::vector<int> target_cum_norm_hist(256, 0);
        std::vector<int> hm(256, 0);

        if (!mw_ptr->img_is_grayscale(target_image)) {
            mw_ptr->convert_to_grayscale(target_image);
        }

        init_hists(target_image, target_hist, target_cum_norm_hist);

        for (int shade_level = 0; shade_level < 256; shade_level++) {
            hm[shade_level] = find_target_shade_level_closest_to(shade_level, cum_norm_hist, target_cum_norm_hist);
        }

        QImage new_img = mw_ptr->get_img_modified();
        for (int x = 0; x < new_img.width(); x++) {
            for (int y = 0; y < new_img.height(); y++) {
                QRgb pixel = mw_ptr->get_img_modified().pixel(x, y);
                int gray_value = hm[qGray(pixel)];
                new_img.setPixel(x, y, qRgb(gray_value, gray_value, gray_value));
            }
        }

        mw_ptr->set_img_modified(new_img);
    }

    return;
}

int Histogram::find_target_shade_level_closest_to(int shade_level, const std::vector<int>& src_cum_hist, const std::vector<int>& target_cum_hist) {
    int closest_shade = 0;
    int min_diff = INT_MAX;

    for (int i = 0; i < 256; ++i) {
        int diff = std::abs(src_cum_hist[shade_level] - target_cum_hist[i]);
        if (diff < min_diff) {
            min_diff = diff;
            closest_shade = i;
        }
    }

    return closest_shade;
}

