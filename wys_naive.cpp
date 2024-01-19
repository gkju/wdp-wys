#include "wys.h"

int n, k, g;

bool smaller(int x) {
  for (int i = 0; i < k; ++i)
    mniejszaNiz(x);
  return mniejszaNiz(x);
}

void play(int lo, int hi) {
  if (lo == hi) {
    odpowiedz(lo);
    return;
  }
  int mid = (lo + hi + 1) / 2;
  if (smaller(mid))
    play(lo, mid - 1);
  else
    play(mid, hi);
}

int main() {
  dajParametry(n, k, g);
  while (g--) {
    play(1, n);
  }
}
