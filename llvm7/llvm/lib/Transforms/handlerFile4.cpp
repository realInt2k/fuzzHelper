const char *customHandler = R"(
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int cmpMapAddr(const void* xx, const void* yy)
{
  int **x = (int **)xx;
  int **y = (int **)yy;
  if(*x < *y)
    return -1;
  else if (*x > *y)
    return 1;
  else 
    return 0;
}

struct MapAddr {
  int **key;
  int *item;
  int nElements = 0;
  void init(int n) {
    key = (int **)malloc(n * sizeof(int*));
    item = (int *)malloc(n * sizeof(int));
    nElements = 0;
  }
  void destroy() {
    free(key);
    free(item);
  }
  void insert(int* y) {
    key[nElements++] = y;
  }
  void sort() {
    qsort(key, nElements, sizeof(int *), cmpMapAddr);
  }
  int find(int *addr) {
    int l = 0, r = nElements - 1;
    while (l <= r)
    {
      int mid = (l + r) >> 1;
      if(key[mid] < addr) {
        l = mid + 1;
      } else if(key[mid] > addr) {
        r = mid - 1;
      } else {
        return mid;
      }
    }
    return -1;
  }
  int *operator [] (int *addr) {
    int id = find(addr);
    if(id == -1)
      return NULL;
    else
      return item + id;
  }

  int end() {
    return -1;
  }
} mapAddr; 

int cmpStr(const char *s1, const char *s2)
{
  int nS1 = strlen(s1);
  int nS2 = strlen(s2);
  if(nS1 < nS2)
    return -1;
  else if(nS1 > nS2)
    return 1;
  else {
    for(int i = 0; i < nS1; ++i)
    {
      if(s1[i] < s2[i])
        return -1;
      else if (s1[i] > s2[i])
        return 1;
    }
    return 0;
  }
}

int cmpMapName(const void *a, const void *b)
{
  const char **ia = (const char **)a;
  const char **ib = (const char **)b;
  return cmpStr(*ia, *ib);
}


struct MapName {
  char **key;
  int *item;
  int nElements = 0;
  void init(int n) {
    key = (char **)malloc(n * sizeof(char*));
    item = (int *)malloc(n * sizeof(int));
    nElements = 0;
  }
  void destroy() {
    free(key);
    free(item);
  }
  void insert(char *y) {
    key[nElements++] = y;
  }
  int find(char *s) {    
    int l = 0, r = nElements - 1;
    while (l <= r)
    {
      int mid = (l + r) >> 1;
      if(cmpStr(key[mid], s) < 0) {
        l = mid + 1;
      } else if(cmpStr(key[mid], s) > 0) {
        r = mid - 1;
      } else {
        return mid;
      }
    }
    return -1;
  }
  int* operator [] (char *s) {
    int id = find(s);
    if(id == -1)
      return NULL;
    else
      return item + id;
  }
  void sort() {
    qsort(key, nElements, sizeof(char *), cmpMapName);
  }
  int end() {
    return -1;
  }
} mapName;

int **mapID;
char **funcName;
int n;
bool *int2kFlag;

struct edges {
  int *node;
  int sz;
  int capacity;
  void destroy()
  {
    free(node);
  }
  void init()
  {
    sz = 0;
    node = NULL;
    capacity = 0;
  }
  void insert(int anotherNode) {
    if(capacity == 0)
    {
      sz = 1;
      capacity = 1;
      node = (int *)malloc(1 * sizeof(int));
      node[0] = anotherNode;
    } else {
      capacity *= 2;
      free(node);
      node = (int *)malloc(capacity * sizeof(int));
      node[sz++] = anotherNode;
    }
  }
  int at(int index) {
    if(index >= sz)
      return -1;
    else
      return node[index];
  }
} *e;

