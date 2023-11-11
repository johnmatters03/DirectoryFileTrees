#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dynarray.h"
#include "nodeFT.h"

struct node {
    Path_T oPPath;
    Node_T oNParent;
    DynArray_T oDChildren;
    boolean isFile;
    void *pvContent;
};

/*
  Links new child oNChild into oNParent's children array at index
  ulIndex. Returns SUCCESS if the new child was added successfully,
  or  MEMORY_ERROR if allocation fails adding oNChild to the array.
*/
static int Node_addChild(Node_T oNParent, Node_T oNChild,
                         size_t ulIndex) {
   assert(oNParent != NULL);
   assert(oNChild != NULL);
   assert(!(oNParent->isFile));

   if(DynArray_addAt(oNParent->oDChildren, ulIndex, oNChild))
      return SUCCESS;
   else
      return MEMORY_ERROR;
}

/*
  Compares the string representation of oNfirst with a string
  pcSecond representing a node's path.
  Returns <0, 0, or >0 if oNFirst is "less than", "equal to", or
  "greater than" pcSecond, respectively.
*/
static int Node_compareString(const Node_T oNFirst,
                                 const char *pcSecond) {
   assert(oNFirst != NULL);
   assert(pcSecond != NULL);

   return Path_compareString(oNFirst->oPPath, pcSecond);
}

int Node_newChild(Path_T oPPath, Node_T oNParent, Node_T *poNResult)
{
    struct node *psNew;
    Path_T oPParentPath = NULL;
    Path_T oPNewPath = NULL;
    size_t ulParentDepth;
    size_t ulIndex;
    int iStatus;
    

    assert(oPPath != NULL);
    assert(oNParent == NULL || CheckerFT_Node_isValid(oNParent));
    assert(!(oNParent->isFile));

    psNew = malloc(sizeof(struct node));
    if (psNew == NULL) {
        *poNResult == NULL;
        return MEMORY_ERROR;
    }

    psNew->isFile = FALSE;
    psNew->pvContent = NULL;

    iStatus = Path_dup(oPPath, &oPNewPath);
    if (iStatus != SUCCESS) {
        free(psNew);
        *poNResult = NULL;
        return iStatus;
    }
    psNew->oPPath = oPNewPath;

    if (oNParent != NULL) {
        size_t ulSharedDepth;

        oPParentPath = oNParent->oPPath;
        ulParentDepth = Path_getDepth(oPParentPath);
        ulSharedDepth = Path_getSharedPrefixDepth(psNew->oPPath,
                                                oPParentPath);
        if (ulSharedDepth < ulParentDepth) {
            Path_free(psNew->oPPath);
            free(psNew);
            *poNResult = NULL;
            return CONFLICTING_PATH;
        }

        /* parent must be exactly one level up from child */
        if(Path_getDepth(psNew->oPPath) != ulParentDepth + 1) {
            Path_free(psNew->oPPath);
            free(psNew);
            *poNResult = NULL;
            return NO_SUCH_PATH;
        }

        if(Node_hasChild(oNParent, oPPath, &ulIndex)) {
            Path_free(psNew->oPPath);
            free(psNew);
            *poNResult = NULL;
            return ALREADY_IN_TREE;
        }
    }
    else {
      /* new node must be root */
      /* can only create one "level" at a time */
        if(Path_getDepth(psNew->oPPath) != 1) {
            Path_free(psNew->oPPath);
            free(psNew);
            *poNResult = NULL;
            return NO_SUCH_PATH;
        }
    }
    psNew->oNParent = oNParent;

    psNew->oDChildren = DynArray_new(0);
    if(psNew->oDChildren == NULL) {
        Path_free(psNew->oPPath);
        free(psNew);
        *poNResult = NULL;
        return MEMORY_ERROR;
    }

    /* Link into parent's children list */
    if(oNParent != NULL) {
        iStatus = Node_addChild(oNParent, psNew, ulIndex);
        if(iStatus != SUCCESS) {
            Path_free(psNew->oPPath);
            free(psNew);
            *poNResult = NULL;
            return iStatus;
        }
    }
    *poNResult = psNew;

    assert(oNParent == NULL || CheckerDT_Node_isValid(oNParent));
    assert(CheckerDT_Node_isValid(*poNResult));

    return SUCCESS;
}

