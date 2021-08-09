// adapted from the pseudocode at https://en.wikipedia.org/wiki/Mersenne_Twister

#define N 624
#define M 397
#define W 32

const unsigned f = 1812433253;
const unsigned a = 0x9908B0DF;
const unsigned u = 11, d = 0xFFFFFFFF;
const unsigned s = 7, b = 0x9D2C5680;
const unsigned t = 15, c = 0xEFC60000;

unsigned state [N];
unsigned next_idx = 0;

void seed(unsigned s) {
	state[0] = s;
	for (int i = 1; i < N; i++) {
		state[i] = ((f * (state[i-1] ^ (state[i-1] >> (W-2)))) + i);
	}
	next_idx = 0;
}

void twist() {
	for (int i = 0; i < N; i++) {
		int x = state[(i+1) % N];
		int xa = x >> 1;
		if (x & 0x1) {
			xa ^= a;
		}
		state[i] = state[(i + M) % N] ^ xa;
	}
}

unsigned next() {
	int y = state[next_idx];
	y ^= ((y >> u) & d);
	y ^= ((y >> s) & b);
	y ^= ((y >> t) & c);
	y ^= (y >> 1);
	next_idx += 1;
	if (next_idx == N) {
		twist();
		next_idx = 0;
	}
	return y;
}
