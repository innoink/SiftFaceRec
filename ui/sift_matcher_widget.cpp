#include <QFileDialog>
#include <vector>
#include <QImage>
#include "sift_matcher_widget.h"
#include "utils/img_fmt_cvt.h"

result_label::result_label(QWidget *parent) :
QLabel(parent)
{
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

QSize result_label::sizeHint() const
{
    return QSize(480, 640);
}

sift_matcher_widget::sift_matcher_widget(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *hl;
    QVBoxLayout *vl;

    //mei yong dao...todo.
    this->lb_pic_1 = new QLabel(this);
    this->lb_pic_2 = new QLabel(this);
    //

    this->lb_result = new result_label(this);
    this->le_pic_1 = new QLineEdit(this);
    this->le_pic_2 = new QLineEdit(this);
    this->pb_pic_1 = new QPushButton(tr("choose pic 1"), this);
    this->pb_pic_2 = new QPushButton(tr("choose pic 2"), this);
    this->pb_match = new QPushButton(tr("match"), this);
    this->te_info = new QTextEdit(this);

    hl = new QHBoxLayout;
    vl = new QVBoxLayout;
    hl->addWidget(this->pb_pic_1);
    hl->addWidget(this->le_pic_1);
    vl->addLayout(hl);
    hl = new QHBoxLayout;
    hl->addWidget(this->pb_pic_2);
    hl->addWidget(this->le_pic_2);
    vl->addLayout(hl);
    vl->addWidget(this->pb_match);
    vl->addWidget(this->te_info);
    hl = new QHBoxLayout;
    hl->addWidget(this->lb_result);
    hl->addLayout(vl);

    this->setLayout(hl);


    connect(this->pb_pic_1, &QPushButton::clicked, this, &sift_matcher_widget::choose_pic_1);
    connect(this->pb_pic_2, &QPushButton::clicked, this, &sift_matcher_widget::choose_pic_2);
    connect(this->pb_match, &QPushButton::clicked, this, &sift_matcher_widget::sift_match);

}

void sift_matcher_widget::choose_pic_1()
{
    QString pic1;
    pic1 = QFileDialog::getOpenFileName(this, tr("choose pic 1"), "./", tr("Image Files (*.png *.jpg *.bmp)"));
    this->le_pic_1->setText(pic1);
}

void sift_matcher_widget::choose_pic_2()
{
    QString pic2;
    pic2 = QFileDialog::getOpenFileName(this, tr("choose pic 2"), "./", tr("Image Files (*.png *.jpg *.bmp)"));
    this->le_pic_2->setText(pic2);
}

void sift_matcher_widget::sift_match()
{
    Mat img1, img2;
    std::vector<KeyPoint> keypoints1,keypoints2;
    Mat descriptors1,descriptors2;

    img1 = imread(this->le_pic_1->text().toLatin1().constData());
    img2 = imread(this->le_pic_2->text().toLatin1().constData());

    this->te_info->clear();

    double t = getTickCount();
    sfd.detect(img1, keypoints1);
    sfd.detect(img2, keypoints2);

    sde.compute(img1, keypoints1, descriptors1);
    sde.compute(img2, keypoints2, descriptors2);
    t = ((double)getTickCount() - t)/getTickFrequency();
    this->te_info->append("图像1特征点个数:"+QString::number(keypoints1.size())+"\n"+
            "图像2特征点个数:"+QString::number(keypoints2.size())+"\n");
    this->te_info->append("SIFT算法用时：" + QString::number(t) + "秒" + "\n");

    //画出特征点
    Mat img_keypoints1,img_keypoints2;
    drawKeypoints(img1,keypoints1,img_keypoints1,Scalar::all(-1),0);
    drawKeypoints(img2,keypoints2,img_keypoints2,Scalar::all(-1),0);

    //特征匹配
    std::vector<DMatch> matches;//匹配结果
    bfm.match( descriptors1, descriptors2, matches );//匹配两个图像的特征矩阵

    this->te_info->append("Match个数：" + QString::number(matches.size()) + "\n");

    //计算匹配结果中距离的最大和最小值
    //距离是指两个特征向量间的欧式距离，表明两个特征的差异，值越小表明两个特征点越接近
    double max_dist = 0;
    double min_dist = 100;
    for(int i=0; i<matches.size(); i++)
    {
        double dist = matches[i].distance;
        if(dist < min_dist) min_dist = dist;
        if(dist > max_dist) max_dist = dist;
    }
    this->te_info->append("最大距离：" + QString::number(max_dist) + "\n");
    this->te_info->append("最小距离：" + QString::number(min_dist) + "\n");

    std::vector<DMatch> goodMatches;
    for(int i=0; i<matches.size(); i++)
    {
        if(matches[i].distance < 0.5 * max_dist)
        {
            goodMatches.push_back(matches[i]);
        }
    }
    this->te_info->append("goodMatch个数：" + QString::number(goodMatches.size()) + "\n");

    //画出匹配结果
    Mat img_matches;
    //红色连接的是匹配的特征点对，绿色是未匹配的特征点
    drawMatches(img1,keypoints1,img2,keypoints2,goodMatches,img_matches,
                Scalar::all(-1)/*CV_RGB(255,0,0)*/,CV_RGB(0,255,0),Mat(),2);
    QImage result_qimg;
    mat2qimage(img_matches, result_qimg);
    this->lb_result->setPixmap(QPixmap::fromImage(result_qimg));
}
