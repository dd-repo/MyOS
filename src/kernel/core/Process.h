#ifndef CORE_PROCESS_H
#define CORE_PROCESS_H

#include <util/cpp.h>
#include <util/LinkedList.h>
#include <core/Thread.h>
#include <memory/AddressSpace.h>
#include <io/FileObject.h>
#include <elf/ELF.h>

#include <dirent.h>


class ProcessDirFDHolder {
public:
    ProcessDirFDHolder(char* path);
    ~ProcessDirFDHolder();
    dirent*    read();
    void     rewind();
private:    
    LinkedList<dirent*>* nodes;
    int pos;
};


class Process {
public:
    Process();
    ~Process();
    static u32int create(char* path, int argc, char** argv, FileObject* stdin, FileObject* stdout, FileObject* stderr);
    void*         requestMemory(u32int sz);
    u32int        openFile(FileObject* f);
    void          reopenFile(u32int fd, FileObject* f);
    u32int        openDir(char* path);
    void          closeDir(u32int dfd);

    u32int pid;
    char* name;
    bool dead;
    FileObject* files[256];
    ProcessDirFDHolder* dirs[256];

    Process* parent;
    LinkedList<Thread*>* threads;
    LinkedList<Process*>* children;
    AddressSpace* addrSpace;

private:
    u32int allocatorTop, lastFD, lastDFD;
};
#endif
