#include <iostream>
#include <algorithm>
#include <bitset>
#include "wys.h"

namespace {
int _n = 12, _k = 3, _g = 100000;
int _x;
int ileGier, ileZapytan, maxZapytan;
int ile_klamalem = 0;
}

std::bitset<20> which_chosen = 0;

void dajParametry(int &n, int &k, int &g) {
  n = _n; k = _k; g = _g;
  _x = rand() % n + 1;
  which_chosen[_x] = 1;
  ileGier = ileZapytan = maxZapytan = 0;
  std::cout << "n = " << _n << ", k = " << _k << ", g = " << _g << '\n';
}

bool mniejszaNiz(int y) {
  ++ileZapytan;
  if (ile_klamalem < _k && rand() % _n == 1) {
    ++ile_klamalem;
    return !(_x < y);
  } else {
    return _x < y;
  }
}


void odpowiedz(int x) {
  if (x != _x) {
    std::cout << "ZLE. Bledna odpowiedz w grze #" << ileGier << ": oczekiwano " << _x << " a uzyskano odpowiedz " << x << '\n';
    exit(1);
  }
  //std::cout << "for " << _x << " in " << ileZapytan << " queries\n";
  maxZapytan = std::max(maxZapytan, ileZapytan);
  ++ileGier;
  if (ileGier == _g) {
    std::cout << "OK. Zadano maksymalnie " << maxZapytan << " zapytan.\n";
    std::cout << "PYTANO O " << which_chosen << " \n";
    exit(0);
  }
  _x = rand() % _n + 1;
  which_chosen[_x] = 1;
  ile_klamalem = 0;
  ileZapytan = 0;
}
