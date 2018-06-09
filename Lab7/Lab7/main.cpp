#include <QCoreApplication>
#include "filesystem.h"
#include <QTextStream>

#define FILE_SIZE 65536
#define CLUSTER_SIZE 256
#define FILE_PATH "../Lab7/FileSystemData"

struct Commands {
  const QString ls = "ls";
  const QString cd = "cd ";
  const QString mkdir = "mkdir ";
  const QString touch = "touch ";
  const QString rmFile = "rm ";
  const QString rmDir = "rm -r ";
  const QString vi = "vi ";
} commands;

bool createFile() {
  QFile file(FILE_PATH);
  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  }
  QByteArray data(FILE_SIZE, '\0');
  data.replace(0, 2, "11", 2);
  Record root;
  root.childCluster = -1;
  root.cluster = 1;
  root.directoryCluster = -1;
  strcpy(root.name, "\0");
  root.parentCluster = -1;
  root.size = 0;
  root.type = FileSystem::DIR;
  data.replace(CLUSTER_SIZE, sizeof(Record), (char*)&root, sizeof(Record));
  file.write(data);
  file.close();
  return true;
}

int main(int argc, char* argv[]) {
  QTextStream cout(stdout);
  QTextStream cin(stdin);
  if (!QFile::exists(FILE_PATH)) {
    if (!createFile()) {
      cout << "Source data cannot be created." << endl;
      return 1;
    }
  }
  FileSystem fileSystem(FILE_PATH, CLUSTER_SIZE);
  QString input;
  bool isGood = true;
  while (true) {
    if (!isGood) {
      cout << fileSystem.getError() << endl;
    }
    cout << fileSystem.getPath() << ">" << flush;
    input = cin.readLine();
    isGood = true;
    if (input == commands.ls) {
      cout << fileSystem.getDirContent() << flush;
    } else if (input.startsWith(commands.cd)) {
      isGood = fileSystem.changeDir(input.remove(commands.cd).toUtf8().data());
    } else if (input.startsWith(commands.mkdir)) {
      isGood = fileSystem.addDir(input.remove(commands.mkdir).toUtf8().data());
    } else if (input.startsWith(commands.touch)) {
      isGood = fileSystem.addFile(input.remove(commands.touch).toUtf8().data());
    } else if (input.startsWith(commands.rmDir)) {
      isGood = fileSystem.removeDir(input.remove(commands.rmDir).toUtf8().data());
    } else if (input.startsWith(commands.rmFile)) {
      isGood = fileSystem.removeFile(input.remove(commands.rmFile).toUtf8().data());
    } else if (input.startsWith(commands.vi)) {
      isGood = fileSystem.openFile(input.remove(commands.vi).toUtf8().data());
    } else if (input == "exit") {
      return 0;
    } else {
      cout << "Unknown command" << endl;
    }
  }
  return 0;
}
