#ifndef MULTIACCESSES_H
#define MULTIACCESSES_H
#include "cconfiginfo.h"
#include "../common/rh_ifcfg.h"

typedef struct {
    char AccessIp[20];
    char ip[20];
    char netmask[20];
    char gateway[20];
    char dns1[20];
} AccessStruct;

struct Node {
   AccessStruct data;
   Node *next;
};

typedef Node AccessStructNode;


class MultiAccesses
{
public:
    MultiAccesses();
    ~MultiAccesses();
    void initLocalNetInfo();
    void set_default(AccessStruct &defaultAccessStruct);
    AccessStruct get_default();
    int writeFile();
    int readFile();

    bool push(AccessStruct AccessStructData);
    bool pop();
    AccessStruct top();
    int size();

private:
    char fileName[255];
    int AccessStructStackSize;
    AccessStruct m_defaultAccessStruct;
    AccessStructNode *m_AccessStructStackHead;
    rh_ifcfg *m_ifcfg;
};


#endif // MULTIACCESSES_H
