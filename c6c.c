#include <stdio.h>
#include "calc3.h"
#include "y.tab.h"
#include <stdlib.h>
#include <string.h>

// for debugging
// #define DEBUG


extern int var_count, func_count, loc_var_count;
extern int vType[200];
extern char* func[200];
extern int argTable[200];
int funcIdx = -1;
int hasreturn = 0;

static int lbl;

//structs for array
typedef struct
{
  char* name;
  int size;
} oneDArray;

typedef struct
{
  char* name;
  int fst_size;
  int scd_size;
} twoDArray;

typedef struct
{
  char* name;
  int fst_size;
  int scd_size;
  int thd_size;
} threeDArray;

oneDArray* onedarray_list[50];
twoDArray* twodarray_list[50];
threeDArray* threedarray_list[50];

int onedarray_count = 0;
int twodarray_count = 0;
int threedarray_count = 0;

// function declaration
//sym related
int inSYM(char *var_name);
void insertSYM(char * var_name, int isGlobal);
int getSYMIdx(char *var_name, int isGlobal);
void emptySYM(int isGlobal);
void insert1DArraySYM(char * array_name, int array_size, int is_global);
void insert2DArraySYM(char * array_name, int array_fst_size, int array_scd_size, int is_global);
void insert3DArraySYM(char * array_name, int array_fst_size, int array_scd_size, int array_thd_size, int is_global);

void printsp(int coun);

void prepass(nodeType *p, int infunc);

void printStackTop(int type);

// array list related
void insert1DArrayList(char* name, int size);
void insert2DArrayList(char* name, int fst_size, int scd_size);
void insert3DArrayList(char* name, int fst_size, int scd_size, int thd_size);
oneDArray* getOneDArray(char* name);
twoDArray* getTwoDArray(char* name);
threeDArray* getThreeDArray(char* name);

//function related
void insertFUNC(char * func_name);
int getFUNCIdx(char * func_name);
void emptyFUNC();
void insertArg(int argnum, int idx);
void setType (int index, int t);

int checkExprType (nodeType* p);
int checkType (int index);

// helper functions
void printSYM();


/*
 function to print the symbol table
*/
void printSYM()
{
#ifdef DEBUG
  printf("[DEBUG]\t\tGlobal SYM begin:\n");
  int i;
  for(i = 0; i < var_count; i++)
  {
    printf("[DEBUG]\t\tidx: %d\tvar_name:\t%s\n", i, sym[i]);
  }
  printf("[DEBUG]\t\tGlobal SYM end\n");
  printf("[DEBUG]\t\tLocal SYM begin:\n");
  for(i = 0; i < loc_var_count; i++)
  {
    printf("[DEBUG]\t\tidx: %d\tvar_name:\t%s\n", i, sym[i + 100]);
  }
  printf("[DEBUG]\t\tLocal SYM end\n");
#endif
}

/*
function to let nas know the number of global variables
*/
void printsp(int coun){
  printf("\tpush\tsp\n");
  printf("\tpush\t%d\n", coun);
  printf("\tadd\n");
  printf("\tpop\tsp\n");
}

