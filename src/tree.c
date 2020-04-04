/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */


/*
gcc -O2 -Wall -Wextra -I./ -o tree tree.c util.c

*/

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include "tree.h"
#include "util.h"

inline void StackInit(stackT *stackP, int maxSize)
{
  stackElementT *newContents;

  /* Allocate a new array to hold the contents. */

  newContents = (stackElementT *)SAFEMALLOC(sizeof(stackElementT) * maxSize);

  if (newContents == NULL) {
    fprintf(stderr, "Insufficient memory to initialize stack.\n");
    exit(1);  /* Exit, returning error code. */
  }

  stackP->contents = newContents;
  stackP->maxSize = maxSize;
  stackP->top = 0;  /* I.e., empty */
  stackP->max_top_reached = 0;
}

inline void StackDestroy(stackT *stackP)
{
  /* Get rid of array. */
  safe_free(stackP->contents);

  stackP->contents = NULL;
  stackP->maxSize = 0;
  stackP->top = 0;  /* I.e., empty */
  stackP->max_top_reached = 0;
}

inline int StackIsEmpty(stackT *stackP)
{
  return stackP->top == 0;
}

inline int StackIsFull(stackT *stackP)
{
  return stackP->top >= stackP->maxSize;
}

//TODO - ERROR handling other than exit
inline void StackPush(stackT *stackP, stackElementT element)
{
  if (StackIsFull(stackP)) {
    fprintf(stderr, "Can't push element on stack: stack is full.\n");
    exit(1);  /* Exit, returning error code. */
  }

  /* Put information in array; update top. */

  stackP->contents[stackP->top] = element;
  ++stackP->top;
  if (stackP->top > stackP->max_top_reached) stackP->max_top_reached = stackP->top;
}

inline stackElementT StackPop(stackT *stackP)
{
  if (StackIsEmpty(stackP)) {
    fprintf(stderr, "Can't pop element from stack: stack is empty.\n");
    exit(1);  /* Exit, returning error code. */
  }

  return stackP->contents[--stackP->top];
}


