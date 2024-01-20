#include <vector>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <climits>
#include <random>
#include <array>
#include <bitset>
#include "wys.h"

#define Inf (INT_MAX - 1)
#define MAX_K 3

// Będziemy trzymać bitseta oznaczjaącego ktore liczby mogą być kandydatami na odpowiedź
typedef int64_t candidates_t;
// kandydaci beda tablica trzymana w intcie jako k+1 blokow n bitow
typedef int64_t candidates_list_t;

void set(int64_t& bitset, int64_t i) {
    bitset |= (1LL << i);
}

void reset(int64_t& bitset, int64_t i) {
    bitset &= ~(1LL << i);
}

// sufit z lg(x)
int64_t lg2c(int64_t x) {
    int64_t cur = 1;
    int64_t ans = 0;
    while(cur < x) {
        cur <<= 1;
        ++ans;
    }
    return ans;
}

// mozliwe stany wierzcholka, stan invalid oznacza niemozliwa konfiguracje odpowiedzi
enum GAME_STATE {
    TERMINAL,
    NON_TERMINAL,
    INVALID
};

struct Query {
    // O co się pytaliśmy
    int64_t y;
    // prawda gdy szukane < y
    bool ans;
};

// ustawia wszystkie bity na 1 miedzy a i b
constexpr void bitset_fill(candidates_list_t& bitset, int64_t a, int64_t b) {
    if(a > b) {
        std::swap(a, b);
    }
    bitset = (1LL << (b + 1)) - (1LL << a);
}

// szybki srand z https://stackoverflow.com/questions/26237419/faster-than-rand
static unsigned int seed;
inline int fast_srand(void) {
    seed = (214013 * seed + 2531011);
    return (seed >> 16) & 0x7FFF;
}

// ta klasa bedzie zapisywac sobie drzewo gry dla danego n i k
class WysSolver {
    int64_t n, k;
    // n losowych permutacji [2, n]
    std::vector<std::vector<int32_t>> shuffles;
    // spamietuje wyniki dla ustalonego stanu gdzie klucz to hash kandydatow dla danego stanu
    std::unordered_map<int64_t, int64_t> memo;
    // dla hashu stanu zapisuje gdzie isc
    std::unordered_map<int64_t, int32_t> where_to_go_map;
    // bedzie zawierac maske ktora ma jedynki na pozycjach od 0 do n - 1
    candidates_list_t mask;
    // analogiczna maska ale dla k + 1 blokow jedynek
    candidates_list_t big_mask;
    
    public:
        WysSolver(int _n, int _k) {
            this->n = _n;
            this->k = _k;

            std::random_device rd;
            std::mt19937 g(rd());

            for(int32_t i = 0; i < n; ++i) {
                shuffles.push_back({});
                for(int32_t j = 2; j <= n; ++j) {
                    shuffles[i].push_back(j);
                }
                std::shuffle(shuffles[i].begin(), shuffles[i].end(), g);
            }

            bitset_fill(mask, 0, n - 1);
            bitset_fill(big_mask, 0, (k + 1) * n - 1);
        }

        std::unordered_map<int64_t, int64_t> get_memo() {
            return memo;
        }

        std::unordered_map<int64_t, int32_t> get_where_to() {
            return where_to_go_map;
        }

        // zmienia array i depth na pojedynczy int64_t (z uwagi na ograniczenia na n i k array to tak naprawde int)
        constexpr int64_t hash_candidates(const candidates_list_t& candidates, int64_t depth) {
            return candidates + (depth << (k + 1) * n);
        }

        // zaznacza liczby ktore moga byc wedlug query
        constexpr candidates_t QueryToCandidates(const Query& query) {
            candidates_t ans = 0;
            int64_t beg = query.ans ? 0 : query.y - 1;
            int64_t end = query.ans ? query.y - 2 : n - 1;
            bitset_fill(ans, beg, end);
            return ans;
        }

        // zwraca sume zbiorow z listy
        constexpr candidates_list_t get_union(const candidates_list_t& list) {
            candidates_list_t ans = list;
            for(int64_t i = 0; i <= k; ++i) {
                ans |= ans >> (i * n);
            }
            ans &= mask;
            return ans;
        }

        // sprawdzamy terminalnosc stanu patrzac na wszystkich mozliwych kandydatow zaleznie od ilosci klamstw, w kazdym przypadku 
        // musi byc ten sam kandydat aby stan byl terminalny
        GAME_STATE is_terminal(const candidates_list_t& candidates) {
            /* Gdyby candidates bylo array chcielibysmy wykonac nastepujace operacje;
            candidates_t check = 0;
            for(size_t i = 0; i <= k; ++i) {
                check |= candidates[i];
            }
            if(!check) {
                return INVALID;
            }
            return __builtin_popcount(check) == 1 ? TERMINAL : NON_TERMINAL;
            */
            // w przypadku int64_t musimy to zrobic inaczej
            candidates_t check = get_union(candidates);
            if(!check) {
                return INVALID;
            }
            return __builtin_popcountll(check) == 1 ? TERMINAL : NON_TERMINAL;
        }

