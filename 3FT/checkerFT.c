/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "checkerFT.h"
#include "dynarray.h"
#include "path.h"

/* see checkerFT.h for specification */
boolean CheckerFT_Node_isValid(Node_T oNNode) {
   Node_T oNParent;
   Path_T oPNPath;
   Path_T oPPPath;

   /* Sample check: a NULL pointer is not a valid node */
   if(oNNode == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   if (Node_isFile(oNNode)) {
      if (Node_getNumChildren(oNNode) != 0) {
         fprintf(stderr, "Number of children of a file must be zero\n");
         return FALSE;
      }
   }

   /* Sample check: parent's path must be the longest possible
      proper prefix of the node's path */
   oNParent = Node_getParent(oNNode);
   if(oNParent != NULL) {
      oPNPath = Node_getPath(oNNode);
      oPPPath = Node_getPath(oNParent);

      if(Path_getSharedPrefixDepth(oPNPath, oPPPath) !=
         Path_getDepth(oPNPath) - 1) {
         fprintf(stderr, "P-C nodes don't have P-C paths: (%s) (%s)\n",
                 Path_getPathname(oPPPath), Path_getPathname(oPNPath));
         return FALSE;
      }
   }

   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at oNNode.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.
*/
static boolean CheckerFT_treeCheck(Node_T oNNode, int *ulCount) {
   size_t ulIndex;
   size_t j;
   int cmp;

   if(oNNode!= NULL) {

      /* Sample check on each node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerFT_Node_isValid(oNNode))
         return FALSE;
      
      /* Check node count */
      if (*ulCount == 0)
      {
         fprintf(stderr, 
         "ulCount provides incorrect count of the number of nodes in DT\n");
         return FALSE;
      }
      *ulCount = *ulCount - 1;

      /* Recur on every child of oNNode */
      for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++)
      {
         Node_T oNChild = NULL;
         Node_getChild(oNNode, ulIndex, &oNChild);

         for (j = ulIndex + 1; j < Node_getNumChildren(oNNode); j++)
         {
            Node_T jChild = NULL;
            int jStatus = Node_getChild(oNNode, j, &jChild);
            if (jStatus != SUCCESS)
            {
               fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
               return FALSE;
            }

            cmp = Path_comparePath(Node_getPath(oNChild), Node_getPath(jChild));

            if (!cmp)
            {
               fprintf(stderr, "Tree contains duplicate paths\n");
               return FALSE;
            }
            else if (cmp > 0)
            {
               fprintf(stderr, "Tree nodes not in lexicographical order\n");
               return FALSE;
            }

            
         }

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!CheckerFT_treeCheck(oNChild, ulCount))
            return FALSE;
      }
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerFT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount) {

   int *count;
   boolean status;

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!bIsInitialized) {
      if(ulCount != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }
   } else {
      if (oNRoot != NULL && Node_isFile(oNRoot)) {
         fprintf(stderr, "Root node cannot be a file\n");
         return FALSE;
      }
   }
   count = malloc(sizeof(size_t));

   if (count == NULL)
   {
      fprintf(stderr, "Memory allocation failed\n");
      return FALSE;
   }


   *count = (int) ulCount;

   /* Now checks invariants recursively at each node from the root. */
   status = CheckerFT_treeCheck(oNRoot, count);

   free(count);
   return status;
}