#include <vector>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <climits>
#include <random>
#include <bitset>

#define Inf (INT_MAX - 1)
// rozmiar maski bitowej pozycji klamstw
#define BITSET_SIZE 40

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

/*
Główny claim - coś jest pewnym kłamstwem gdy nie zgadza się z odpowiedziami k+1 query
Po usunięciu pewnych kłamstw, kłamstwem mogą być dowolne query. - może nie? lepiej sprawdzić na start i zobaczyć jak szybko bedzie chodzic
*/

// zwraca sufit lg(n)
int log2c(int n) {
    int res = 0;
    int cur = 1;
    while(cur < n) {
        cur *= 2;
        res++;
    }
    return res;
}

// liczy z iloma query nie zgadza się odpowiedź w q
int count_discrepancies(const Query &q, const std::vector<Query> &vec, const std::bitset<BITSET_SIZE>& lies_indices) {
    int cnt = 0;
    auto& [y, ans] = q;
    for(int i = 0; i < vec.size(); ++i) {
        auto& p = vec[i];
        if(lies_indices[i]) {
            continue;
        }
        if(ans) {
            // jesli query mowi, ze x < y, to sprzeczne beda odpowiedzi z x >= w gdzie w >= y
            cnt += (p.y >= y && !p.ans);
        } else {
            // jesli query mowi, ze x >= y, to sprzeczne beda odpowiedzi z x < w gdzie w <= y
            cnt += (p.y <= y && p.ans);
        }
    }
    return cnt;
}

// typ do trzymania listy krotek liczb do iterowania sie
typedef std::vector<std::vector<int>> tuple_list_t;

// zwraca wszystkie k-elementowe niemalejace krotki liczb z przedzialu [a, b]
tuple_list_t gen_incr_tuples(int a, int b, int k) {
    tuple_list_t res;
    if(!k) {
        res.push_back({});
        return res;
    }
    // rekurencyjnie generujemy krotki po ustaleniu 1. elementu
    for(int i = a; i <= b - k + 1; ++i) {
        auto vec = gen_incr_tuples(i + 1, b, k - 1);
        for(auto& v : vec) {
            v.insert(v.begin(), i);
            res.push_back(v);
        }
    }
    return res;
}

// szybki srand z https://stackoverflow.com/questions/26237419/faster-than-rand
static unsigned int seed;
inline int fast_srand(void) {
    seed = (214013 * seed + 2531011);
    return (seed >> 16) & 0x7FFF;
}

