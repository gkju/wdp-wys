#include <algorithm>
#include <unordered_map>
#include <climits>
#include <cstdint>
#include "wys.h"

#define Inf (INT_MAX - 1)

// Będziemy trzymać bitseta oznaczającego ktore liczby mogą być kandydatami na odpowiedź
typedef uint64_t candidates_t;
// kandydaci beda tablica trzymana w intcie jako k+1 blokow n bitow
typedef uint64_t candidates_list_t;

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
    uint64_t y;
    // prawda gdy szukane < y
    bool ans;
};

// ustawia wszystkie bity na 1 miedzy a i b
void bitset_fill(candidates_list_t& bitset, uint64_t a, uint64_t b) {
    if(a > b) {
        std::swap(a, b);
    }
    bitset = (1ULL << (b + 1)) - (1ULL << a);
}

// ta klasa bedzie zapisywac sobie drzewo gry dla danego n i k
class WysSolver {
    uint64_t n, k;
    // spamietuje (wyniki, gdzie isc) dla ustalonego stanu gdzie klucz to hash kandydatow dla danego stanu
    typedef std::unordered_map<uint64_t, std::pair<uint32_t, uint32_t>> memo_t;
    memo_t memo;
    // bedzie zawierac maske ktora ma jedynki na pozycjach od 0 do n - 1
    candidates_t mask;
    // analogiczna maska ale dla k + 1 blokow jedynek
    candidates_list_t big_mask;
    
    public:
        WysSolver(int _n, int _k) {
            this->n = _n;
            this->k = _k;

            bitset_fill(mask, 0, n - 1);
            bitset_fill(big_mask, 0, (k + 1) * n - 1);
        }

        memo_t get_memo() {
            return memo;
        }

        // zmienia array i depth na pojedynczy int64_t (z uwagi na ograniczenia na n i k array to tak naprawde int)
        uint64_t hash_candidates(const candidates_list_t& candidates) {
            return candidates;
        }

        // zaznacza liczby ktore są zgodne z query
        candidates_t QueryToCandidates(const Query& query) {
            candidates_t ans = 0;
            uint64_t beg = query.ans ? 0 : query.y - 1;
            uint64_t end = query.ans ? query.y - 2 : n - 1;
            bitset_fill(ans, beg, end);
            return ans;
        }

        // zwraca sume zbiorow z listy
        candidates_t get_union(const candidates_list_t& list) {
            candidates_t ans = list;
            for(uint64_t i = 0; i <= k; ++i) {
                ans |= ans >> (i * n);
            }
            ans &= mask;
            return ans;
        }

        // sprawdzamy terminalnosc stanu patrzac na wszystkich mozliwych kandydatow zaleznie od ilosci klamstw, w kazdym przypadku 
        // musi byc ten sam kandydat aby stan byl terminalny
        GAME_STATE is_terminal(const candidates_list_t& candidates) {
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

        uint64_t solve_game() {
            return _solve_game(gen_initial_candidates(), Inf);
        }

        // uwzglednia nowe query i liczy jacy candidates powinni byc w nastepnym stanie
        candidates_list_t get_new_candidates(const candidates_list_t& candidates, const Query& query) {
            candidates_t query_canidates = QueryToCandidates(query);
            // ta maska bedzie zawierac k + 1 kopii query_candidates do szybkiego andowania
            candidates_t query_candidates_mask = 0;
            for(uint32_t i = 0; i <= k; ++i) {
                query_candidates_mask <<= n;
                query_candidates_mask |= query_canidates;
            }
            candidates_list_t new_candidates = candidates & query_candidates_mask;
            new_candidates |= (candidates << n) & ~query_candidates_mask;
            new_candidates &= big_mask;
            return new_candidates;
        }

        uint64_t extract_ans(const candidates_list_t& candidates) {
            return __builtin_ctzll(get_union(candidates)) + 1;
        }

        // rozwiazuje gre i zwraca ilosc ruchow potrzebnych w optymalnej strategii
        uint64_t _solve_game(const candidates_list_t& candidates, int64_t max_depth, int64_t depth = 0) {
            uint64_t hash = hash_candidates(candidates);
            if(memo.find(hash) != memo.end()) {
                return memo[hash].first;
            }

            auto terminality = is_terminal(candidates);

            if(terminality != NON_TERMINAL) {
                memo.insert({hash, {0, 0}});
                return 0;
            }

            uint64_t ans = Inf;
            uint32_t where_to_go = 0;
            // nasza wartosc musi byc dobrze zdefiniowana aby ja wlozyc do mapy
            bool can_insert = true;

            // rozwazamy mozliwe ruchy przy czym pytanie sie o jedynke jest bez sensu gdy drugi gracz gra optymalnie
            for(uint32_t i = 2; i <= n; ++i) {
                Query queries[2];
                for(uint32_t j = 0; j < 2; ++j) {
                    queries[j] = {i, (bool) j};
                }
                // jesli da sie odpowiedziec na nasze pytanie tak, ze stan sie nie zmienia, to nie ma sensu pytac o i
                bool check = false;
                for(auto& q : queries) {
                    if(get_new_candidates(candidates, q) == candidates) {
                        check = true;
                    }
                }
                if(check) {
                    continue;
                }
                // 0 jest domyslnie nieprawidlowa wartoscia, bo jestesmy w stanie nieterminalnym
                uint64_t moves_needed = 0;
                // musimy wziac maksymalna ilosc ruchow z dwoch mozliwych odpowiedzi na nasze pytanie
                for(auto& q : queries) {
                    // max, bo w optymalnej strategii interesuje nas najgorszy przypadek
                    moves_needed = std::max(
                        moves_needed, 
                        _solve_game(get_new_candidates(candidates, q), max_depth, depth + 1) + 1);
                }
                
                if(moves_needed != 0 && moves_needed < ans) {
                    ans = moves_needed;
                    where_to_go = i;
                } else if(moves_needed == Inf) {
                    can_insert = false;
                }
            }

            if(can_insert) {
                memo.insert({hash, {ans, where_to_go}});
            }

            return ans;
        }
};

int main() {
    int n, k, g;
    dajParametry(n, k, g);
    WysSolver solver(n, k);
    solver.solve_game();
    auto memo = solver.get_memo();
    for(int i = 0; i < g; ++i) {
        candidates_list_t candidates = solver.gen_initial_candidates();
        uint64_t hash = solver.hash_candidates(candidates);
        while(memo[hash].first) {
            uint32_t where_to_go = memo[hash].second;
            Query q = {where_to_go, mniejszaNiz(where_to_go)};
            candidates = solver.get_new_candidates(candidates, q);
            hash = solver.hash_candidates(candidates);
        }
        odpowiedz((int32_t) solver.extract_ans(candidates));
    }
    return 0;
}