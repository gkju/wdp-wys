#include <vector>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <climits>
#include <random>
#include <bitset>

#define Inf (INT_MAX - 1)
#define MAX_K 3

// Będziemy trzymać bitseta oznaczjaącego ktore liczby mogą być kandydatami na odpowiedź
typedef uint64_t candidates_t;
typedef std::array<candidates_t, MAX_K + 1> candidates_list_t;

void set(uint64_t& bitset, int i) {
    bitset |= (1ULL << i);
}

void reset(uint64_t& bitset, int i) {
    bitset &= ~(1ULL << i);
}

// sufit z lg(x)
int lg2c(int x) {
    int cur = 1;
    int ans = 0;
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
    int y;
    // prawda gdy szukane < y
    bool ans;
};

// ustawia wszystkie bity na 1 miedzy a i b
void bitset_fill(candidates_t& bitset, int a, int b) {
    if(a > b) {
        std::swap(a, b);
    }
    bitset = (1ULL << (b + 1)) - (1ULL << a);
}

/*
Główny claim - coś jest pewnym kłamstwem gdy nie zgadza się z odpowiedziami k+1 query
Po usunięciu pewnych kłamstw, kłamstwem mogą być dowolne query. - może nie? lepiej sprawdzić na start i zobaczyć jak szybko bedzie chodzic
*/

// szybki srand z https://stackoverflow.com/questions/26237419/faster-than-rand
static unsigned int seed;
inline int fast_srand(void) {
    seed = (214013 * seed + 2531011);
    return (seed >> 16) & 0x7FFF;
}

// ta klasa bedzie zapisywac sobie drzewo gry dla danego n i k
class WysSolver {
    int n, k;
    std::vector<Query> state_vec;
    // n losowych permutacji [2, n]
    std::vector<std::vector<int>> shuffles;
    
    public:
        WysSolver(int n, int k) {
            this->n = n;
            this->k = k;

            std::random_device rd;
            std::mt19937 g(rd());

            for(int i = 0; i < n; ++i) {
                shuffles.push_back({});
                for(int j = 2; j <= n; ++j) {
                    shuffles[i].push_back(j);
                }
                std::shuffle(shuffles[i].begin(), shuffles[i].end(), g);
            }
        }

        // zaznacza liczby ktore moga byc wedlug query
        candidates_t QueryToCandidates(const Query& query) {
            candidates_t ans = 0;
            int beg = query.ans ? 1 : query.y;
            int end = query.ans ? query.y - 1 : n;
            bitset_fill(ans, beg, end);
            return ans;
        }

        // sprawdzamy terminalnosc stanu patrzac na wszystkich mozliwych kandydatow zaleznie od ilosci klamstw, w kazdym przypadku 
        // musi byc ten sam kandydat aby stan byl terminalny
        GAME_STATE is_terminal(const candidates_list_t& candidates) {
            candidates_t check = 0;
            for(int i = 0; i <= k; ++i) {
                if(!candidates[i]) {
                    return INVALID;
                }
                check |= candidates[i];
            }
            return __builtin_popcount(check) == 1 ? TERMINAL : NON_TERMINAL;
        }

        int solve_game() {
            candidates_list_t candidates;
            for(int i = 0; i <= k; ++i) {
                candidates[i] = 0;
                for(int j = 1; j <= n; ++j) {
                    set(candidates[i], j);
                }
            }

            // WIP OPTYMISTYCZNIE OGRANICZAM PRZEZ lg2(n) * (k + 1)
            // musimy przekazac naszego bounda na strategie, aby nie wszedl w zle galezi
            return _solve_game(candidates, lg2c(n) * (2 * k + 1));
        }

        // uwzglednia nowe query i liczy jacy candidates powinni byc w nastepnym stanie
        candidates_list_t get_new_candidates(const candidates_list_t& candidates, const Query& query) {
            candidates_t query_canidates = QueryToCandidates(query);
            candidates_list_t new_candidates;
            new_candidates[0] = candidates[0] & query_canidates;
            for(int i = 1; i <= k; ++i) {
                // nasze query jest klamstwem albo nie, stad albo mamy starych kandydatow obostrzonych o query albo bierzemy kandydatow dla mniejszej ilosci klamstw 
                // i obostrzamy ich o to klamstwo
                new_candidates[i] = candidates[i] & query_canidates;
                new_candidates[i] |= candidates[i - 1] & ~query_canidates;
            }

            return new_candidates;
        }

        // rozwiazuje gre i zwraca ilosc ruchow potrzebnych w optymalnej strategii
        int _solve_game(const candidates_list_t& candidates, int current_best, int depth = 0) {
            auto terminality = is_terminal(candidates);
            if(terminality == TERMINAL || terminality == INVALID) {
                return 0;
            } else if(depth > current_best) {
                return Inf;
            }

            // bedziemy sie iterowac po losowej permutacji
            int shuffle_ind = fast_srand() % n;
            int ans = Inf;

            // rozwazamy mozliwe ruchy przy czym pytanie sie o jedynke jest bez sensu gdy drugi gracz gra optymalnie
            for(int _i = 2; _i <= n; ++_i) {
                // prawdziwy indeks bedzie pochodzic z losowej permutacji permutacja
                int i = shuffles[shuffle_ind][_i - 2];
                int moves_needed = -Inf;
                // musimy wziac maksymalna ilosc ruchow z dwoch mozliwych odpowiedzi na nasze pytanie
                for(int j = 0; j < 2; ++j) {
                    state_vec.push_back((Query) {i, (bool) j});
                    // max, bo w opt strat interesuje nas najgorszy przypadek
                    moves_needed = std::max(
                        moves_needed, 
                        _solve_game(get_new_candidates(candidates, state_vec.back()), current_best, depth + 1) + 1);
                    state_vec.pop_back();
                }
                
                ans = std::min(ans, moves_needed);
                current_best = std::min(current_best, ans);
            }

            return ans;
        }
};

int main() {
    int n, k;
    std::cin >> n >> k;
    WysSolver solver(n, k);
    std::cout << solver.solve_game() << std::endl;
    return 0;
}