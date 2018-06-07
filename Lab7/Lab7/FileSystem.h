//
// Created by elchin on 6/1/18.
//

#ifndef FILESYSTEM_FILESYSTEM_H
#define FILESYSTEM_FILESYSTEM_H

#include <iostream>
#include <string>
#include "Record.h"
#include <vector>

using namespace std;

class FileSystem {
public:
    void start();
    void view_root();
    void view_content();
    void change_dir(string dir_name);
    void remove(string name);
    void open_file(string name);
    void make_file(string name);
    void make_directory(string name);
    FileSystem();
    ~FileSystem();

private:
    FILE *fsys;
    string file_name;
    string temp_file_name;
    Record cur_dir;
    vector<string> root;
    void list_dir(int offset);
    fpos_t block_pos(int offset);
    Record read_info_block(const fpos_t position, fpos_t &content_position);
    void read_content(const fpos_t position);
    void change_dir_up(int offset);
    void change_dir_down();

    void insert_file(fpos_t content_begin, fpos_t content_end, string file_content);

    void make_record(int type, string name);

    int read_node_number();
    void increment_node_number();
    void insert_info();

    void delete_record(int offset);
    fpos_t block_end(const fpos_t block_begin);
    string read_rest(const fpos_t block_end);
    void open_via_vim(int offset);
};


#endif //FILESYSTEM_FILESYSTEM_H
