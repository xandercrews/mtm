#include <chrono>
#include <cmath>
#include <cstdlib>
#include <random>

#include <stdio.h>

using std::mt19937;
using std::chrono::system_clock;
using std::min;
using std::max;

/**
 * Return the cumulative probability that a number will be equal or less than
 * the specified number, where probability is a classic bell curve with range
 * 0..1.
 *
 * Numbers < 0.5 have a neg score; numbers > 0.5 have a positive score.
 */
double raw_norminv(double q) {
	if (q == .5)
		return 0;

	q = 1.0 - q;

	double p = (q > 0.0 && q < 0.5) ? q : (1.0 - q);
	double t = sqrt(log(1.0 / pow(p, 2.0)));

	double c0 = 2.515517;
	double c1 = 0.802853;
	double c2 = 0.010328;

	double d1 = 1.432788;
	double d2 = 0.189269;
	double d3 = 0.001308;

	double x = t
			- (c0 + c1 * t + c2 * pow(t, 2.0))
					/ (1.0 + d1 * t + d2 * pow(t, 2.0) + d3 * pow(t, 3.0));
	if (q > .5)
		x *= -1.0;

	return x;
}

double get_random_ratio() {
	static unsigned seed = system_clock::now().time_since_epoch().count();
	static mt19937 randomizer(seed);
	unsigned n = static_cast<unsigned>(randomizer()) % 1000000000u;
	return n / 1000000000.0;
}

/**
 * Return the cumulative probability that a number will be equal or less than
 * the specified number, where probability is a classic bell curve with range
 * 0..1.
 */
double norminv(double q) {
	return raw_norminv(q) + 0.5;
}

/**
 * Return the cumulative probability that a number will be equal or less than
 * the specified number, given a classic bell curve with the specified mean
 * and stddev. Unlike raw_norminv, this func returns no negative values, so
 * the probability at the right side of the curve approaches 1.0, and the
 * probability at the left side approaches 0.0.
 */
double norminv(double q, double mean, double stddev) {
	// Assume 7 std deviations encompasses everything interesting.
	double lower_bound = mean - 3.5 * stddev;
	if (q < lower_bound) {
		return 0.0;
	}
	double upper_bound = lower_bound + (7.0 * stddev);
	if (q > upper_bound) {
		return 1.0;
	}
	double range = upper_bound - lower_bound;
	// Map q into a proportional spot in the canonical range.
	double ratio = (q - lower_bound) / range;
	return norminv(ratio);
}

double probability_of_war() {
	// low but rising slightly over time
	return norminv(0.2 * get_random_ratio());
}

double probability_that_market_will_tank() {
	// low but rising slightly over time; higher than war
	return 1.5 * norminv(0.5 * get_random_ratio());
}

double probability_that_competitor_will_implode() {
	// random, uniform
	return get_random_ratio();
}

double probability_of_competitor_quantum_leap() {
	// low but rising slightly over time
	return norminv(min(0.3 + get_random_ratio(), 0.6));
}

double probability_of_quantum_leap() {
	// sawtooth
	auto raw = get_random_ratio();
	return (raw > 0.5) ? 1.0 - raw : raw *= 0.4;
}

double day_of_week_influence() {
	return norminv(get_random_ratio());
}

double time_of_day_influence() {
	auto raw = get_random_ratio();
	if (raw < 0.33) {
		return raw * .2;
	} else if (raw > 0.65) {
		return raw * .3;
	} else if (raw < 0.5) {
		return max(1.0, raw * 2);
	}
	return raw;
}

double probability_of_gvt_shutdown() {
	double raw = get_random_ratio();
	if (raw < 0.25) {
		raw = 0;
	}
	if (raw < 0.75) {
		raw = min(1.0, raw * 2);
	}
	return norminv(raw);
}

int main(int argc, char ** argv) {

	if (argc > 1 && (argv[1][0] == '-' || argv[1][0] == '?')) {
		printf("monte -- simulate ticker price under various scenarios\n");
		return 0;
	}

	double initial_price = 37.50;
	double timeframe = get_random_ratio();

	double multiplier = 1.0
			+ probability_of_quantum_leap()
			- (probability_of_competitor_quantum_leap()
					- probability_that_competitor_will_implode() * 0.5)
			- (probability_that_market_will_tank() + 0.5)
			- (probability_of_war());

	multiplier = multiplier
			* (0.5 + day_of_week_influence())
			* (0.5 + time_of_day_influence())
			* timeframe;

	// Sanity filter
	if (multiplier < 0.35) {
		multiplier = 1.0 - multiplier;
	}
	while (multiplier > 2.5) {
		multiplier = norminv(multiplier, 1.5, 1);
	}

	double simulated_price = initial_price * (1.0 + multiplier);

	printf("%.2f\n", simulated_price);
	return 0;
}
