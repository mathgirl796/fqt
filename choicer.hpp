#pragma once
#include <vector>
#include <random>
#include <cstring>

class Choicer {
private:
	int _seed;
	int _nsample;
	int* _nreads;
public:
	std::vector<int> v;
	int* nreads_status;
	Choicer(int nsample, int* nreads, int seed = 0) {
		_seed = seed;
		_nsample = nsample;
		_nreads = nreads;
		_nreads = (int*)malloc(nsample * sizeof(int));
        memcpy(_nreads, nreads, _nsample * sizeof(int));
		nreads_status = (int*)malloc(nsample * sizeof(int));
		reset();
	}
	int next() {
		if (v.size() <= 0) return -1;
		int i = rand() % v.size();
		int id = v[i];
		nreads_status[id]--;
		if (nreads_status[id] == 0) v.erase(v.begin() + i);
		return id;
	}
	void reset() {
		srand(_seed);
		v.clear();
		for (int i = 0; i < _nsample; ++i) {
			v.emplace_back(i);
		}
		memcpy(nreads_status, _nreads, _nsample * sizeof(int));
	}
};