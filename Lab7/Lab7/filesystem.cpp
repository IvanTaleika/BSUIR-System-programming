#include "filesystem.h"

FileSystem::FileSystem(QString fileName, short clusterSize) {
  this->clusterSize =  clusterSize;
  recordSize =  sizeof(Record);
  recordInfoSize = sizeof(RecordInfo);
  data.setFileName(fileName);
  if (data.exists()) {
    data.open(QIODevice::ReadWrite);
  }
  clustersTable = data.read(clusterSize);
  currentCluster = data.read(clusterSize);
  currentDir = (Record*) currentCluster.data();
  path = currentDir->name;
  path += '/';
}

FileSystem::~FileSystem() {
  writeCurrent();
  data.seek(0);
  data.write(clustersTable);
  data.close();
}

QString FileSystem::getDirContent() {
  QString content;
  QDataStream stream(&currentCluster, QIODevice::ReadOnly);
  stream.device()->seek(recordSize);
  RecordInfo recordInfo;
  stream.readRawData((char*)&recordInfo, recordInfoSize);
  while (!stream.atEnd() && recordInfo.cluster != 0) {
    content += recordInfo.type == DIR ? "dir: " : "file: ";
    content += recordInfo.name;
    content += '\n';
    stream.readRawData((char*)&recordInfo, recordInfoSize);
  }
  return content;
}

QString FileSystem::getError() {
  return error;

}

QString FileSystem::getPath() {
  return path;
}

bool FileSystem::changeDir(char* name) {
  if (!strcmp(name, "..")) {
    if (currentDir->directoryCluster == -1) {
      return true;
    }
    int currentDirNameLength = strlen(currentDir->name);
    writeCurrent();
    data.seek(currentDir->directoryCluster * clusterSize);
    currentCluster = data.read(clusterSize);
    currentDir = (Record*) currentCluster.data();
    path.truncate(path.length() - currentDirNameLength - 2);
    path += '/';
    return true;
  } else {
    QDataStream stream(&currentCluster, QIODevice::ReadOnly);
    stream.device()->seek(recordSize);
    RecordInfo recordInfo;
    stream.readRawData((char*)&recordInfo, recordInfoSize);
    while (!stream.atEnd() && recordInfo.cluster != 0) {
      if (recordInfo.type == DIR && !strcmp(recordInfo.name, name)) {
        writeCurrent();
        data.seek(recordInfo.cluster * clusterSize);
        currentCluster = data.read(clusterSize);
        currentDir = (Record*) currentCluster.data();
        path += currentDir->name;
        path += '/';
        return true;
      }
      stream.readRawData((char*)&recordInfo, recordInfoSize);
    }
  }
  error = "No such directory.";
  return false;
}

bool FileSystem::addDir(char* name) {
  return addRecord(name, DIR);
}

bool FileSystem::addFile(char* name) {
  return addRecord(name, FILE);
}

bool FileSystem::removeFile(char* name) {
  QDataStream stream(&currentCluster, QIODevice::ReadWrite);
  stream.device()->seek(recordSize);
  RecordInfo recordInfo;
  stream.readRawData((char*)&recordInfo, recordInfoSize);
  while (!stream.atEnd() && recordInfo.cluster != 0) {
    if (recordInfo.type == FILE && !strcmp(recordInfo.name, name)) {
      removeFileByCluster(recordInfo.cluster);
      repairDir(stream);
      return true;
    }
    stream.readRawData((char*)&recordInfo, recordInfoSize);
  }
  error = "No such file";
  return false;
}