/*
function for prepassing the whole tree
  p: node
  infunc:
    0: outside functions
    1: inside functions
*/
void prepass(nodeType *p, int infunc){
  if (!p) return;
  switch(p->type) {
    case typeFunc:
    insertFUNC(p->func.name);
    insertArg(p->func.argc, getFUNCIdx(p->func.name));
    prepass(p->func.args, 1);
    prepass(p->func.op, 1);
    break;
    case typeId:
    if (infunc == 0){
      p->id.isGlobal=1;
    }
    insertSYM(p->id.var_name, p->id.isGlobal);
    break;
    case typeOneDArray:
    {
      if(infunc == 0)
      {
        p->onedarray.name->id.isGlobal = 1;
      }
      break;
    }
    case typeTwoDArray:
    {
      if(infunc == 0)
      {
        p->twodarray.name->id.isGlobal = 1;
      }
    }
    case typeThreeDArray:
    {
      if(infunc == 0)
      {
        p->threedarray.name->id.isGlobal = 1;
      }
    }
    case typeOpr:
    switch(p->opr.oper) {
      case MAIN:
      prepass(p->opr.op[0], infunc);
      prepass(p->opr.op[1], infunc);
      break;
      case FOR:
      prepass(p->opr.op[0], infunc);
      prepass(p->opr.op[1], infunc);
      prepass(p->opr.op[3], infunc);
      prepass(p->opr.op[2], infunc);
      break;
      case WHILE:
      prepass(p->opr.op[0], infunc);
      prepass(p->opr.op[1], infunc);
      break;
      case RETURN:
      prepass(p->opr.op[0], infunc);
      break;
      case CONTINUE:
      break;
      case BREAK:
      break;
      case IF:
      if (p->opr.nops > 2) {
        /* if else then */
        prepass(p->opr.op[0], infunc);
        prepass(p->opr.op[1], infunc);
        prepass(p->opr.op[2], infunc);
      } else {
        /* if else */
        prepass(p->opr.op[0], infunc);
        prepass(p->opr.op[1], infunc);
      }
      break;
      case '=':
      if(p->opr.op[0]->type == typeId) {
        /* insert into symbol table */
        if (infunc == 0){
          p->opr.op[0]->id.isGlobal=1;
        }
        insertSYM(p->opr.op[0]->id.var_name, p->opr.op[0]->id.isGlobal);
      }else if(p->opr.op[0]->type == typeOpr && p->opr.op[0]->opr.oper == '@'){
        prepass(p->opr.op[0], infunc);
      }
      prepass(p->opr.op[1], infunc);
      break;
      // case '@':
      // if(p->opr.op[0]->type == typeId && infunc == 1) {
      //   /* insert into symbol table */
      //   insertSYM(p->opr.op[0]->id.var_name, 1);
      // }else{
      //   yyerror("Wrong @ usage!");
      //   exit(0);
      // }
      break;
      case ARRAY_DECLARE:
      {
// #ifdef DEBUG
//         printf("[DEBUG]\tprepass func, array declaration\n");
// #endif
        char * array_name = p->opr.op[0]->id.var_name;
        // int is_global = p->opr.op[0]->id.isGlobal;
        int array_fst_size = p->opr.op[1]->con.value;

        // if outside function then the array is global, the reverse is not true
        if (infunc == 0){
          p->opr.op[0]->id.isGlobal = 1;
        }

        if(p->opr.nops == 2)//1D Array
        {
          insert1DArraySYM(array_name, array_fst_size, p->opr.op[0]->id.isGlobal);
          insert1DArrayList(array_name, array_fst_size);
        }
        if(p->opr.nops == 3)//2D Array
        {
          int array_scd_size = p->opr.op[2]->con.value;
          insert2DArraySYM(array_name, array_fst_size, array_scd_size, p->opr.op[0]->id.isGlobal);
          insert2DArrayList(array_name, array_fst_size, array_scd_size);
        }
        if(p->opr.nops == 4)//3D Array
        {
          int array_scd_size = p->opr.op[2]->con.value;
          int array_thd_size = p->opr.op[3]->con.value;
          insert3DArraySYM(array_name, array_fst_size, array_scd_size, array_thd_size, p->opr.op[0]->id.isGlobal);
          insert3DArrayList(array_name, array_fst_size, array_scd_size, array_thd_size);
        }
        break;
      }
      case GETI:
      case GETS:
      case GETC:
      case PUTI:
      case PUTI_:
      case PUTC:
      case PUTC_:
      case PUTS:
      case PUTS_:
      prepass(p->opr.op[0], infunc);
      break;
      case CALL:
      prepass(p->opr.op[1], infunc);
      break;
      case ';':
      prepass(p->opr.op[0],infunc);
      prepass(p->opr.op[1],infunc);
      break;
      default:
      prepass(p->opr.op[0],infunc);
      prepass(p->opr.op[1],infunc);
      break;
    }
    default:
    break;
  }
}

/*
function to deal with printing for different type
*/
void printStackTop(int type){

  if(type == 1){
    printf("\tputi\n");
  }else if(type == 2){
    printf("\tputs\n");
  }else if(type == 3){
    printf("\tputc\n");
  }else{
    // unknown type
    yyerror("unknown type for printing");
  }
}

/*
function for checking existance of a variable in the symbol table
var_name: variable name
*/
int inSYM(char * var_name)
{
  int i;
  for(i = 0; i < var_count; i++)
  {
    if(strcmp(sym[i], var_name) == 0)
    {
      return 1; // in sym
    }
  }
  return 0; // not in sym
}

/*
function for adding variables to symbol table
var_name: variable name
*/
void insertSYM(char * var_name, int isGlobal)
{
  if(var_count >= 200)
  {
    printf("\tNumber of variables exceeds the limit!\n");
    return;
  }
  if(getSYMIdx(var_name, isGlobal) < 0)//not in sym
  {
    if(isGlobal == 1){ //global variable
      sym[var_count] = (char *) malloc (strlen(var_name));
      strcpy(sym[var_count], var_name);
      var_count ++;
    } else {
      sym[loc_var_count+100] = (char *) malloc (strlen(var_name));
      strcpy(sym[loc_var_count+100], var_name);
      loc_var_count ++;
    }
  }
  else
  {
    // do nothing
    //printf("\tError: variable -%s- already declared!\n", var_name);
  }
}

