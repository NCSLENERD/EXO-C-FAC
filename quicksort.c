#include <stdlib.h>

typedef unsigned int uint;

typedef struct {
  uint len;
  int* val;
} Tab;

void swap (Tab t, uint i, uint j) {
  int tmp = t.val[i];
  t.val[i] = t.val[j];
  t.val[j] = tmp;
}

Tab slice (Tab t, uint first, uint last) {
  Tab s;
  s.len = last - first + 1;
  s.val = t.val + first;
  return s;
}

uint partition (Tab t) {
  int pivot = t.val[(t.len-1)/2];
  uint a = -1;
  uint b = t.len;
  while (1) {
    do { a++; } while (t.val[a] < pivot);
    do { b--; } while (t.val[b] > pivot);
    if (a >= b) { return b; }
    swap(t, a, b);
  }
}

void sort (Tab t) {
  uint p;
  if (t.len > 1) {
    p = partition(t);
    sort(slice(t, 0, p));
    sort(slice(t, p+1, t.len-1));
  }
}
