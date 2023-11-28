#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


// FORWARD
class Histogram;
class OriginalImageWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class FILTER : int {
        CUSTOM = 0,
        GAUSSIAN,
        LAPLACIAN,
        GENERIC_HIGH_PASS,
        PREWITT_HX,
        PREWITT_HY,
        SOBEL_HX,
        SOBEL_HY
    };

    // ------------------------------------------------ PUBLIC FUNCTIONS ------------------------------------------------

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // GETTERS
    QImage& get_img_original();
    QImage& get_img_modified();

    // SETTERS
    void set_img_original(QImage& img);
    void set_img_modified(QImage& img);

    // convert the image to grayscale using luminance
    void convert_to_grayscale(QImage& img);

    // return true if the image is in grayscale, false otherwise
    bool img_is_grayscale(QImage img);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // ------------------------------------------------ AUXILIAR FUNCTIONS ------------------------------------------------

    // center the image label when window resized
    void center_image_label();

    // update the QPixel map of modified image
    void update_modified_img();

    // return true if the modified img is null and show a pop up informing this, false if it is not null
    bool img_is_null();

    // creates a dialog containing a spinbox
    // label_text is the text label informed, min and max specifies the range of the spinbox
    int create_spinbox_dialog(QString label_text, int min, int max);

    // ------------------------------------------------ MENU FILE OPERATIONS ------------------------------------------------

    // when import button is clicked, pop up an interface to import an image
    // SHORTCUT: Ctrl + I
    void on_actionImport_Image_triggered();

    // when save button is clicked, pop up an interface to save the modified
    // SHORTCUT: Ctrl + S
    void on_actionSave_Image_As_triggered();

    // reset the size of the original image window
    void on_actionReset_Original_Image_Size_triggered();

    // copy the modified image to clipboard
    // SHORTCUT: Ctrl + C
    void on_actionCopy_Image_to_Clipboard_triggered();

    // when reset button is clicked, reset the modified image leaving the same as the original
    // SHORTCUT: Ctrl + X
    void on_actionReset_Image_to_Original_triggered();

    // ------------------------------------------------ MENU TRANSFORMATION OPERATIONS ------------------------------------------------

    // when mirror vertically button is clicked, flip the y axis of the modified image
    void on_actionMirror_Vertically_triggered();

    // when mirror horizontally button is clicked, flip the x axis of the modified image
    void on_actionMirror_Horizontally_triggered();

    // rotate the image 90° clockwise
    void on_actionRotate_90_Clockwise_triggered();

    // rotate the image 90° counterclockwise
    void on_actionRotate_90_Counterclockwise_triggered();

    // zoom in the image
    // SHORTCUT: Ctrl + +
    void on_actionZoom_in_triggered();

    // zoom out the image
    // SHORTCUT: CTRL + -
    void on_actionZoom_out_triggered();

    // ------------------------------------------------ MENU ADJUSTMENT OPERATIONS ---------------------------------------------------

    // show on separate window a histogram of the image, each column represent a tone of gray
    void on_actionShow_Histogram_triggered();

    // when grayscale button is clicked, grayscale the image using luminance
    void on_actionGrayscale_triggered();

    // when negative button is clicked, calculate the negative of the image g(x,y) = 255 - f(x,y)
    void on_actionNegative_triggered();

    // recalculate the brightness of the image using the value b of the spinbox g(x,y) = f(x,y) + b
    void on_actionAdjust_Brightness_triggered();

    // recalculate the contrast of the image using the value a of the spinbox g(x,y) = a * f(x,y)
    void on_actionAdjust_Contrast_triggered();

    // when quantize button is clicked, quantize the image using the number of shades informed on the interface
    void on_actionQuantize_triggered();

    // ------------------------------------------------ MENU FILTER & CONVOLUTION OPERATIONS ------------------------------------------------

    // run conv n times for Gaussian (Low-Pass) kernel
    void on_actionGaussian_lowpass_triggered();

    // run conv for Laplacian (High-Pass) kernel
    void on_actionLaplacian_High_Pass_triggered();

    // run conv for a Generic High-Pass kernel
    void on_actionGeneric_High_Pass_triggered();

    // run conv for Prewitt Hx kernel
    void on_actionPrewitt_Hx_triggered();

    // run conv for Prewitt Hy kernel
    void on_actionPrewitt_Hy_triggered();

    // run conv for Sobel Hx kernel
    void on_actionSobel_Hx_triggered();

    // run conv for Sobel Hy kernel
    void on_actionSobel_Hy_triggered();

    // when custom kernel is clicked, run conv for this custom kernel
    void on_actionCustom_3x3_Kernel_triggered();

    // convolution the image using a kernel
    //filter is used to specify which kernel for specific operations
    void conv(std::vector<std::vector<float>>& kernel, FILTER filter);

    // rotate the kernel 180 degrees
    void rotate_180_kernel(std::vector<std::vector<float>>& kernel);

signals:
    void resized();

private:

    Ui::MainWindow* ui;
    OriginalImageWindow* oiw;
    Histogram* h;

    QImage img_original;
    QImage img_modified;
};
#endif // MAINWINDOW_H