/* function for inserting an array name into the SYM
array_name: Name of the array
array_size: Size of the array
is_global: if the array is declared globally
*/
void insert1DArraySYM(char * array_name, int array_size, int is_global)
{
// #ifdef DEBUG
//   printf("[DEBUG]\tinsert1DArraySYM func: Inserting array into sym\n");
// #endif
  // check for resource availability
  if(var_count > 200)
  {
    printf("\t Size of the array: %s exceeds the memory limit!\n", array_name);
    return;
  }

  if(getSYMIdx(array_name, is_global) < 0)//not in sym
  {
// #ifdef DEBUG
//         printf("[DEBUG]\tinsert1DArraySYM func: got array index\n");
// #endif

    if(is_global == 1)
    {
      // global
      int i;
      for(i = 0; i < array_size; i++)
      {
// #ifdef DEBUG
//         printf("[DEBUG]\tinsert1DArraySYM func: Inserted one element\n");
// #endif
        sym[var_count] = (char *) malloc (strlen(array_name));
        strcpy(sym[var_count], array_name);
        var_count ++;
      }
    }
    else
    {
      // local
      int i;
      for(i = 0; i < array_size; i++)
      {
// #ifdef DEBUG
//         printf("[DEBUG]\tLocal: Inserted one element\n");
// #endif
        sym[loc_var_count+100] = (char *) malloc (strlen(array_name));
        strcpy(sym[loc_var_count+100], array_name);
        loc_var_count ++;
      }

    }
  }
}

/* function for inserting an array name into the SYM
array_name: Name of the array
array_fst_size: Size of the the first dimension
array_scd_size: Size of the second dimension
is_global: if the array is declared globally
*/
void insert2DArraySYM(char * array_name, int array_fst_size, int array_scd_size, int is_global)
{
// #ifdef DEBUG
//   printf("[DEBUG]\tinsert1DArraySYM func: Inserting array into sym\n");
// #endif
  // check for resource availability
  if(var_count > 200)
  {
    printf("\t Size of the array: %s exceeds the memory limit!\n", array_name);
    return;
  }

  if(getSYMIdx(array_name, is_global) < 0)//not in sym
  {
// #ifdef DEBUG
//         printf("[DEBUG]\tinsert1DArraySYM func: got array index\n");
// #endif

    if(is_global == 1)
    {
      // global
      int i;
      for(i = 0; i < array_fst_size; i++)
      {
        int j;
        for(j = 0; j < array_scd_size; j++)
        {
          sym[var_count] = (char *) malloc (strlen(array_name));
          strcpy(sym[var_count], array_name);
          var_count ++;
        }
      }
    }
    else
    {
      // local
      int i;
      for(i = 0; i < array_fst_size; i++)
      {
        int j;
        for(j = 0; j < array_scd_size; j++)
        {
          sym[loc_var_count+100] = (char *) malloc (strlen(array_name));
          strcpy(sym[loc_var_count+100], array_name);
          loc_var_count ++;
        }
      }

    }
  }

}


/* function for inserting an array name into the SYM
array_name: Name of the array
array_fst_size: Size of the first dimension
array_scd_size: Size of the second dimension
array_thd_size: Size of the third dimension
is_global: if the array is declared globally
*/
void insert3DArraySYM(char * array_name, int array_fst_size, int array_scd_size, int array_thd_size, int is_global)
{
// #ifdef DEBUG
//   printf("[DEBUG]\tinsert1DArraySYM func: Inserting array into sym\n");
// #endif
  // check for resource availability
  if(var_count > 200)
  {
    printf("\t Size of the array: %s exceeds the memory limit!\n", array_name);
    return;
  }

  if(getSYMIdx(array_name, is_global) < 0)//not in sym
  {
// #ifdef DEBUG
//         printf("[DEBUG]\tinsert1DArraySYM func: got array index\n");
// #endif

    if(is_global == 1)
    {
      // global
      int i;
      for(i = 0; i < array_fst_size; i++)
      {
        int j;
        for(j = 0; j < array_scd_size; j++)
        {
          int k;
          for(k = 0; k < array_thd_size; k++)
          {
            sym[var_count] = (char *) malloc (strlen(array_name));
            strcpy(sym[var_count], array_name);
            var_count ++;
          }
        }
      }
    }
    else
    {
      // local
      int i;
      for(i = 0; i < array_fst_size; i++)
      {
        int j;
        for(j = 0; j < array_scd_size; j++)
        {
          int k;
          for(k = 0; k < array_thd_size; k++)
          {
            sym[loc_var_count+100] = (char *) malloc (strlen(array_name));
            strcpy(sym[loc_var_count+100], array_name);
            loc_var_count ++;
          }
        }
      }

    }
  }

}


/*
Insert an 1D array into the list
name: Array name
size: Array size
*/
void insert1DArrayList(char* name, int size)
{
  if(onedarray_count < 50)
  {
    oneDArray* array;
    int nodeSize;
    nodeSize = sizeof(array);
    if((array = malloc(nodeSize)) == NULL)
    {
      yyerror("Error: not enough memory for allocating 1D array list\n");
      return;
    }

    array->name = name;
    array->size = size;

    onedarray_list[onedarray_count] = array;
    onedarray_count++;
  }
  else
  {
    printf("Number of arrays exceeds limit: %d at most\n", 50);
  }
}

