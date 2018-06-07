//
// Created by elchin on 6/1/18.
//

#ifndef FILESYSTEM_RECORD_H
#define FILESYSTEM_RECORD_H

#include <string>
#include <map>
using namespace std;

class Record {
public:
    int node;
    int type;
    string name;
    int parent;
    map<int, string> dir_list;
};


#endif //FILESYSTEM_RECORD_H