sturm_word_t* sturm_word_new(letter first, letter** const map, const size_t* const length, size_t map_size, size_t max_length) {
  size_t l,s;
  size_t i,j,k;
  size_t max[2] = { 0 };
  size_t level = 0;  //How many levels. Value bigger than 1 means that expansion was used
  size_t *length_after_expand[3];
  length_after_expand[0] = (size_t *) SAFECALLOC (map_size, sizeof(size_t*) ); //Length in the last step
  length_after_expand[1] = (size_t *) SAFECALLOC (map_size, sizeof(size_t*) ); //Length in the current step
  //length_after_expand[2] is used for the swap


  //fprintf(stderr, "sturm_word_new max_length = %zu\n", max_length);
  sturm_word_t* sturm_word= (sturm_word_t*) SAFEMALLOC ( sizeof(sturm_word_t) );

  sturm_word->n = 0;
  sturm_word->a = first;
  sturm_word->first = first;
  sturm_word->i = 1;   //Since prefix is the same all the time we will take the index #2 ( i==0 means first index)
  sturm_word->map_size = map_size;
  StackInit(&sturm_word->stack, 1024);
#ifndef NDEBUG
  sturm_word->control_sum = 0;
#endif
  //Do we need to expand the rule?
 
  //Iterate over all letters of alphabet
  for (l=0; l<map_size; ++l) {
    //fprintf(stderr, "length[%zu] = %zu\n", l, length[l]);
    if ( max[0] < length[l]) max[0] = length[l];
  }

#if 0
  for (l=0; l<map_size; ++l) {
    fprintf(stderr, "%zu - > ", l);
    for (s=0; s<length[l];++s) {
      fprintf(stderr, "%u,", map[l][s]);
    } 
    fprintf(stderr, "\n");
  }
#endif

  if ( max_length > max[0] ) {

    //fprintf(stderr, "sturm_word_new: Trying to expand the rule from %zu to %zu.\n", max[0], max_length);
    //We will try to expand the rules
    //Step 1 => derive matrix describing length
    //M00 = From '0', how many new '0'
    //M01 = From '1', how many new '0'
    size_t **M[4];
    M[0] = (size_t **) SAFECALLOC (map_size, sizeof(size_t*) ); //Matrix itself
    M[1] = (size_t **) SAFECALLOC (map_size, sizeof(size_t*) ); //M^(n-1)
    M[2] = (size_t **) SAFECALLOC (map_size, sizeof(size_t*) ); //M^(n)
                                                            //M[3] used for the swap => no memory allocated
    for (s=0; s<map_size; ++s) {
      M[0][s] = SAFECALLOC (map_size, sizeof(size_t) );
      M[1][s] = SAFECALLOC (map_size, sizeof(size_t) );
      M[2][s] = SAFECALLOC (map_size, sizeof(size_t) );
    }


    for (l=0; l<map_size; ++l) {
      length_after_expand[0][l] = length[l];
    }

    //Iterate over letter of alphabet
    for (l=0; l<map_size; ++l) {
      for (s=0; s<length[l];++s) {
        M[0][map[l][s]][l]++;
        M[1][map[l][s]][l]++;
      } 
    }

#if 0
    for (l=0; l<map_size; ++l) {
      for (s=0; s<map_size;++s) {
        fprintf(stderr, "%zu", M[0][l][s]);

      }
     fprintf(stderr, "\n"); 
    }
#endif

    do {
      //Step 2 => compute M[2] = M[1] * M[0]. It describes length at the next level
      //Sum of elements in l-th column of M => length of sturm word started from letter l at the given level

      for (i = 0; i < map_size; i++)
        for (j = 0; j < map_size; j++) {
          M[2][i][j] = 0;
          for (k = 0; k < map_size; k++)
            M[2][i][j] += M[1][i][k]*M[0][k][j];
        }

#if 0
      for (l=0; l<map_size; ++l) {
        for (s=0; s<map_size;++s) {
          fprintf(stderr, "%zu", M[2][l][s]);

        }
        fprintf(stderr, "\n"); 
      }
#endif

      M[3] = M[1];
      M[1] = M[2];
      M[2] = M[3];

      //length[l] = sum of elements in column of M[1]

      max[1] = 0;
      for (l=0; l<map_size; ++l) {
        length_after_expand[1][l] = 0;
        for (s=0; s<map_size; ++s) {
          length_after_expand[1][l] += M[1][s][l];
        }
        //fprintf(stderr, "Level %zu, length[%zu] = %zu\n", level + 1, l, length_after_expand[1][l]);
        if ( length_after_expand[1][l] > max[1] ) max[1] = length_after_expand[1][l];
      }

      if ( max_length >= max[1] ) {
        length_after_expand[2] = length_after_expand[0];
        length_after_expand[0] = length_after_expand[1];
        length_after_expand[1] = length_after_expand[2];
        max[0] = max[1];
        ++level;
      } 
    } while ( max_length >= max[1] );

    for (s=0; s<map_size; ++s) {
      safe_free(M[0][s]);
      safe_free(M[1][s]);
      safe_free(M[2][s]);
    }
    safe_free(M[0]);
    safe_free(M[1]);
    safe_free(M[2]);

  }

  sturm_word->map = (letter**) SAFEMALLOC(map_size * sizeof(letter*) );
  sturm_word->length = (size_t*) SAFEMALLOC(map_size * sizeof(size_t));

  if ( level > 0 ) {
    //Expansion was successful - we need to generate new map matrix
    //fprintf(stderr, "sturm_word_new max_length Expanding to level %zu\n", level);
    letter* temp[3];
    size_t cur_length[2];
    temp[0] = SAFECALLOC(max[0], sizeof(letter));
    temp[1] = SAFECALLOC(max[0], sizeof(letter));
    //temp[3] will be used to swap the pointers

    for(s=0; s<map_size; ++s) {
      sturm_word->map[s] = (letter*) SAFECALLOC( length_after_expand[0][s], sizeof(letter));
      //Now we need to derive the rule s -> {}
      memset(temp[0], 0, max[0]);
      memset(temp[1], 0, max[0]);
      memcpy(temp[0], map[s], length[s] * sizeof(letter));
      cur_length[0] = length[s];
      cur_length[1] = 0;
      for (l=0;l<level;++l) {
        for (i=0;i<cur_length[0];++i) {
          memcpy(temp[1]+cur_length[1], map[temp[0][i]], length[temp[0][i]] * sizeof(letter));
          cur_length[1] += length[temp[0][i]];
        }
        temp[2] = temp[0];
        temp[0] = temp[1];
        temp[1] = temp[2];
        cur_length[0] = cur_length[1];
        memset(temp[1], 0, max[0]);
        cur_length[1] = 0;
#if 0
        fprintf(stderr, "Level %zu\n",l);
        fprintf(stderr, "%zu - > ", s);
        for (i=0;i<cur_length[0];++i) {
          fprintf(stderr, "%d,", temp[0][i]);
        }
        fprintf(stderr, "\n");
#endif      
      }
      //fprintf(stderr, "\nlength_after_expand[0][s]=%zu, cur_length[0]=%zu\n", length_after_expand[0][s], cur_length[0]);
      assert(length_after_expand[0][s] == cur_length[0] );
      memcpy( sturm_word->map[s], temp[0],cur_length[0] * sizeof(letter));
      sturm_word->length[s] = cur_length[0];
    }
    safe_free(temp[0]);
    safe_free(temp[1]);
  } else {
    //Deep copy 2D map array
    for(s=0; s<map_size; ++s) {
      sturm_word->map[s] = (letter*) SAFEMALLOC( length[s] * sizeof(letter));
      memcpy( sturm_word->map[s], map[s],length[s] * sizeof(letter));
    }
    //Deep copy length array
    memcpy( sturm_word->length, length,map_size * sizeof(size_t));
  }

#if 0
  for(i=0;i<map_size;++i) {
    fprintf(stderr, "%zu -> ", i);
    for(j=0;j<sturm_word->length[i];++j) {
      fprintf(stderr, "%d,", sturm_word->map[i][j]);
    }
    fprintf(stderr, "\n");
  }
#endif  

  safe_free(length_after_expand[0]);
  safe_free(length_after_expand[1]);

  return sturm_word;
}