class WysSolver {
    // max_len to gorny bound na dlugosc rozwiazania
    int n, k, max_len;
    std::vector<Query> state_vec;
    // n losowych permutacji [2, n]
    std::vector<std::vector<int>> shuffles;
    // uniwersalne bufory
    std::bitset<40> buf;
    std::vector<int> vec_buf;
    // bedziemy trzymac wszystkie 1, 2, ..., k-elementowe rosnace krotki liczb od 0 do 0, 1, 2, ..., lg(n)*(2k+1) aby sie iterowac po konfiguracjach klamstw
    std::vector<std::vector<tuple_list_t>> incr_tuples;
    public:
        WysSolver(int n, int k) {
            this->n = n;
            this->k = k;
            incr_tuples.resize(k);
            int lg = log2c(n);
            this->max_len = lg * (2 * k + 1);
            vec_buf.reserve(max_len);
            for(int i = 0; i < k; ++i) {
                incr_tuples[i].resize(max_len + 2);
                for(int j = 0; j <= max_len + 1; ++j) {
                    // teraz na [i][j] mamy rosnace krotki i + 1 liczb od 0 do j
                    incr_tuples[i][j] = gen_incr_tuples(0, j, i + 1);
                }
            }

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

        int get_max_len() {
            return max_len;
        }

        // sprawdzamy terminalnosc stanu
        GAME_STATE is_terminal() {
            std::cout << "CHECKING TERMINALITY FOR " << state_vec.size() << " " << std::endl;
            // wszystkie query moga byc klamstwami
            if(state_vec.size() <= k) {
                return NON_TERMINAL;
            }
            // od teraz niech bufor trzyma pozycje klamstw (bedziemy sie iterowac po konfiguracjach)
            for(int i = 0; i < state_vec.size(); ++i) {
                buf[i] = 0;
            }
            // znajdzmy pewne klamstwa
            int k_left = k;
            for(int i = 0; i < state_vec.size(); ++i) {
                auto& q = state_vec[i];
                if(count_discrepancies(q, state_vec, buf) > k_left) {
                    k_left--;
                    buf[i] = 1;
                }
            }

            if(k_left < 0) {
                return INVALID;
            }

            // jednoznacznie okreslone sa pozycje klamstw, mozemy od razu sprawdzic czy znamy odpowiedz
            if(!k_left) {
                return 0 < get_ans_in_state(buf) ? TERMINAL : NON_TERMINAL;
            }
            
            vec_buf.clear();
            // vec_buf trzyma mozliwe indeksy klamstw
            for(int j = 0; j < state_vec.size(); ++j) {
                if(buf[j]) {
                    vec_buf.push_back(j);
                }
            }

            tuple_list_t tuples = incr_tuples[k_left - 1][vec_buf.size() - 1];

            // ta zmienna bedzie trzymac jaka powinna byc odpowiedz we wszystkich konfiguracjach klamstw
            int current_ans = -1;
            for(auto& t : tuples) {
                for(int ind : t) {
                    buf[vec_buf[ind]] = 1;
                }

                int ans = get_ans_in_state(buf);
                if(ans == -2) {
                    continue;
                } else if(ans == -1) {
                    // jesli nie da sie jednoznacznie okreslic liczby, to nie mozemy byc w terminalnym stanie
                    return NON_TERMINAL;
                } 
                // jesli da sie jednoznacznie okreslic liczbe, to sprawdzamy czy zgadza sie z poprzednimi
                if(current_ans == -1) {
                    current_ans = ans;
                } else if(current_ans != ans) {
                    return NON_TERMINAL;
                }

                for(int ind : t) {
                    buf[vec_buf[ind]] = 0;
                }
            }

            return current_ans > 0 ? TERMINAL : NON_TERMINAL;
        }

        // ta funkcja sprawdza czy dla okreslonych indeksow klamstw da sie uzyskac odpowiedz i jaka ona jest
        // -1 oznacza ze nie da sie jednoznacznie okreslic liczby
        // -2 oznacza ze niemozliwa jest taka konfiguracja klamstw
        // wpp zwraca liczbe
        int get_ans_in_state(std::bitset<BITSET_SIZE>& lies_indices) {
            // na jakim przedziale moze byc obecnie liczba
            int beg = 1, end = n;

            for(int i = 0; i < this->state_vec.size(); ++i) {
                if(lies_indices[i]) {
                    continue;
                }

                auto& q = this->state_vec[i];
                auto& [y, ans] = q;
                if(ans) {
                    // query mowi ze x < y
                    end = std::min(end, y - 1);
                } else {
                    // query mowi ze x >= y
                    beg = std::max(beg, y);
                }
            }

            // dostalismy sprzeczna konfiguracje, nie mozliwe jest, ze klamstwa tak byly rozlozone
            if(beg > end) {
                return -2;
            }

            // da sie jednoznacznie okreslic liczbe
            if(beg == end) {
                return beg;
            }

            return -1;
        }

        // rozwiazuje gre i zwraca ilosc ruchow potrzebnych w optymalnej strategii
        int solve_game(int current_best, int depth = 0) {
            auto terminality = is_terminal();
            if(terminality == TERMINAL) {
                return 0;
            } else if(depth > current_best || terminality == INVALID) {
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
                    moves_needed = std::max(moves_needed, solve_game(current_best, depth + 1) + 1);
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
    std::cout << solver.solve_game(solver.get_max_len()) << std::endl;
    return 0;
}