bool FileSystem::removeDir(char* name) {
  QDataStream stream(&currentCluster, QIODevice::ReadWrite);
  stream.device()->seek(recordSize);
  RecordInfo recordInfo;
  stream.readRawData((char*)&recordInfo, recordInfoSize);
  while (!stream.atEnd() && recordInfo.cluster != 0) {
    if (recordInfo.type == DIR && !strcmp(recordInfo.name, name)) {
      short currentClusterNumber = currentDir->cluster;
      writeCurrent();
      recursiveDirRemoving(recordInfo.cluster);
      data.seek(currentClusterNumber * clusterSize);
      currentCluster = data.read(clusterSize);
      repairDir(stream);
      return true;
    }
    stream.readRawData((char*)&recordInfo, recordInfoSize);
  }
  error = "No such directory";
  return false;
}



bool FileSystem::openFile(char* name) {
  QDataStream stream(&currentCluster, QIODevice::ReadWrite);
  stream.device()->seek(recordSize);
  RecordInfo recordInfo;
  stream.readRawData((char*)&recordInfo, recordInfoSize);
  while (!stream.atEnd() && recordInfo.cluster != 0) {
    if (recordInfo.type == FILE && !strcmp(recordInfo.name, name)) {
      data.seek(recordInfo.cluster * clusterSize);
      Record record;
      data.read((char*)&record, recordSize);
      QFile file("~temp");
      while (file.exists()) {
        file.setFileName(file.fileName() + "1");
      }
      if (!file.open(QIODevice::WriteOnly)) {
        error = "Unknown error";
        return false;
      }
      file.write(data.read(record.size));
      while (record.childCluster != -1) {
        data.seek(record.childCluster * clusterSize);
        data.read((char*)&record, recordSize);
        file.write(data.read(record.size));
      }
      file.close();
      QString command = "vi ";
      command += file.fileName();
      system(command.toUtf8().data());
      file.open(QIODevice::ReadOnly);
      data.seek(recordInfo.cluster * clusterSize);
      data.read((char*)&record, recordSize);
      QByteArray temp(clusterSize - recordSize, '\0');
      int nBytes = file.read(temp.data(), clusterSize - recordSize);
      data.write(temp);
      record.size = nBytes;
      data.seek(record.cluster * clusterSize);
      data.write((char*)&record, recordSize);
      while (nBytes == clusterSize - recordSize) {
        if (record.childCluster == -1) {
          short freeCluster = takeFreeCluster();
          if (freeCluster == -1) {
            error = "No memory. File will be truncated.";
            return false;
          }
          record.childCluster = freeCluster;
          data.seek(record.cluster * clusterSize);
          data.write((char*)&record, recordSize);
          record = createRecord(freeCluster, record.directoryCluster, record.cluster, FILE, record.name);
          data.seek(record.cluster * clusterSize);
          data.write((char*)&record, recordSize);
        } else {
          data.seek(record.childCluster * clusterSize);
          data.read((char*)&record, recordSize);
        }
        nBytes = file.read(temp.data(), clusterSize - recordSize);
        data.write(temp);
        record.size = nBytes;
        data.seek(record.cluster * clusterSize);
        data.write((char*)&record, recordSize);
      }
      file.remove();
      if (record.childCluster != -1) {
        removeFileByCluster(record.childCluster);
        record.childCluster = -1;
        data.seek(record.cluster * clusterSize);
        data.write((char*)&record, recordSize);
      }
      return true;
    }
    stream.readRawData((char*)&recordInfo, recordInfoSize);
  }
  error = "No such file";
  return false;
}

bool FileSystem::addRecord(char* name, int type) {
  QDataStream stream(&currentCluster, QIODevice::ReadWrite);
  stream.device()->seek(recordSize);
  RecordInfo recordInfo;
  stream.readRawData((char*)&recordInfo, recordInfoSize);
  while (!stream.atEnd()) {
    if (recordInfo.type == type && !strcmp(recordInfo.name, name)) {
      error = "Such record is already exist.";
      return false;
    }
    if (recordInfo.cluster == 0) {
      int pos = takeFreeCluster();
      if (pos == -1) {
        break;
      }
      recordInfo.cluster = pos;
      strcpy(recordInfo.name, name);
      recordInfo.type = type;
      stream.device()->seek(stream.device()->pos() - recordInfoSize);
      stream.writeRawData((char*)&recordInfo, recordInfoSize);
      writeCurrent();

      Record record = createRecord(pos, currentDir->cluster, -1, type, name);
      data.seek(pos * clusterSize);
      data.write((char*)&record, recordSize);

      return true;
    }
    stream.readRawData((char*)&recordInfo, recordInfoSize);
  }
  error = "Not enough memory.";
  return false;
}