        candidates_list_t gen_initial_candidates() {
            candidates_list_t candidates = 0;
            bitset_fill(candidates, 0, (k + 1) * n - 1);
            return candidates;
        }

        int64_t solve_game() {
            int64_t max_depth = lg2c(n) * (2 * k + 1);
            return _solve_game(gen_initial_candidates(), max_depth);
        }

        // uwzglednia nowe query i liczy jacy candidates powinni byc w nastepnym stanie
        candidates_list_t get_new_candidates(const candidates_list_t& candidates, const Query& query) {
            candidates_t query_canidates = QueryToCandidates(query);
            // ta maska bedzie zawierac k + 1 kopii query_candidates do szybkiego andowania
            candidates_t query_candidates_mask = 0;
            for(int i = 0; i <= k; ++i) {
                query_candidates_mask <<= n;
                query_candidates_mask |= query_canidates;
            }
            candidates_list_t new_candidates = 0;
            /* Dla array kod bylby nastepujacy;
            new_candidates[0] = candidates[0] & query_canidates;
            for(size_t i = 1; i <= k; ++i) {
                // nasze query jest klamstwem albo nie, stad albo mamy starych kandydatow obostrzonych o query albo bierzemy kandydatow dla mniejszej ilosci klamstw 
                // i obostrzamy ich o to klamstwo
                new_candidates[i] = candidates[i] & query_canidates;
                new_candidates[i] |= candidates[i - 1] & ~query_canidates;
            }
            */
            new_candidates = candidates & query_candidates_mask;
            new_candidates |= (candidates << n) & ~query_candidates_mask;
            new_candidates &= big_mask;
            return new_candidates;
        }

        int64_t extract_ans(const candidates_list_t& candidates) {
            return __builtin_ctzll(get_union(candidates)) + 1;
        }

        // rozwiazuje gre i zwraca ilosc ruchow potrzebnych w optymalnej strategii
        int64_t _solve_game(const candidates_list_t& candidates, int64_t max_depth, int64_t depth = 0) {
            int64_t hash = hash_candidates(candidates, depth);
            if(memo.count(hash)) { 
                return memo[hash];
            }

            auto terminality = is_terminal(candidates);

            if(terminality != NON_TERMINAL) {
                memo.insert({hash, 0});
                return 0;
            } else if(depth > max_depth) {
                return Inf;
            }

            // bedziemy sie iterowac po losowej permutacji
            int64_t shuffle_ind = fast_srand() % n;
            int64_t ans = Inf;
            int32_t where_to_go = -1;
            // nasza wartosc musi byc dobrze zdefiniowana aby ja wlozyc do mapy
            bool can_insert = true;

            // rozwazamy mozliwe ruchy przy czym pytanie sie o jedynke jest bez sensu gdy drugi gracz gra optymalnie
            for(int32_t _i = 2; _i <= n; ++_i) {
                // prawdziwy indeks bedzie pochodzic z losowej permutacji permutacja
                int32_t i = shuffles[shuffle_ind][_i - 2];
                int64_t moves_needed = -Inf;
                // musimy wziac maksymalna ilosc ruchow z dwoch mozliwych odpowiedzi na nasze pytanie
                for(int j = 0; j < 2; ++j) {
                    auto q = Query {i, (bool) j};
                    // max, bo w opt strat interesuje nas najgorszy przypadek
                    moves_needed = std::max(
                        moves_needed, 
                        _solve_game(get_new_candidates(candidates, q), max_depth, depth + 1) + 1);
                }
                
                if(moves_needed != -Inf && moves_needed < ans) {
                    ans = moves_needed;
                    max_depth = std::min(max_depth, depth + moves_needed);
                    where_to_go = i;
                } else if(moves_needed == Inf) {
                    can_insert = false;
                }
            }

            if(can_insert) {
                where_to_go_map[hash] = where_to_go;
                memo.insert({hash, ans});
            }

            return ans;
        }
};

int main() {
    int n, k, g;
    dajParametry(n, k, g);
    WysSolver solver(n, k);
    solver.solve_game();
    auto where_to_map = std::move(solver.get_where_to());
    auto memo = std::move(solver.get_memo());
    for(int i = 0; i < g; ++i) {
        candidates_list_t candidates = solver.gen_initial_candidates();
        int32_t depth = 0;
        int64_t hash = solver.hash_candidates(candidates, depth);
        while(memo[hash]) {
            int32_t where_to_go = where_to_map[hash];
            Query q = {where_to_go, mniejszaNiz(where_to_go)};
            candidates = solver.get_new_candidates(candidates, q);
            hash = solver.hash_candidates(candidates, ++depth);
        }
        odpowiedz((int32_t) solver.extract_ans(candidates));
    }
    return 0;
}