/*
Insert a 2D Array into the list
name: Array name
row: row size
col: column size
*/
void insert2DArrayList(char* name, int fst_size, int scd_size)
{
  if(twodarray_count < 50)
  {
    twoDArray* array;
    int nodeSize;
    nodeSize = sizeof(array);
    if((array = malloc(nodeSize)) == NULL)
    {
      yyerror("Error: not enough memory for allocating 2D array list\n");
      return;
    }

    array->name = name;
    array->fst_size = fst_size;
    array->scd_size = scd_size;

    twodarray_list[twodarray_count] = array;
    twodarray_count++;
  }
  else
  {
    printf("Number of arrays exceeds limit: %d at most\n", 50);
  }
}

/*
Insert a 3D Array into the list
name: Array name
fst_size: row size
scd_size: column size
thd_size: the third size
*/
void insert3DArrayList(char* name, int fst_size, int scd_size, int thd_size)
{
  if(threedarray_count < 50)
  {
    threeDArray* array;
    int nodeSize;
    nodeSize = sizeof(array);
    if((array = malloc(nodeSize)) == NULL)
    {
      yyerror("Error: not enough memory for allocating 3D array list\n");
      return;
    }

    array->name = name;
    array->fst_size = fst_size;
    array->scd_size = scd_size;
    array->thd_size = thd_size;

    threedarray_list[threedarray_count] = array;
    threedarray_count++;
  }
  else
  {
    printf("Number of arrays exceeds limit: %d at most\n", 50);
  }
}


/*
get the settings of an 1D array
*/
oneDArray* getOneDArray(char* name)
{
  int i;
  for(i = 0; i < onedarray_count; i++)
  {
    if(strcmp(onedarray_list[i]->name, name) == 0)
    {
      return onedarray_list[i];
    }
  }
  return NULL;
}

/*
get the settings of a 2D array
*/
twoDArray* getTwoDArray(char* name)
{
  int i;
  for(i = 0; i < twodarray_count; i++)
  {
    if(strcmp(twodarray_list[i]->name, name) == 0)
    {
      return twodarray_list[i];
    }
  }
  return NULL;
}

/*
get the settings of a 3D array
*/
threeDArray* getThreeDArray(char* name)
{
  int i;
  for(i = 0; i < threedarray_count; i++)
  {
    if(strcmp(threedarray_list[i]->name, name) == 0)
    {
      return threedarray_list[i];
    }
  }
  return NULL;
}

/*
function for getting index of a variable in sym
*/
int getSYMIdx(char * var_name, int isGlobal)
{
  int i;
  if (isGlobal == 1){ // for global variable
// #ifdef DEBUG
//         printf("[DEBUG]\t getSYMIdx func: global == 1\n");
// #endif
    for(i = 0; i < var_count; i++)
    {
      if(strcmp(sym[i], var_name) == 0)
      {
        return i;
      }
    }
  }else{ // for local variable
    for(i = 100; i < 100 + loc_var_count; i++)
    {
      if(strcmp(sym[i], var_name) == 0)
      {
        return i;
      }
    }
  }

// #ifdef DEBUG
//   printf("[DEBUG] getSYMIdx func: return -1\n");
// #endif
  return -1; // not found
}

/*
function for clearing the symbol table
*/
void emptySYM(int isGlobal)
{
  int i;
  if(isGlobal){
    for(i = 0; i < var_count; i++)
    {
      free(sym[i]);
    }
    var_count = 0;
  }
  else{
    for(i = 100; i < 100 + loc_var_count; i++)
    {
      free(sym[i]);
    }
    loc_var_count = 0;
  }
}

/*
function to get type for stored variable
*/
int checkType (int index)
{
  return vType[index];
}

/*
function for adding functions to func table
func_name: function name
*/
void insertFUNC(char * func_name)
{
  if(func_count >= 200)
  {
    printf("\tNumber of variables exceeds the limit!\n");
    return;
  }

  if(getFUNCIdx(func_name) < 0)//not in sym
  {
    func[func_count] = (char *) malloc (strlen(func_name));
    strcpy(func[func_count], func_name);
    func_count ++;
  }
  else
  {
    // the function has already been defined
    yyerror("\tFunction has already been defined!\n");
    exit(0);
  }
}

/*
function for getting index of a function in func
*/
int getFUNCIdx(char * func_name)
{
  int i;
  for(i = 0; i < func_count; i++)
  {
    if(strcmp(func[i], func_name) == 0)
    {
      return i;
    }
  }
  return -1; // not found
}

/*
function for clearing the function table
*/
void emptyFUNC()
{
  int i;
  for(i = 0; i < func_count; i++)
  {
    free(func[i]);
  }
  func_count = 0;
}

void insertArg(int argnum, int idx)
{
  argTable[idx] = argnum;
}

/*
function to set type for some variable
*/
void setType (int index, int t)
{
  vType[index] = t;
  return;
}

/*
function for checking type for an expression
*/
int checkExprType (nodeType* p){
  switch(p->type) {
    case typeId:{
      char *str = p->id.var_name;
      int index = getSYMIdx(str, p->id.isGlobal);
      int type = checkType(index);
      return type;
    }
    case typeCon:
    return 1;
    case typeStrCon:
    return 2;
    case typeCharCon:
    return 3;
    case typeOpr:
    switch(p->opr.oper) {
      case UMINUS:
      return 1;
      default:
      return 1;
    }
  }
}

