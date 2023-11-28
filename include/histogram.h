#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QDialog>

class MainWindow;

namespace Ui {
class Histogram;
}

class Histogram : public QDialog
{
    Q_OBJECT

public:
    explicit Histogram(QWidget* parent = nullptr);
    ~Histogram();

    void set_mw_ptr(MainWindow *ptr);

    // update hist attribute with this img
    // update the cum_norm_hist attribute with this img
    void load_first_plot();

    // given a histogram, update the plot with the values of the histogram
    void set_plot(const std::vector<int>& hist, int axis);

    // calculate the given histogram for the given image
    void histogram_calculate(const QImage& img, std::vector<int>& histogram);

    // calculate the accumulated normalized histogram given a initial histogram
    void histogram_accumulate_normalize(const std::vector<int>& histogram, std::vector<int>& cum_norm_histogram);

    // plot the equalized histogram
    void plot_equalized(const std::vector<int>& equal_histogram);

    void init_hists(const QImage& img, std::vector<int>& histogram, std::vector<int>& cum_norm_histogram);

    void clear_hists(std::vector<int>& histogram, std::vector<int>& cum_norm_histogram);

    int find_target_shade_level_closest_to(int shade_level, const std::vector<int>& src_cum_hist, const std::vector<int>& target_cum_hist);


private slots:
    // if checked show cum_norm_hist as the plot
    // otherwise, show the normal hist plot
    void on_checkbox_show_cum_hist_toggled(bool checked);

    // equalize the image and update modified image
    void on_btn_hist_equalize_clicked();

    // match the histogram of modified image with imported image
    void on_btn_hist_match_clicked();


private:
    Ui::Histogram *ui;
    MainWindow* mw_ptr;

    std::vector<int> hist;
    std::vector<int> cum_norm_hist;

    bool equal_clicked = false;
    std::vector<int> equal_hist;
    std::vector<int> equal_cum_norm_hist;

};

#endif // HISTOGRAM_H
