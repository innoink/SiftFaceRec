#include "face_recognizer.h"

#include <climits>
#include "face_collection/face_collection.h"

void print_kpds(std::vector<sift_keypoint_descr_t> &kpds)
{
    for (unsigned i = 0; i < kpds.size(); i++) {
        printf("->x=%f, y=%f, sigma=%f, angle=%f\n", kpds[i].x, kpds[i].y, kpds[i].sigma, kpds[i].angle);
        printf("->descr=");
        for (unsigned j = 0; j < 128; j++) {
            printf("%f  ", kpds[i].descr[j]);
        }
        printf("\n");
    }

}

face_recognizer::face_recognizer()
{
}

void face_recognizer::train(Mat &img, std::vector<struct sift_keypoint_descr_t> &kpds)
{
    enter_func();

    IplImage iplimg = img;
    int noctaves = -1;
    int nlevels = 3;
    int o_min = 0;
    //vl_sift_pix === float
    vl_sift_pix *img_data = new vl_sift_pix[iplimg.height * iplimg.width];
    unsigned char *pixel;
    for (int i = 0; i < iplimg.height; i++) {
        for (int j = 0; j < iplimg.width; j++) {
            pixel = (unsigned char *)(iplimg.imageData + i * iplimg.width + j);
            img_data[i * iplimg.width + j] = *(pixel);
        }
    }
    kpds.clear();
    VlSiftFilt *sf = NULL;
    sf = vl_sift_new(iplimg.width, iplimg.height, noctaves, nlevels, o_min);
    int kp_cnt = 0;
    int idx = 0;
    if (vl_sift_process_first_octave(sf, img_data) != VL_ERR_EOF) {
        while (true) {
            vl_sift_detect(sf);
            kp_cnt += sf->nkeys;
            VlSiftKeypoint *kp = sf->keys;
            for (int i = 0; i < sf->nkeys; i++) {
                const VlSiftKeypoint *k = kp + i;
                idx++;
                double angles[4];
                int angle_cnt = vl_sift_calc_keypoint_orientations(sf, angles, k);
                for (int j = 0; j < angle_cnt; j++) {
                    struct sift_keypoint_descr_t kpd;
                    memset(&kpd, 0x00, sizeof(kpd));

                    vl_sift_calc_keypoint_descriptor(sf, kpd.descr, k, angles[j]);
                    kpd.x = k->x;
                    kpd.y = k->y;
                    kpd.sigma = k->sigma;//scale
                    kpd.angle = angles[j];//orientation
                    kpds.push_back(kpd);
                }
            }
            if (vl_sift_process_next_octave(sf) == VL_ERR_EOF) break;
        }
    }
    vl_sift_delete(sf);
    delete []img_data;
    leave_func();
}

void face_recognizer::match(std::vector<sift_keypoint_descr_t> &kpds1, std::vector<sift_keypoint_descr_t> &kpds2, double thresh, int *m_cnt)
{
    int k1, k2;
    *m_cnt = 0;
    for (k1 = 0; k1 < kpds1.size(); ++k1) {
        float best = FLT_MAX;
        float second_best = FLT_MAX;
        int bestk = -1;

        for (k2 = 0; k2 < kpds2.size(); ++k2) {
            float acc = 0;
            for (int i = 0; i < 128; ++i) {
                float delta = kpds1.at(k1).descr[i] -
                              kpds2.at(k2).descr[i];
                acc += delta * delta;
            }
            fprintf(stderr, "acc = %f, best = %f\n", acc, best);
            if(acc < best) {
                second_best = best ;
                best = acc ;
                bestk = k2 ;
            } else if(acc < second_best) {
                second_best = acc ;
            }
        }
        /* Lowe's method: accept the match only if unique. */
        if(thresh * second_best > best &&
            bestk != -1) {
            fprintf(stderr, "k1=%d k2=%d score=%f\n", k1, bestk, best);
                (*m_cnt)++;
        }
    }
}
