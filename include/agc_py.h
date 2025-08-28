/*   agc_py.h   */

typedef struct agcInfo {
  int window;        /*   Agc window in points   */
  int dwind;         /*   Detection window       */
  gboolean detect;        /*   Boolean for detection  */
  float threshold;   /*   Detection threshold    */
} AGCINFO ;

typedef struct filteredMem {
  void *filteredPointer;   /*   Pointer to filtered trace   */
  int n;                   /*   Number of unfreed traces    */
} FILTEREDMEM ;
