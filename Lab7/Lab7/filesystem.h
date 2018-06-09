#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include <QString>;
#include <QFile>
#include <QByteArray>
#include <QDataStream>

struct Record {
  short cluster;
  short directoryCluster;
  short childCluster;
  short parentCluster;
  short type;
  short size;
  char name[16];
};

struct RecordInfo {
  char name[16];
  short cluster;
  short type;
};



class FileSystem {
 public:
  static const int DIR = 1;
  static const int FILE = 0;

  FileSystem(QString fileName, short clusterSize);
  ~FileSystem();
  QString getDirContent();
  QString getError();
  QString getPath();
  bool changeDir(char* name);
  bool addDir(char* name);
  bool addFile(char* name);
  bool removeFile(char* name);
  bool removeDir(char* name);
  bool openFile(char* name);

 private:
  QString error;
  QFile data;
  QByteArray clustersTable;
  QByteArray currentCluster;
  Record* currentDir;
  QString path;

  short clusterSize;
  short recordSize;
  short recordInfoSize;

  bool addRecord(char* name, int type);
  short takeFreeCluster();
  void returnCluster(int nCluster);
  Record createRecord(short cluster, short directoryCluster, short parentCluster, short type, char* name);
  void writeCurrent();
  void recursiveDirRemoving(short nCluster);
  void repairDir(QDataStream& stream);
  void removeFileByCluster(char nCluster);
};

#endif // FILESYSTEM_H
