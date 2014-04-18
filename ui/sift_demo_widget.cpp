#include <QFileDialog>
#include <vector>
#include <QImage>
#include "sift_demo_widget.h"
#include "utils/img_fmt_cvt.h"

#include <QDebug>

result_label::result_label(QWidget *parent) :
QLabel(parent)
{
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

QSize result_label::sizeHint() const
{
    return QSize(480, 640);
}

sift_demo_widget::sift_demo_widget(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *hl;
    QVBoxLayout *vl;

    //mei yong dao...todo.
    //this->lb_pic_1 = new QLabel;
    //this->lb_pic_2 = new QLabel;
    //

    this->gb_type = new QGroupBox(tr("type"), this);
    this->rb_detect = new QRadioButton(tr("detect keypoints only"), this->gb_type);
    this->rb_detect_and_match = new QRadioButton(tr("detect + match"), this->gb_type);
    this->dsb_good_rate = new QDoubleSpinBox(this->gb_type);

    vl = new QVBoxLayout;
    hl = new QHBoxLayout;
    hl->addWidget(new QLabel(tr("good rate:"), this->gb_type));
    hl->addWidget(this->dsb_good_rate);
    vl->addWidget(this->rb_detect);
    vl->addWidget(this->rb_detect_and_match);
    vl->addLayout(hl);
    this->gb_type->setLayout(vl);

    this->gb_method = new QGroupBox(tr("method"), this);
    this->rb_opencv = new QRadioButton(tr("opencCV"), this->gb_method);
    this->rb_vlfeat = new QRadioButton(tr("VLFeat"), this->gb_method);
    this->rb_opencv->setChecked(true);
    vl = new QVBoxLayout;
    vl->addWidget(this->rb_opencv);
    vl->addWidget(this->rb_vlfeat);
    this->gb_method->setLayout(vl);

    this->gb_source = new QGroupBox(tr("source"), this);
    this->le_pic_1 = new QLineEdit(this->gb_source);
    this->le_pic_2 = new QLineEdit(this->gb_source);
    this->pb_pic_1 = new QPushButton(tr("choose pic 1"), this->gb_source);
    this->pb_pic_2 = new QPushButton(tr("choose pic 2"), this->gb_source);

    hl = new QHBoxLayout;
    vl = new QVBoxLayout;
    hl->addWidget(this->pb_pic_1);
    hl->addWidget(this->le_pic_1);
    vl->addLayout(hl);
    hl = new QHBoxLayout;
    hl->addWidget(this->pb_pic_2);
    hl->addWidget(this->le_pic_2);
    vl->addLayout(hl);
    this->gb_source->setLayout(vl);

    this->pb_do_sift = new QPushButton(tr("do SIFT"), this);
    this->te_info = new QTextEdit(this);
    this->lb_result = new result_label(this);

    vl = new QVBoxLayout;
    hl = new QHBoxLayout;
    vl->addWidget(this->gb_type);
    vl->addWidget(this->gb_method);
    vl->addWidget(this->gb_source);
    vl->addWidget(this->pb_do_sift);
    vl->addWidget(this->te_info);
    hl->addWidget(this->lb_result);
    hl->addLayout(vl);

    this->setLayout(hl);

    this->dsb_good_rate->setMaximum(1.0);
    this->dsb_good_rate->setMinimum(0.0);
    this->dsb_good_rate->setSingleStep(0.1);
    this->dsb_good_rate->setValue(0.6);

    this->rb_detect->setChecked(true);
    this->dsb_good_rate->setDisabled(true);
    this->pb_pic_2->setDisabled(true);
    this->le_pic_2->setDisabled(true);
    this->te_info->setReadOnly(true);

    connect(this->rb_detect, &QRadioButton::clicked,
        [this]()
        {
            this->dsb_good_rate->setDisabled(true);
            this->pb_pic_2->setDisabled(true);
            this->le_pic_2->setDisabled(true);
            this->le_pic_2->clear();
        }
    );
    connect(this->rb_detect_and_match, &QRadioButton::clicked,
        [this]()
        {
            this->dsb_good_rate->setEnabled(true);
            this->pb_pic_2->setEnabled(true);
            this->le_pic_2->setEnabled(true);
        }
    );

    this->recent_path = QString("./");

    connect(this->pb_pic_1, &QPushButton::clicked, this, &sift_demo_widget::choose_pic_1);
    connect(this->pb_pic_2, &QPushButton::clicked, this, &sift_demo_widget::choose_pic_2);
    connect(this->pb_do_sift, &QPushButton::clicked, this, &sift_demo_widget::do_sift);

}

void sift_demo_widget::choose_pic_1()
{
    QString pic1;
    pic1 = QFileDialog::getOpenFileName(this, tr("choose pic 1"), this->recent_path, tr("Image Files (*.png *.jpg *.bmp *.pgm)"));
    if (pic1.isEmpty()) return;
    this->le_pic_1->setText(pic1);
    this->recent_path = pic1.left(pic1.lastIndexOf('/'));
}

void sift_demo_widget::choose_pic_2()
{
    QString pic2;
    pic2 = QFileDialog::getOpenFileName(this, tr("choose pic 2"), this->recent_path, tr("Image Files (*.png *.jpg *.bmp *.pgm)"));
    if (pic2.isEmpty()) return;
    this->le_pic_2->setText(pic2);
    this->recent_path = pic2.left(pic2.lastIndexOf('/'));
}

void sift_demo_widget::do_sift()
{
    static Mat img1, img2;
    static bool match;
    static double rate;
    match = this->rb_detect_and_match->isChecked();
    rate = this->dsb_good_rate->value();
    this->te_info->clear();
    img1 = imread(this->le_pic_1->text().toLocal8Bit().constData());
    if (match) {
        img2 = imread(this->le_pic_2->text().toLocal8Bit().constData());
    }
    if (img1.data == NULL) {
        this->te_info->append(tr("cannot open image:"));
        this->te_info->append(this->le_pic_1->text());
        return;
    }
    if (match && img2.data == NULL) {
        this->te_info->append(tr("cannot open image:"));
        this->te_info->append(this->le_pic_2->text());
        return;
    }
    if (this->rb_opencv->isChecked()) {
        this->sift_opencv(img1, img2, match, rate);
    } else { //this->rb_vlfeat->isChecked()
        this->sift_vlfeat(img1, img2, match, rate);
    }
}

void sift_demo_widget::sift_opencv(Mat &img1, Mat &img2, bool match, double rate)
{
    std::vector<KeyPoint> keypoints1,keypoints2;
    Mat descriptors1,descriptors2;
    Mat img_keypoints;
    QImage result_qimg;
    std::vector<DMatch> matches;//匹配结果
    std::vector<DMatch> goodMatches;
    Mat img_matches;

    SiftFeatureDetector sfd;
    SiftDescriptorExtractor sde;
    BFMatcher bfm;
//FlannBasedMatcher bfm;
    double t = getTickCount();

    sfd.detect(img1, keypoints1);
    sde.compute(img1, keypoints1, descriptors1);
    if (match) {
        sfd.detect(img2, keypoints2);
        sde.compute(img2, keypoints2, descriptors2);
    }
    t = ((double)getTickCount() - t)/getTickFrequency();
    this->te_info->append("图像1特征点个数:"+QString::number(keypoints1.size())+"\n");
    if (match)
        this->te_info->append("图像2特征点个数:"+QString::number(keypoints2.size())+"\n");
    this->te_info->append("SIFT算法用时：" + QString::number(t) + "秒" + "\n");

    //画出特征点
    if (!match) {
        drawKeypoints(img1,keypoints1,img_keypoints,Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        mat2qimage(img_keypoints, result_qimg);
        this->lb_result->setPixmap(QPixmap::fromImage(result_qimg));
        return;
    }
    //特征匹配

    bfm.match(descriptors1, descriptors2, matches);//匹配两个图像的特征矩阵
    this->te_info->append("Match个数：" + QString::number(matches.size()) + "\n");

    //计算匹配结果中距离的最大和最小值
    //距离是指两个特征向量间的欧式距离，表明两个特征的差异，值越小表明两个特征点越接近
    double max_dist;
    double min_dist;
    max_dist = min_dist = matches[0].distance;
    for(int i=0; i<matches.size(); i++)
    {
        double dist = matches[i].distance;
        if(dist < min_dist) min_dist = dist;
        if(dist > max_dist) max_dist = dist;
    }
    this->te_info->append("最大距离：" + QString::number(max_dist) + "\n");
    this->te_info->append("最小距离：" + QString::number(min_dist) + "\n");

    for(int i=0; i<matches.size(); i++)
    {
        if(matches[i].distance <= rate * max_dist)
        {
            goodMatches.push_back(matches[i]);
        }
    }
    this->te_info->append("goodMatch个数：" + QString::number(goodMatches.size()) + "\n");

    //画出匹配结果
    drawMatches(img1,keypoints1,img2,keypoints2,goodMatches,img_matches,
                Scalar::all(-1)/*CV_RGB(255,0,0)*/,CV_RGB(0,255,0),Mat(),DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    mat2qimage(img_matches, result_qimg);
    this->lb_result->setPixmap(QPixmap::fromImage(result_qimg));
}

void sift_demo_widget::sift_vlfeat(Mat &img1, Mat &img2, bool match, double rate)
{
    /*
    VL_PRINT("hello world!\n");
    IplImage img, img2;
    Mat mimg, mimg2;
    mimg = imread(this->le_pic_1->text().toLocal8Bit().constData());
    cvtColor(mimg, mimg2, COLOR_BGR2GRAY);
    img = mimg2;
img2 = mimg;
    int noctaves = 4;
    int nlevels = 2;
    int o_min = 0;
    //vl_sift_pix === float
    vl_sift_pix *img_data = new vl_sift_pix[img.height * img.width];
    unsigned char *pixel;
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            pixel = (unsigned char *)(img.imageData + i * img.width + j);
            img_data[i * img.width + j] = *(pixel);
        }
    }
    VlSiftFilt *sf = NULL;
    sf = vl_sift_new(img.width, img.height, noctaves, nlevels, o_min);
    int key_point = 0;
    int idx = 0;
    if (vl_sift_process_first_octave(sf, img_data) != VL_ERR_EOF) {
        while (true) {
            vl_sift_detect(sf);
            key_point += sf->nkeys;
            VlSiftKeypoint *kp = sf->keys;
            for (int i = 0; i < sf->nkeys; i++) {
                VlSiftKeypoint tmp_kp = *kp;
                kp++;
                cvDrawCircle(&img2, cvPoint(tmp_kp.x, tmp_kp.y), tmp_kp.sigma/2.0, CV_RGB(255,0,0));
                idx++;
                double angles[1024];
                int angle_cnt = vl_sift_calc_keypoint_orientations(sf, angles, &tmp_kp);
                for (int j = 0; j < angle_cnt; j++) {
                    double tmp_angle = angles[j];
                    float *desp = new float[128];
                    vl_sift_calc_keypoint_descriptor(sf, desp, &tmp_kp, tmp_angle);


                    //...
                    delete []desp;
                }
            }
            if (vl_sift_process_next_octave(sf) == VL_ERR_EOF) break;
        }
    }
    vl_sift_delete(sf);
    delete []img_data;
    cvNamedWindow("Source Image",1);
    cvShowImage("Source Image",&img2);
    cvWaitKey(0);
    cvDestroyAllWindows();
    */
}