int Node_newFile(Path_T oPPath, Node_T oNParent, Node_T *poNResult, 
                void *pvContent) {
    int iStatus;

    iStatus = Node_newNode(oPPath, oNParent, poNResult);

    if (iStatus != SUCCESS) {
        return iStatus;
    }

    (*poNResult)->isFile = TRUE;
    (*poNResult)->pvContent = pvContent;
}

size_t Node_free(Node_T oNNode) {
    size_t ulIndex;
    size_t ulCount = 0;

    assert(oNNode != NULL);
    assert(CheckerDT_Node_isValid(oNNode));

    /* remove from parent's list */
    if(oNNode->oNParent != NULL) {
      if(DynArray_bsearch(
            oNNode->oNParent->oDChildren,
            oNNode, &ulIndex,
            (int (*)(const void *, const void *)) Node_compare)
        )
         (void) DynArray_removeAt(oNNode->oNParent->oDChildren,
                                  ulIndex);
    }

    if (!oNNode->isFile) {
        /* recursively remove children */
        while(DynArray_getLength(oNNode->oDChildren) != 0) {
            ulCount += Node_free(DynArray_get(oNNode->oDChildren, 0));
        }
    }
    DynArray_free(oNNode->oDChildren);

    /* remove path */
    Path_free(oNNode->oPPath);

    /* finally, free the struct node */
    free(oNNode);
    ulCount++;
    return ulCount;
}

Path_T Node_getPath(Node_T oNNode) {
    assert(oNNode != NULL);

    return oNNode->oPPath;
}

boolean Node_hasChild(Node_T oNParent, Path_T oPPath,
                         size_t *pulChildID) {
   assert(oNParent != NULL);
   assert(oPPath != NULL);
   assert(pulChildID != NULL);
   assert(!oNParent->isFile);

   /* *pulChildID is the index into oNParent->oDChildren */
   return DynArray_bsearch(oNParent->oDChildren,
            (char*) Path_getPathname(oPPath), pulChildID,
            (int (*)(const void*,const void*)) Node_compareString);
}

size_t Node_getNumChildren(Node_T oNParent) {
   assert(oNParent != NULL);
   assert(!oNParent->isFile);

   return DynArray_getLength(oNParent->oDChildren);
}

int  Node_getChild(Node_T oNParent, size_t ulChildID,
                   Node_T *poNResult) {

   assert(oNParent != NULL);
   assert(poNResult != NULL);
    assert(!oNParent->isFile);

   /* ulChildID is the index into oNParent->oDChildren */
   if(ulChildID >= Node_getNumChildren(oNParent)) {
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }
   else {
      *poNResult = DynArray_get(oNParent->oDChildren, ulChildID);
      return SUCCESS;
   }
}

Node_T Node_getParent(Node_T oNNode) {
   assert(oNNode != NULL);

   return oNNode->oNParent;
}

void *Node_getCont(Node_T oNNode) {
    assert(oNNode != NULL);
    assert(oNNode->isFile);

    return oNNode->pvContent;
}

boolean Node_isFile(Node_T oNNode) {
    assert(oNNode != NULL);

    return oNNode->isFile;
}


int Node_compare(Node_T oNFirst, Node_T oNSecond) {
   assert(oNFirst != NULL);
   assert(oNSecond != NULL);

   assert(oNFirst->isFile == oNSecond->isFile);

   return Path_comparePath(oNFirst->oPPath, oNSecond->oPPath);
}

char *Node_toString(Node_T oNNode) {
   char *copyPath;

   assert(oNNode != NULL);

   copyPath = malloc(Path_getStrLength(Node_getPath(oNNode))+1);
   if(copyPath == NULL)
      return NULL;
   else
      return strcpy(copyPath, Path_getPathname(Node_getPath(oNNode)));
}