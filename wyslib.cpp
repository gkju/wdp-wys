#include <iostream>
#include <algorithm>
#include <bitset>
#include <chrono>
#include "wys.h"

namespace {
int _n = 12, _k = 3, _g = 10000000;
int _x;
int ileGier, ileZapytan, maxZapytan, maxKlamstw, ileMaxKlamstw;
int ile_klamalem = 0;
}

std::bitset<20> which_chosen = 0;

void dajParametry(int &n, int &k, int &g) {
  std::cin >> n >> k >> g;
  _n = n;
  _k = k;
  _g = g;
  _x = rand() % n + 1;
  which_chosen[_x] = 1;
  ileGier = ileZapytan = maxZapytan = maxKlamstw = ileMaxKlamstw = 0;
  //std::cout << "n = " << _n << ", k = " << _k << ", g = " << _g << '\n';
}

bool mniejszaNiz(int y) {
  ++ileZapytan;
  if (ile_klamalem < _k && (rand() % (_n + 1)) < _k) {
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
  maxKlamstw = std::max(maxKlamstw, ile_klamalem);
  if (ile_klamalem == _k) {
    ++ileMaxKlamstw;
  }
  ++ileGier;
  if (ileGier == _g) {
    std::cout << maxZapytan << '\n';
    //std::cout << "OK. Zadano maksymalnie " << maxZapytan << " zapytan.\n";
    //std::cout << "OK. Maksymalnie " << maxKlamstw << " razy klamano.\n";
    //std::cout << "OK. " << ileMaxKlamstw << " razy klamano " << _k << " razy.\n";
    //std::cout << "PYTANO O " << which_chosen << " \n";
    exit(0);
  }
  _x = rand() % _n + 1;
  which_chosen[_x] = 1;
  ile_klamalem = 0;
  ileZapytan = 0;
}