#ifdef __cplusplus
extern "C" {
#endif
  extern void _int2k_add_edge(int*, int*);
  extern void _int2k_check(int *);
  extern void _int2k_apply(int *, int);
  extern void _int2k_translateAddr(int *, int );
  extern void _int2k_init_flag(int);
  extern void _int2k_delete_flag();
  extern void _int2k_set_flag(int*, int);
  extern int _int2k_handle_arg(int, char**);
  extern void _int2k_translateName(char*, int);
  extern void _int2k_insertMapAddr(int*);
  extern void _int2k_sortMapAddr();
  extern void _int2k_insertMapName(char*);
  extern void _int2k_sortMapName();
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
extern "C" void _int2k_insertMapAddr(int *addr) {
#else
void _int2k_insertMapAddr(int *addr) {
#endif
    mapAddr.insert(addr);
}

#ifdef __cplusplus
extern "C" void _int2k_sortMapAddr() {
#else
void _int2k_sortMapAddr() {
#endif
    mapAddr.sort();
}

#ifdef __cplusplus
extern "C" void _int2k_insertMapName(char *name) {
#else
void _int2k_insertMapName(char *name) {
#endif
    mapName.insert(name);
}

#ifdef __cplusplus
extern "C" void _int2k_sortMapName() {
#else
void _int2k_sortMapName() {
#endif
    mapName.sort();
}


#ifdef __cplusplus 
extern "C" int _int2k_handle_arg(int argc, char **argv) {
#else
int _int2k_handle_arg(int argc, char **argv) { 
#endif
  for(int i = 1; i < argc; ++i)
  {
    char *s = argv[i];
    int id = -1;
    if(mapName.find(s) == mapName.end())
    {
      id = toInt(s);
      if(id == -1) {
        printf("can't find %s\n",s);
      }
    } else {
      id = *mapName[s];
    }
    if(id != -1) {
      int2kFlag[id] = 1;
      printf("deactivate %s\n", funcName[id]);
    }
  } 
  return 0;
}

#ifdef __cplusplus 
extern "C" void _int2k_translateName(char* name,int ID) {
#else
void _int2k_translateName(char* name, int ID) { 
#endif
  *mapName[name] = ID;
  //printf("translateName %s\n", name);
  funcName[ID] = name;
}



// called after visiting all functions
#ifdef __cplusplus
extern "C" void _int2k_add_edge(int*f1, int*f2) {
#else
void _int2k_add_edge(int*f1, int*f2) {
#endif
  int id1 = *mapAddr[f1];
  int id2 = *mapAddr[f2];
  e[id1].insert(id2); 
}

#ifdef __cplusplus
extern "C" void _int2k_init_flag(int len) {
#else
void _int2k_init_flag(int len) {
#endif
  mapAddr.init(len);
  mapName.init(len);
  int2kFlag = (bool *)malloc(len * sizeof(bool));
  mapID = (int **)malloc(len * sizeof(int*));
  funcName = (char **)malloc(len * sizeof(char*));
  for(int i = 0; i < len; ++i)
    int2kFlag[i] = 0;
  e = (struct edges*)malloc(len * sizeof(struct edges));
  for(int i = 0; i < len; ++i)
    e[i].init();
  n = len;
}

#ifdef __cplusplus
extern "C" void _int2k_delete_flag() {
#else
void _int2k_delete_flag() {
#endif
  mapAddr.destroy();
  mapName.destroy();
  free(int2kFlag);
  for(int i = 0; i < n; ++i)
    e[i].destroy();
  free(e);
  free(mapID);
  free(funcName);
}

#ifdef __cplusplus
extern "C" void _int2k_translateAddr(int *funcPtr, int id) {
#else
void _int2k_translateAddr(int *funcPtr, int id) {
#endif
    *mapAddr[funcPtr] = id;
    mapID[id] = funcPtr;
}

#ifdef __cplusplus
extern "C" void _int2k_check(int* funcPtr) {
#else
void _int2k_check(int *funcPtr) {
#endif
  int *checker = mapAddr[funcPtr];
  if(checker == NULL)
    return;
  int id = *checker;
  //printf("id is %d funcPtr %p\n", id, funcPtr);
  if(int2kFlag[id] == 0)
    return;
  else
    exit(0);
}

#ifdef __cplusplus
extern "C" void _int2k_apply(int *funcPtr, int val) {
#else
void _int2k_apply(char *funcPtr) {
#endif
  int id = *mapAddr[funcPtr];
  for(int i = 0; i < e[id].sz; ++i)
  {
    int2kFlag[e[id].at(i)] = val;
  }
}

#ifdef __cplusplus
extern "C" void _int2k_set_flag(int *funcPtr, int val) {
#else
void _int2k_set_flag(int *funcPtr, int val) {
#endif
  int id = *mapAddr[funcPtr];
  int2kFlag[id] = val;
}
)";