void sturm_word_delete(sturm_word_t* data) {
  StackDestroy(&data->stack);

  size_t s;
  for(s=0; s < data->map_size; ++s) {
    safe_free(data->map[s]);
  }
  safe_free(data->map);
  
  safe_free(data->length);
  safe_free(data);
 
}

size_t sturm_word_get_current_size(sturm_word_t* data) {
  return sizeof(stackElementT) * data->stack.top;
}

size_t sturm_word_get_max_size(sturm_word_t* data) {
  return sizeof(stackElementT) * data->stack.max_top_reached;
}


uint64_t traverse(uint64_t elements, letter* buf, sturm_word_t* data) {

  uint64_t generated = 0;
  stackElementT stack_data;
  uint32_t level;

  if ( elements == 0 ) return 0;

  if (data->n == 0) {
    buf[generated++] = data->first;
  }

  while(generated<elements) {

    //Traverse all childs of the current element
#if 0
    while ( data->i < data->length[data->a] && generated<elements ) {
      //fprintf(stderr, "\n map[%d, %d] = %d", data->a, data->i, data->map[data->a][data->i]);
      buf[generated++] = data->map[data->a][data->i];
      ++data->i;
    }
#else
    size_t n = ( (elements - generated) > (data->length[data->a] - data->i) ) ? data->length[data->a] - data->i : elements - generated;
    memcpy(&buf[generated], &data->map[data->a][data->i], n * sizeof(letter));
    generated += n;
    data->i   += n;
#endif    

    //We need to go up in the tree looking for the first element with untraversed childs
    level = 0;
    while ( (! StackIsEmpty(&data->stack) ) && (data->i == data->length[data->a] ) ) {
      stack_data = StackPop(&data->stack);
      data->i = stack_data.i;
      ++data->i;
      data->a = stack_data.a;
      ++level;
    }

   //If stack is empty and there are no childs left then we have reached the root of the tree
   //We need to go one level down
   //Thanks to the fix prefix we will start traverse at index = 2 (implies i = 1)
   if (  StackIsEmpty(&data->stack) && (data->i == data->length[data->a] ) ) {
     data->a = data->first;
     data->i = 1;
     ++level;
     //fprintf(stderr,"\nStack empty at %"PRIu64" \n", data->n + (uint64_t) generated);
   }

   //We need to go down in the tree until we reach the needed level
   while ( level > 0 ) {
      stack_data.a = data->a;
      stack_data.i = data->i;
      StackPush (&data->stack, stack_data);
      data->a = data->map[data->a][data->i];
      data->i=0;
      --level;
   }
  }
  assert(generated == elements);
  data->n += generated;

#ifndef NDEBUG
  uint64_t i;
  for(i=0; i<generated; ++i) {
    data->control_sum += buf[i];
  }
#endif

  return generated;
}
