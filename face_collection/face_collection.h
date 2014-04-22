#ifndef FACE_COLLECTION_H
#define FACE_COLLECTION_H

extern "C" {
#include <unqlite.h>
}

class face_collection
{
    public:
        face_collection();
        ~face_collection();
        bool load();
        bool create();
        bool new_id();
        bool delete_id();
        bool add_face();
        bool erase_face();
};

#endif // FACE_COLLECTION_H
