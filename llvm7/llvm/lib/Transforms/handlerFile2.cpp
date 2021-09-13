const char *customHandler = R"(
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
using namespace std;

bool *int2kFlag = NULL;
vector<int> *e = NULL;
map<int*, int> mapAddr; 
map<string, int> mapName;
int **mapID = NULL;
char **funcName = NULL;
int n;

#ifdef __cplusplus
extern "C" {
#endif
  void _int2k_add_edge(char*, char*);
  void _int2k_check(char*);
  void _int2k_apply(char*, int);
  void _int2k_translateAddr(int *, int );
  void _int2k_init_flag(int);
  void _int2k_delete_flag();
  void _int2k_set_flag(char*, int);
  int _int2k_handle_arg(int, char**);
  void _int2k_translateName(char*, int);
#ifdef __cplusplus
}
#endif

int toInt(char *s) {
  int res = 0;
  for(int i = 0; i < strlen(s); ++i)
  {
    if(s[i] > '9' || s[i] < '0')
      return -1;
    res = res * 10 + s[i] - '0';
  }
  return res;
}

#ifdef __cplusplus 
extern "C" int _int2k_handle_arg(int argc, char **argv) {
#else
int _int2k_handle_arg(int argc, char **argv) { 
#endif
  for(int i = 1; i < argc; ++i)
  {
    char *s = argv[i];
    printf("%s\n", s);
    int id = -1;
    if(mapName.find(s) == mapName.end())
    {
      id = toInt(s);
      if(id == -1) {
        printf("can't find %s\n",s);
      }
    } else {
      id = mapName[s];
    }
    if(id != -1) {
      int2kFlag[id] = 1;
      printf("deactivate %s\n", funcName[id]);
    }
  } 
  printf("\n");
  return 0;
}

#ifdef __cplusplus 
extern "C" void _int2k_translateName(char* name,int ID) {
#else
void _int2k_translateName(char* name, int ID) { 
#endif
  mapName[name] = ID;
  funcName[ID] = name;
}



// called after visiting all functions
#ifdef __cplusplus
extern "C" void _int2k_add_edge(char *f1, char *f2) {
#else
void _int2k_add_edge(char *f1, char *f2) {
#endif
  int id1 = mapName[f1];
  int id2 = mapName[f2];
  e[id1].push_back(id2);
}

#ifdef __cplusplus
extern "C" void _int2k_init_flag(int len) {
#else
void _int2k_init_flag(int len) {
#endif
  int2kFlag = new bool[len];
  mapID = new int*[len];
  funcName = new char*[len];
  for(int i = 0; i < len; ++i)
    int2kFlag[i] = 0;
  e = new vector<int>[len];
  n = len;
}

#ifdef __cplusplus
extern "C" void _int2k_delete_flag() {
#else
void _int2k_delete_flag() {
#endif
  delete [] int2kFlag;
  for(int i = 0; i < n; ++i)
    e[i].clear();
  delete [] e;
  delete [] mapID;
  delete [] funcName;
}

#ifdef __cplusplus
extern "C" void _int2k_translateAddr(int *funcPtr, int id) {
#else
void _int2k_translateAddr(int *funcPtr, int id) {
#endif
    mapAddr[funcPtr] = id;
    mapID[id] = funcPtr;
}

#ifdef __cplusplus
extern "C" void _int2k_check(char* funcName) {
#else
void _int2k_check(char *funcName) {
#endif
  int id = mapName[funcName];
  if(int2kFlag[id] == 0)
    return;
  else
    exit(0);
}

#ifdef __cplusplus
extern "C" void _int2k_apply(char *funcName, int val) {
#else
void _int2k_apply(char *funcName, int val) {
#endif
  int id = mapName[funcName];
  for(int i = 0; i < (int)e[id].size(); ++i)
  {
    int2kFlag[e[id][i]] = val;
  }
}

#ifdef __cplusplus
extern "C" void _int2k_set_flag(char *funcName, int val) {
#else
void _int2k_set_flag(char *funcName, int val) {
#endif
  int id = mapName[funcName];
  int2kFlag[id] = val;
}

)";