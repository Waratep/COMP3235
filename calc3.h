typedef enum { typeCon, typeStrCon, typeCharCon, typeId, typeOpr, typeFunc } nodeEnum;

/* constants */
typedef struct {
    int value;                  /* value of constant */
} conNodeType;

/* character constants */
typedef struct {
    char *value;                 /* value of string constant */
} charConNodeType;

/* string constants */
typedef struct {
    char *value;                /* value of string constant */
} strConNodeType;

/* identifiers */
typedef struct {
    char *var_name;             /* subscript to sym array */
    int isGlobal;
} idNodeType;

/* operators */
typedef struct {
    int oper;                   /* operator */
    int nops;                   /* number of operands */
    struct nodeTypeTag *op[1];  /* operands (expandable) */
} oprNodeType;

/* function */
typedef struct {
    char* name; 	      	/* name of the function */
    int argc;			/* count of number of arguments */
    struct nodeTypeTag *args;  	/* function arguments */ 
    struct nodeTypeTag *op;    	/* statements in the function */
} funcNodeType;

typedef struct nodeTypeTag {
    nodeEnum type;              /* type of node */

    /* union must be last entry in nodeType */
    /* because operNodeType may dynamically increase */
    union {
        conNodeType con;        /* constants */
	charConNodeType charCon; /* character constants */
	strConNodeType strCon;	/* string constants */
        idNodeType id;          /* identifiers */
        oprNodeType opr;        /* operators */
	funcNodeType func;	/* functions */
    };
} nodeType;

extern char* sym[200];

#define MAIN 997
#define CALL 996