short FileSystem::takeFreeCluster() {
  short i = 2;
  while (i < clusterSize) {
    if (clustersTable[i] == '\0') {
      clustersTable.replace(i, 1, "1", 1);
      return i;
    }
    i++;
  }
  return -1;
}

void FileSystem::returnCluster(int nCluster) {
  clustersTable.replace(nCluster, 1, "\0", 1);
}

Record FileSystem::createRecord(short cluster, short directoryCluster, short parentCluster, short type, char* name) {
  Record record;
  record.childCluster = -1;
  record.cluster = cluster;
  record.directoryCluster = directoryCluster;
  strcpy(record.name, name);
  record.parentCluster = parentCluster;
  record.size = 0;
  record.type = type;
  return record;
}

void FileSystem::writeCurrent() {
  data.seek(currentDir->cluster * clusterSize);
  data.write(currentCluster);
}

void FileSystem::recursiveDirRemoving(short nCluster) {
  data.seek(nCluster * clusterSize);
  currentCluster = data.read(clusterSize);
  QDataStream stream(&currentCluster, QIODevice::ReadWrite);
  stream.device()->seek(recordSize);
  RecordInfo recordInfo;
  stream.readRawData((char*)&recordInfo, recordInfoSize);
  while (!stream.atEnd() && recordInfo.cluster != 0) {
    if (recordInfo.type == DIR) {
      recursiveDirRemoving(recordInfo.cluster);
      data.seek(nCluster * clusterSize);
      currentCluster = data.read(clusterSize);
    } else  {
      removeFileByCluster(recordInfo.cluster);
    }
    stream.readRawData((char*)&recordInfo, recordInfoSize);
  }
  QByteArray emptyCluster(clusterSize, '\0');
  data.seek(nCluster * clusterSize);
  data.write(emptyCluster);
  returnCluster(nCluster);
}

void FileSystem::repairDir(QDataStream& stream) {
  RecordInfo recordInfo;
  while (true) {

    if (stream.atEnd()) {
      stream.setStatus(QDataStream::Ok);
      stream.device()->seek(stream.device()->pos() - recordInfoSize);
      recordInfo.cluster = 0;
      recordInfo.type = 0;
      strcpy(recordInfo.name, "\0");
      break;
    }
    stream.readRawData((char*)&recordInfo, recordInfoSize);
    stream.device()->seek(stream.device()->pos() - 2 * recordInfoSize);
    if (recordInfo.cluster == 0) {
      break;
    }

    stream.writeRawData((char*)&recordInfo, recordInfoSize);
    stream.device()->seek(stream.device()->pos() + recordInfoSize);
  }
  stream.writeRawData((char*)&recordInfo, recordInfoSize);
}

void FileSystem::removeFileByCluster(char nCluster) {
  int sourceCluster = nCluster;
  Record fileRecord;
  while (true) {
    data.seek(nCluster * clusterSize);
    data.read((char*)&fileRecord, recordSize);
    if (fileRecord.childCluster == -1) {
      break;
    }
    nCluster = fileRecord.childCluster;
  }
  QByteArray emptyCluster(clusterSize, '\0');
  while (true) {
    data.seek(nCluster * clusterSize);
    data.read((char*)&fileRecord, recordSize);
    data.seek(nCluster * clusterSize);
    data.write(emptyCluster);
    returnCluster(nCluster);
    if (nCluster == sourceCluster) {
      return;
    }
    nCluster = fileRecord.parentCluster;
  }
}