int ex(nodeType *p, int blbl, int clbl, int infunc) {
  int lblx, lbly, lblz;

  if (!p) return 0;
  switch(p->type) {
    case typeCon:
    {
      printf("\tpush\t%d\n", p->con.value);
      break;
    }
    case typeCharCon:
    {
      printf("\tpush\t%s\n", p->charCon.value);
      break;
    }
    case typeStrCon:
    {
      printf("\tpush\t\"%s\"\n", p->strCon.value);
      break;
    }
    case typeId:
    {
      char *str = p->id.var_name;
      int index = getSYMIdx(str, p->id.isGlobal);
      if (index==-1)
      {
        printf("\tError: Variable \"%s\" uninitialized\n", str);
        exit(0);
      }
      if (p->id.isGlobal == 1)
      {
        printf("\tpush\tsb[%d]\n", index);
      }
      else { // for local
        index = index-100;
        if (index >= argTable[funcIdx]) // for parameter ? local variable????
        {
          printf("\tpush\tfp[%d]\n",  index - argTable[funcIdx]);
        }
        else // for local variable ? parameter???
        {
          printf("\tpush\tfp[%d]\n",  -3 - argTable[funcIdx] + index); //3 is accounted for the saved caller's pc, fp, sp
        }
      }
      break;
    }
    case typeFunc:
    {
      funcIdx = getFUNCIdx(p->func.name);
      prepass(p->func.args, 1);
      prepass(p->func.op, 1);

      printf("\tjmp\tL%03d\n", 501 + 2*getFUNCIdx(p->func.name));
      printf("L%03d:\n", 500 + 2*getFUNCIdx(p->func.name));
      //printf("loc_var_count: %d, argc: %d\n", loc_var_count, p->func.argc);
      printsp(loc_var_count-argTable[funcIdx]);
      ex(p->func.op, blbl, clbl, 1);
      // place to exit the function, check whether there is a return,
      if (hasreturn == 0){
        printf("\tpush\t0\n");
        printf("\tret\n");
      } else {
        hasreturn=0;
      }
      emptySYM(0);
      printf("L%03d:\n", 501 + 2*getFUNCIdx(p->func.name));
      break;
    }
    case typeOneDArray:
    {
      char * array_name = p->onedarray.name->id.var_name;
      int index = getSYMIdx(array_name, p->onedarray.name->id.isGlobal);
      if (index==-1)
      {
        printf("\t[Debug]: Array name %s, is global: %d\n", array_name, p->onedarray.name->id.isGlobal);
        printf("\tError: Array \"%s\" Undeclared\n", array_name);
        exit(0);
      }
      //TODO check array index outofbound

      // evaluate the expression to get index
      ex(p->onedarray.index, -1, -1, infunc);

      // get array index offset
      // check if the array is global
      if (p->onedarray.name->id.isGlobal == 1)
      {
        // global
        printf("\tpush\t%d\n", index);
      }
      else
      { // for local
        printf("\tpush\t%d\n", index-100);
      }

      // add up offset and the index
      printf("\tadd\n");

      // pop the index to the in register
      printf("\tpop\tin\n");

      if(p->onedarray.name->id.isGlobal == 1)
      {
        // global
        printf("\tpush\tsb[in]\n");
      }
      else
      {
        // local
        printf("\tpush\tfp[in]\n");
      }
      break;
    }

    case typeTwoDArray:
    {
      char * array_name = p->twodarray.name->id.var_name;
      int index = getSYMIdx(array_name, p->twodarray.name->id.isGlobal);
      if (index==-1)
      {
        printf("\t[Debug]: Array name %s, is global: %d\n", array_name, p->twodarray.name->id.isGlobal);
        printf("\tError: Array \"%s\" Undeclared\n", array_name);
        exit(0);
      }

      twoDArray* array = getTwoDArray(array_name);
      int scd_size = array->scd_size;
      int fst_size = array->fst_size;

      // evaluate the expression to get index
      ex(p->twodarray.scd_index, -1, -1, infunc);
      ex(p->twodarray.fst_index, -1, -1, infunc);

      //TODO check array index outofbound
      printf("\tpush\t%d\n", scd_size);
      printf("\tmul\n");
      printf("\tadd\n");

      // get array index offset
      // check if the array is global
      if (p->twodarray.name->id.isGlobal == 1)
      {
        // global
        printf("\tpush\t%d\n", index);
      }
      else
      { // for local
        printf("\tpush\t%d\n", index-100);
      }

      // add up offset and the index
      printf("\tadd\n");

      // pop the index to the in register
      printf("\tpop\tin\n");

      if(p->twodarray.name->id.isGlobal == 1)
      {
        // global
        printf("\tpush\tsb[in]\n");
      }
      else
      {
        // local
        printf("\tpush\tfp[in]\n");
      }
      break;
    }
    case typeThreeDArray:
    {
      char * array_name = p->threedarray.name->id.var_name;
      int index = getSYMIdx(array_name, p->threedarray.name->id.isGlobal);
      if (index==-1)
      {
        printf("\t[Debug]: Array name %s, is global: %d\n", array_name, p->threedarray.name->id.isGlobal);
        printf("\tError: Array \"%s\" Undeclared\n", array_name);
        exit(0);
      }

      threeDArray* array = getThreeDArray(array_name);
      int fst_size = array->fst_size;
      int scd_size = array->scd_size;
      int thd_size = array->thd_size;

      // evaluate the expression to get index
      ex(p->threedarray.thd_index, -1, -1, infunc);
      ex(p->threedarray.scd_index, -1, -1, infunc);
      ex(p->threedarray.fst_index, -1, -1, infunc);

      //TODO check array index outofbound
      printf("\tpush\t%d\n", scd_size);
      printf("\tmul\n");
      printf("\tadd\n");

      printf("\tpush\t%d\n", thd_size);
      printf("\tmul\n");
      printf("\tadd\n");

      // get array index offset
      // check if the array is global
      if (p->threedarray.name->id.isGlobal == 1)
      {
        // global
        printf("\tpush\t%d\n", index);
      }
      else
      { // for local
        printf("\tpush\t%d\n", index-100);
      }

      // add up offset and the index
      printf("\tadd\n");

      // pop the index to the in register
      printf("\tpop\tin\n");

      if(p->threedarray.name->id.isGlobal == 1)
      {
        // global
        printf("\tpush\tsb[in]\n");
      }
      else
      {
        // local
        printf("\tpush\tfp[in]\n");
      }
      break;
    }
    case typeOpr:
    switch(p->opr.oper) {
      case RETURN:
      {
        hasreturn=1;
        ex(p->opr.op[0], -1, -1, infunc);
        printf("\tret\n");
        break;
      }
      case BREAK:
      {
        printf("\tjmp\tL%03d\n", blbl);
        break;
      }
      case CONTINUE:
      {
        printf("\tjmp\tL%03d\n", clbl);
        break;
      }
      case FOR:
      {
        lblx = lbl++;
        lbly = lbl++;
        lblz = lbl++;
        ex(p->opr.op[0], blbl, clbl, infunc);
        printf("L%03d:\n", lblx);
        ex(p->opr.op[1], blbl, clbl, infunc);
        printf("\tj0\tL%03d\n", lbly);
        ex(p->opr.op[3], lbly, lblz, infunc);
        printf("L%03d:\n", lblz);
        ex(p->opr.op[2], blbl, clbl, infunc);
        printf("\tjmp\tL%03d\n", lblx);
        printf("L%03d:\n", lbly);
        break;
      }
      case WHILE:
      {
        lblx = lbl++;
        lbly = lbl++;
        printf("L%03d:\n", lblx);
        ex(p->opr.op[0], blbl, clbl, infunc);
        printf("\tj0\tL%03d\n", lbly);
        ex(p->opr.op[1], lbly, lblx, infunc);
        printf("\tjmp\tL%03d\n", lblx);
        printf("L%03d:\n", lbly);
        break;
      }
      case IF:
      {
        ex(p->opr.op[0], blbl, clbl, infunc);
        if (p->opr.nops > 2) {
          /* if else */
          printf("\tj0\tL%03d\n", lblx = lbl++);
          ex(p->opr.op[1], blbl, clbl, infunc);
          printf("\tjmp\tL%03d\n", lbly = lbl++);
          printf("L%03d:\n", lblx);
          ex(p->opr.op[2], blbl, clbl, infunc);
          printf("L%03d:\n", lbly);
        } else {
          /* if */
          printf("\tj0\tL%03d\n", lblx = lbl++);
          ex(p->opr.op[1], blbl, clbl, infunc);
          printf("L%03d:\n", lblx);
        }
        break;
      }
      // case '@':
      // {
      //   ex(p->opr.op[0], blbl, clbl, infunc);
      //   break;
      // }
      case CALL:
      {
        ex(p->opr.op[1], blbl, clbl, infunc);
        int thisfuncIdx = getFUNCIdx(p->opr.op[0]->id.var_name);
        printf("\tcall\tL%03d, %d\n", 500+2*thisfuncIdx, argTable[thisfuncIdx]);
        break;
      }
      case ARRAY_DECLARE:
      {
        break;
      }
      case '=':
      {
        if(p->opr.nops == 2)// Variable assignment:  vari = expr
        {
          ex(p->opr.op[1], blbl, clbl, infunc);
          if(p->opr.op[0]->type == typeId) {
            /* examine type */
            int index = getSYMIdx(p->opr.op[0]->id.var_name, p->opr.op[0]->id.isGlobal);
            int thisT = checkExprType(p->opr.op[1]);
            setType(index, thisT);
            if (index == -1)
            insertSYM(p->opr.op[0]->id.var_name, p->opr.op[0]->id.isGlobal);
            if (p->opr.op[0]->id.isGlobal)// for global
            {
              printf("\tpop\tsb[%d]\n", index);
            }
            else{ // for local
              index = index-100;
              if (index >= argTable[funcIdx]) // for parameter
              printf("\tpop\tfp[%d]\n",  index - argTable[funcIdx]);
              else // for local variable
              printf("\tpop\tfp[%d]\n",  -3 - argTable[funcIdx] + index);
            }
          } else if (p->opr.op[0]->type == typeOpr && p->opr.op[0]->opr.oper == '@') { /* @ for global variable */
            nodeType* tmp = p->opr.op[0]->opr.op[0];
            int index = getSYMIdx(tmp->id.var_name, tmp->id.isGlobal);
            int thisT = checkExprType(p->opr.op[1]);
            setType(index, thisT);

            if (tmp->id.isGlobal)// for global
            printf("\tpop\tsb[%d]\n", index);
          }

        }
        else if(p->opr.nops == 3) // Array element assignment: vari [exper] = expr
        {
          char * array_name = p->opr.op[0]->id.var_name;
          int is_global = p->opr.op[0]->id.isGlobal;
          int index = getSYMIdx(array_name, is_global);
          if(index == -1)
          {
            printf("\tError: Array %s used before declared!\n", array_name);
          }

          // evaluate the expression to get the value to be assigned
          ex(p->opr.op[2], blbl, clbl, infunc);

          // evaluate the index
          ex(p->opr.op[1], blbl, clbl, infunc);

          //TODO check for array index outofbound

          // push the index offset on the stack
          if(is_global)
          {
            printf("\tpush\t%d\n", index);
          }
          else
          {
            printf("\tpush\t%d\n", index - 100);
          }

          // add up the index and the offset and store it in "in" register
          printf("\tadd\n");
          printf("\tpop\tin\n");

          if(is_global)
          {
            printf("\tpop\tsb[in]\n");
          }
          else
          {
            printf("\tpop\tfp[in]\n");
          }
        }
        else if(p->opr.nops == 4) // 2D Array element assignment: vari [exper][expr] = expr
        {
          char * array_name = p->opr.op[0]->id.var_name;
          int is_global = p->opr.op[0]->id.isGlobal;
          int index = getSYMIdx(array_name, is_global);

          if(index == -1)
          {
            printf("\tError: Array %s used before declared!\n", array_name);
          }

          // evaluate the expression to get the value to be assigned
          ex(p->opr.op[3], blbl, clbl, infunc);

          // evaluate the index
          ex(p->opr.op[2], blbl, clbl, infunc);
          ex(p->opr.op[1], blbl, clbl, infunc);

          twoDArray* array = getTwoDArray(array_name);
          int scd_size = array->scd_size;
          int fst_size = array->fst_size;

          //TODO check for index out of bound

          printf("\tpush\t%d\n", scd_size);
          printf("\tmul\n");
          printf("\tadd\n");

          // push the index offset on the stack
          if(is_global)
          {
            printf("\tpush\t%d\n", index);
          }
          else
          {
            printf("\tpush\t%d\n", index - 100);
          }

          // add up the index and the offset and store it in "in" register
          printf("\tadd\n");
          printf("\tpop\tin\n");

          if(is_global)
          {
            printf("\tpop\tsb[in]\n");
          }
          else
          {
            printf("\tpop\tfp[in]\n");
          }
        }
        else if(p->opr.nops == 5) // 3D Array element assignment: vari [exper][expr][expr] = expr
        {
          char * array_name = p->opr.op[0]->id.var_name;
          int is_global = p->opr.op[0]->id.isGlobal;
          int index = getSYMIdx(array_name, is_global);

          if(index == -1)
          {
            printf("\tError: Array %s used before declared!\n", array_name);
          }

          // evaluate the expression to get the value to be assigned
          ex(p->opr.op[4], blbl, clbl, infunc);

          // evaluate the index
          ex(p->opr.op[3], blbl, clbl, infunc);
          ex(p->opr.op[2], blbl, clbl, infunc);
          ex(p->opr.op[1], blbl, clbl, infunc);

          threeDArray* array = getThreeDArray(array_name);
          int fst_size = array->fst_size;
          int scd_size = array->scd_size;
          int thd_size = array->thd_size;
          //TODO check for index out of bound

          printf("\tpush\t%d\n", scd_size);
          printf("\tmul\n");
          printf("\tadd\n");

          printf("\tpush\t%d\n", thd_size);
          printf("\tmul\n");
          printf("\tadd\n");

          // push the index offset on the stack
          if(is_global)
          {
            printf("\tpush\t%d\n", index);
          }
          else
          {
            printf("\tpush\t%d\n", index - 100);
          }

          // add up the index and the offset and store it in "in" register
          printf("\tadd\n");
          printf("\tpop\tin\n");

          if(is_global)
          {
            printf("\tpop\tsb[in]\n");
          }
          else
          {
            printf("\tpop\tfp[in]\n");
          }
        }
        else
        {
          yyerror("Error assignment operation: wrong number of operands\n");
        }
        break;
      }

      case UMINUS:
      {
        ex(p->opr.op[0], blbl, clbl, infunc);
        printf("\tneg\n");
        break;
      }
      case PUTI:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0],blbl, clbl, infunc);
        printf("\tputi\n");
        break;
      }
      case PUTI_:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0],blbl, clbl, infunc);
        printf("\tputi_\n");

        break;
      }
      case PUTC:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0],blbl,clbl, infunc);
        printf("\tputc\n");
        break;
      }
      case PUTC_:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0],blbl,clbl, infunc);
        printf("\tputc_\n");

        break;
      }
      case PUTS:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0],blbl,clbl, infunc);
        printf("\tputs\n");
        break;
      }
      case PUTS_:
      {
        int typeP = checkExprType(p->opr.op[0]);
        ex(p->opr.op[0],blbl,clbl, infunc);
        printf("\tputs_\n");
        break;
      }
      case GETI:
      {
        nodeType* tmp;
        if(p->opr.op[0]->type == typeId) {
          tmp = p->opr.op[0];
        } else if (p->opr.op[0]->type == typeOpr){
          tmp = p->opr.op[0]->opr.op[0];
        } else {
          yyerror("error.\n");
        }
        int index = getSYMIdx(tmp->id.var_name, tmp->id.isGlobal);
        setType(index, 1);
        printf("\tgeti\n");
        if (tmp->id.isGlobal)
        {
          printf("\tpop\tsb[%d]\n", getSYMIdx(tmp->id.var_name, tmp->id.isGlobal));
        }
        else
        {
          index = index - 100;
          if (index >= argTable[funcIdx]) // for parameter ? local variable????
          {
            printf("\tpop\tfp[%d]\n",  index - argTable[funcIdx]);
          }
          else // for local variable ? parameter???
          {
            printf("\tpop\tfp[%d]\n",  -3 - argTable[funcIdx] + index); //3 is accounted for the saved caller's pc, fp, sp
          }
        }
        break;
      }
      case GETC:
      {
        nodeType* tmp;
        if(p->opr.op[0]->type == typeId) {
          tmp = p->opr.op[0];
        } else if (p->opr.op[0]->type == typeOpr){
          tmp = p->opr.op[0]->opr.op[0];
        } else {
          yyerror("error.\n");
        }
        int index = getSYMIdx(tmp->id.var_name, tmp->id.isGlobal);
        setType(index, 3);
        printf("\tgetc\n");
        if (tmp->id.isGlobal)
        {
          printf("\tpop\tsb[%d]\n", getSYMIdx(tmp->id.var_name, tmp->id.isGlobal));
        }
        else
        {
          index = index - 100;
          if (index >= argTable[funcIdx]) // for parameter ? local variable????
          {
            printf("\tpop\tfp[%d]\n",  index - argTable[funcIdx]);
          }
          else // for local variable ? parameter???
          {
            printf("\tpop\tfp[%d]\n",  -3 - argTable[funcIdx] + index); //3 is accounted for the saved caller's pc, fp, sp
          }
        }
        break;
      }
      case GETS:
      {
        nodeType* tmp;
        if(p->opr.op[0]->type == typeId) {
          tmp = p->opr.op[0];
        } else if (p->opr.op[0]->type == typeOpr){
          tmp = p->opr.op[0]->opr.op[0];
        } else {
          yyerror("error.\n");
        }
        int index = getSYMIdx(tmp->id.var_name, tmp->id.isGlobal);
        setType(index, 2);
        printf("\tgets\n");
        if (tmp->id.isGlobal)
        {
          printf("\tpop\tsb[%d]\n", getSYMIdx(tmp->id.var_name, tmp->id.isGlobal));
        }
        else
        {
          index = index - 100;
          if (index >= argTable[funcIdx]) // for parameter ? local variable????
          {
            printf("\tpop\tfp[%d]\n",  index - argTable[funcIdx]);
          }
          else // for local variable ? parameter???
          {
            printf("\tpop\tfp[%d]\n",  -3 - argTable[funcIdx] + index); //3 is accounted for the saved caller's pc, fp, sp
          }
        }
        break;
      }
      default:
      ex(p->opr.op[0], blbl, clbl, infunc);
      ex(p->opr.op[1], blbl, clbl, infunc);
      switch(p->opr.oper) {
        case '+':   printf("\tadd\n"); break;
        case '-':   printf("\tsub\n"); break;
        case '*':   printf("\tmul\n"); break;
        case '/':   printf("\tdiv\n"); break;
        case '%':   printf("\tmod\n"); break;
        case '<':   printf("\tcompLT\n"); break;
        case '>':   printf("\tcompGT\n"); break;
        case GE:    printf("\tcompGE\n"); break;
        case LE:    printf("\tcompLE\n"); break;
        case NE:    printf("\tcompNE\n"); break;
        case EQ:    printf("\tcompEQ\n"); break;
        case AND:   printf("\tand\n"); break;
        case OR:    printf("\tor\n"); break;
      }
    }
  }

  return 0;
}

void eop() {
  // add end of program label
  printf("\tjmp\tL999\n");
  printf("L998:\n");
  printf("\tpush\t999999\n");
  printf("\tputi\n");
  printf("L999:\n");
}
