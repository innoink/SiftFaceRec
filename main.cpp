#include <QApplication>
#include "ui/main_window.h"

#include "face_collection/face_collection.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    vedis_lib_config(VEDIS_LIB_CONFIG_THREAD_LEVEL_MULTI);
    qDebug() << vedis_lib_is_threadsafe();

    fprintf(stderr, "%f\n", FLT_MAX);
    main_window w;
    w.show();

    return a.exec();
}


/*
int main()
{
    face_collection fc;
    fc.create("test", "/tmp/test.fcdb");
    fc.new_id(1, "lq");
    std::string name;
    fc.clt_name(name);
    std::cout << name << std::endl;
    fc.id_name(1, name);
    std::cout << name << std::endl;
    int cnt;
    fc.id_face_cnt(1, &cnt);
    std::cout << cnt << endl;
    name.append(" LQ");
    fc.id_rename(1, name);
    fc.id_name(1, name);
    std::cout << name << endl;
    fc.save();
    fc.close();
    return 0;
}
*/
