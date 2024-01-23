//Task: Wyszukiwanie
//Author: Bartosz Niedzwiedz
#include <bitset>
#include <map>
#include "wys.h"

const char N = 12, K = 4, MAX_QUESTION = 28;

typedef unsigned long long int LL;

/*
We will consider all sensible configurations of answers - we will keep a bitset<N> possible[K],
where possible[i][j] = 0 if and only if from all the previosly asked questions me can deduct
that if Alice lied j times answer is certainly not i (otherwise if we cannot make such a claim possible[i][j] = 1)
in every state of the backtrack we will look for.
For every configuration of answers, we will keep the best question which should be asked next, in the std::map (named move)
Additionally, to make the backtrack more efficient
we will use std::map (named depth) to keep record of an upper bound on the number of questions we have to ask to know the answer
*/

static std::map<LL, char> depth, move;
static std::bitset<N> pref[N], suf[N];
static char n, k;

//converts an array of bitsets to unsigned long long int, to keep them in a more efficient way in the map
inline LL bitsets_to_LL(const std::bitset<N> possible[K]) {
	LL ans = 0;
	for(int i = 0; i < K; i++)
		ans |= ((possible[i].to_ullong()) << (N * i));
	return ans;
}

//checks whether the number of possible answers is lower than 2
inline char deductable(const std::bitset<N> possible[K]) {
	char ans = 0;
	for(int i = 0; i < n; i++) {
		int pos = 0;
		for(int j = 0; j <= k; j++)
			pos += (possible[j][i] == 1);
		if(pos)
			ans++;
	}
	return ans <= 1;
}

//for a question mniejszeNiz(q) returns a bitset corresponding to the possible answer after this query
inline std::bitset<N> get_mask(const bool &alice_answers_yes, const char &q) {
	if(alice_answers_yes)
		return pref[q - 1];
	return suf[(int)q];
}

//updates the newly created state (new_move) according to the question about q and the answer type
inline void ask_question(const std::bitset<N> possible[K], std::bitset<N> new_move[K], const char &q, const bool &type) {
	//Either Alice didn't lie (then we consider the previous answer  with 'i' lies updated accordingly by the question q) 
	//or she lied (then we consider previous answer with 'i - 1' lies updated accordingly by the question q)
	for(int i = k; i; i--)
		new_move[i] = ((possible[i] & get_mask(type, q)) | (possible[i - 1] & get_mask(type ^ 1, q)));
	new_move[0] = possible[0] & get_mask(type, q);
}

//returns a pessimistic number of questions left to ask from the possible[K] state
char backtrack(const std::bitset<N> possible[K]) {
	auto ans = depth.find(bitsets_to_LL(possible));
	if(ans != depth.end())
		return ans->second;
	if(deductable(possible)) {
		depth[bitsets_to_LL(possible)] = 0;
		return 0;
	}
	char best_depth = MAX_QUESTION, best_move = 0;
	for(char q = 1; q < n; q++) {
		char move_depth = 0;
		for(char type = 0; type < 2; type++) {
			std::bitset<N> new_move[K];
			ask_question(possible, new_move, q, type);
			//we don't want to visit the same configuration of answers
			if(bitsets_to_LL(new_move) == bitsets_to_LL(possible)) {
				move_depth = MAX_QUESTION;
				break;
			}
			move_depth = std::max(move_depth, (char)(backtrack(new_move) + 1));
		}
		if(best_depth > move_depth) {
			best_depth = move_depth;
			best_move = q;
		}
	}
	move[bitsets_to_LL(possible)] = best_move;
	depth[bitsets_to_LL(possible)] = best_depth;
	return best_depth;
}

inline void play() {
	std::bitset<N> state[K];
	for(int i = 0; i <= k; i++)
		state[i] = pref[n - 1];
	while(!deductable(state)) {
		char question = move[bitsets_to_LL(state)];
		bool ans = mniejszaNiz(question + 1);
		std::bitset<N> new_move[K];
		ask_question(state, new_move, question, ans);
		for(int i = 0; i <= k; i++)
			state[i] = new_move[i];
	}
	int ans = 0;
	for(int i = 0; i < n; i++)
		for(int j = 0; j <= k; j++)
			if(state[j][i])
				ans = i;
	odpowiedz(ans + 1);
}

int main() {
	int range, lies, nr_questions;
	dajParametry(range, lies, nr_questions);
	n = (char)range; k = (char)lies;
	for(int i = 0; i < n; i++)
		for(int j = 0; j <= i; j++)
			pref[i][j] = 1;
	for(int i = 0; i < n; i++)
		for(int j = i; j < n; j++)
			suf[i][j] = 1;
	std::bitset<N> start[K];
	for(int i = 0; i <= k; i++)
		start[i] = pref[n - 1];
	backtrack(start);
	while(nr_questions--)
		play();
	depth.erase(depth.begin(), depth.end());
	move.erase(move.begin(), move.end());
	return 0;
}