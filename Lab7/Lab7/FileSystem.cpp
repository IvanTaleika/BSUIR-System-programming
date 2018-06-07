//
// Created by elchin on 6/1/18.
//

#include "FileSystem.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <_G_config.h>
#include "Record.h"

using namespace std;

struct rec {
    string name;
    int type;
    int own;
};


void FileSystem::start() {
    fsys = fopen(file_name.c_str(), "r+");
    if (fsys == NULL) {
        cout << "Error";
    }
    const int start_dir = 0;

    fpos_t con_pos;
    fpos_t rec_pos = block_pos(start_dir);
    cur_dir = read_info_block(rec_pos, con_pos);
    cur_dir.parent = start_dir;
    read_content(con_pos);
    root.push_back(cur_dir.name);
}

void FileSystem::list_dir(int offset) {
    rewind(fsys);
    fpos_t con_pos;
    fpos_t rec_pos = block_pos(offset);
    Record rec = read_info_block(rec_pos, con_pos);
    cur_dir.dir_list.insert(pair<int, string>(offset, rec.name));
}

fpos_t FileSystem::block_pos(int offset) {
    rewind(fsys);
    int cur_offset = -1;
    char buf[256];
    while (!feof(fsys)) {
        fscanf(fsys, "%s", buf);
        fpos_t position;
        if (buf[0] == '#') {
            fgetpos(fsys, &position);
            fscanf(fsys, "%d", &cur_offset);
        }
        if (cur_offset == offset) {
            return position;
        }
    }
}

Record FileSystem::read_info_block(const fpos_t position, fpos_t &content_position) {
    char buf[256];
    Record rec;
    fsetpos(fsys, &position);
    fscanf(fsys, "%s", buf);
    rec.node = atoi(buf);
    fscanf(fsys, "%s", buf);
    rec.type = atoi(buf);
    fscanf(fsys, "%s", buf);
    rec.name = buf;
    fgetpos(fsys, &content_position);
    return rec;
}

void FileSystem::read_content(const fpos_t position) {
    fsetpos(fsys, &position);
    vector<int> elems;
    char buf[256];
    while (!feof(fsys)) {
        fscanf(fsys, "%s", buf);
        if (!strcmp(buf, "$$$")) {
            break;
        }
        else {
            elems.push_back(atoi(buf));
        }
    }
    while (!elems.empty()) {
        list_dir(elems.back());
        elems.pop_back();
    }
    return;
}

void FileSystem::view_root() {
    for (auto it = root.begin(); it != root.end(); ++it) {
        cout << "->" << (*it);
    }
    cout << ' ';
}

void FileSystem::view_content() {
    for (auto it = cur_dir.dir_list.begin(); it != cur_dir.dir_list.end(); ++it) {
        cout << (*it).second << endl;
    }
}

void FileSystem::change_dir_up(int offset) {
    fpos_t con_pos;
    fpos_t rec_pos = block_pos(offset);
    int parent = cur_dir.node;
    Record up_dir = read_info_block(rec_pos, con_pos);
    if (up_dir.type == 0) {
        cur_dir = up_dir; //is directory
    } else {
        cout << "No such directory!" << endl;
        return;
    }
    cur_dir.parent = parent;
    read_content(con_pos);
    root.push_back(cur_dir.name);
    // read_content(cur_content_pos);
}

void FileSystem::change_dir_down() {
    if (cur_dir.parent == cur_dir.node) {
        cout << "You in root directory" << endl;
        return;
    }
    string dir_name = cur_dir.name;
    fpos_t con_pos;
    fpos_t rec_pos = block_pos(cur_dir.parent);
    cur_dir = read_info_block(rec_pos, con_pos);
    read_content(con_pos);
    root.pop_back();
}

void FileSystem::make_record(int type, string name) {
    if (name.empty()) {
        cout << "Enter name" << endl;
        return;
    }
    for (auto it = cur_dir.dir_list.begin(); it != cur_dir.dir_list.end(); ++it) {
        if ((*it).second == name) {
            cout << "Already exists" << endl;
            return;
        }
    }
    int node = read_node_number();
    fseek(fsys, 0, SEEK_END);
    fprintf(fsys, "\n# %d %d %s $$$", ++node, type, name.c_str());
    fflush(fsys);
    cur_dir.dir_list.insert(pair<int, string>(node, name));
    increment_node_number();
    insert_info();
    fflush(fsys);
}

int FileSystem::read_node_number() {
    int i = 0;
    rewind(fsys);
    fscanf(fsys, "%d", &i);
    return i;
}

void FileSystem::increment_node_number() {
    int node = read_node_number();
    node++;
    rewind(fsys);
    fprintf(fsys, "%-10d$$$", node);
}

void FileSystem::insert_info() {
    fpos_t con_pos, end_con_pos;
    fpos_t rec_pos = block_pos(cur_dir.node);
    read_info_block(rec_pos, con_pos);
    string file;
    char buf[256];
    while (!feof(fsys)) {
        fscanf(fsys, "%s", buf);
        if (!strcmp(buf, "$$$")) {
            fgetpos(fsys, &end_con_pos);
            break;
        }
    }
    while (true) {
        char b = fgetc(fsys);
        if (b == EOF) break;
        file = file + b;
    }
    fsetpos(fsys, &con_pos);
    for (auto it = cur_dir.dir_list.begin(); it != cur_dir.dir_list.end(); ++it) {
        fprintf(fsys, " %d", (*it).first);
    }
    ftruncate(fileno(fsys), ftell(fsys));
    fflush(fsys);
    fprintf(fsys, " $$$");
    fprintf(fsys, "%s", file.c_str());
    fflush(fsys);
}

void FileSystem::change_dir(string dir_name) {
    if (dir_name == "..") {
        change_dir_down();
    }
    for (auto it = cur_dir.dir_list.begin(); it != cur_dir.dir_list.end(); ++it) {
        if ((*it).second == dir_name) {
            change_dir_up((*it).first);
            return;
        }
    }
}

void FileSystem::delete_record(int offset) {
    fpos_t block_begin = block_pos(offset);
    fpos_t block_end = this->block_end(block_begin);
    string file_rest = read_rest(block_end);
    file_rest.erase(0, 1);
    block_begin.__pos -= 1;
    fsetpos(fsys, &block_begin);
    fprintf(fsys, "%s", file_rest.c_str());
    fflush(fsys);
    int size = block_begin.__pos + file_rest.size();
    ftruncate(fileno(fsys), size);
    fflush(fsys);
    cur_dir.dir_list.erase(offset);
    insert_info();
    return;
}

fpos_t FileSystem::block_end(const fpos_t block_begin) {
    fsetpos(fsys, &block_begin);
    fpos_t block_end;
    char buf[256];
    while (!feof(fsys)) {
        fscanf(fsys, "%s", buf);
        if (!strcmp(buf, "$$$")) {
            fgetpos(fsys, &block_end);
            break;
        }
    }
    return block_end;
}

string FileSystem::read_rest(const fpos_t block_end) {
    fsetpos(fsys, &block_end);
    string file_rest;
    while (true) {
        char c = fgetc(fsys);
        if (c == EOF) break;
        file_rest = file_rest + c;
    }
    return file_rest;
}

void FileSystem::remove(string name) {
    for (auto it = cur_dir.dir_list.begin(); it != cur_dir.dir_list.end(); ++it) {
        if ((*it).second == name) {
            delete_record((*it).first);
            return;
        }
    }
}

void FileSystem::open_via_vim(int offset) {
    fpos_t  con_pos;
    fpos_t block_begin = block_pos(offset);
    fpos_t block_end = this->block_end(block_begin);
    Record rec = read_info_block(block_begin, con_pos);
    if (rec.type !=1) {
        cout << "That's not a file" << endl;
        return;
    }
    fsetpos(fsys, &con_pos);
    char buf[256];
    string file_content;
    while (!feof(fsys)) {
        fscanf(fsys, "%[^\n]s", buf);
        file_content += buf;
    }

    file_content.erase(0, 1);
    file_content.erase(file_content.end() - 3, file_content.end());
    FILE *temp = fopen(temp_file_name.c_str(), "w+");
    fprintf(temp, "%s", file_content.c_str());
    fclose(temp);

    string vim_file = "vim " + temp_file_name;
    system(vim_file.c_str());

    file_content.clear();
    temp = fopen(temp_file_name.c_str(), "r");
    fseek(temp, 0, SEEK_END);
    int size = ftell(temp);
    rewind(temp);
    fread(buf, sizeof(char), size, temp);
    fclose(temp);
    if (size == 0) {
        buf[0] = '\0';
    }
    else {
        buf[size - 1] = '\0';
    }
    file_content = buf;
    insert_file(con_pos, block_end, file_content);

    return;
}

void FileSystem::insert_file(fpos_t content_begin, fpos_t content_end, string file_content) {
    string file;
    fsetpos(fsys, &content_end);

    while (true) {
        char b = fgetc(fsys);
        if (b == EOF) break;
        file = file + b;
    }

    fsetpos(fsys, &content_begin);
    fprintf(fsys, " %s", file_content.c_str());

    ftruncate(fileno(fsys), ftell(fsys));
    fflush(fsys);
    fprintf(fsys, " $$$");
    fprintf(fsys, "%s", file.c_str());
    fflush(fsys);
}

void FileSystem::open_file(string name) {
    for (auto it = cur_dir.dir_list.begin(); it != cur_dir.dir_list.end(); ++it) {
        if ((*it).second == name) {
            open_via_vim((*it).first);
            return;
        }
    }
    cout << "No such file" << endl;
}

void FileSystem::make_directory(string name) {
    make_record(0, name);
}

void FileSystem::make_file(string name) {
    make_record(1, name);
}

FileSystem::FileSystem() {
    file_name = "FileSys";
    temp_file_name = "~temp";

}

FileSystem::~FileSystem() {

}
