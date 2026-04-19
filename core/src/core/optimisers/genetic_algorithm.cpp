#include "optimisers/genetic_algorithm.hpp"

#include "utils/math.hpp"
#include "utils/random.hpp"

namespace {
struct Individual {
  std::vector<double> genes;
  double fitness;
};

const Individual& TournamentSelect(const std::vector<Individual>& pop, int k) {
  // Select a best individual at random
  int best_idx = vanta::utils::RandInt(0, pop.size() - 1);

  // Select k individuals from the population at random and see which is best
  for (int i = 1; i < k; ++i) {
    int idx = vanta::utils::RandInt(0, pop.size() - 1);
    if (pop[idx].fitness < pop[best_idx].fitness) {
      best_idx = idx;
    }
  }
  return pop[best_idx];
}

std::vector<double> Crossover(const std::vector<double>& a,
                              const std::vector<double>& b,
                              double alpha = 0.5) {
  // Define child vector
  std::vector<double> child(a.size());

  // BLX-alpha crossover
  for (size_t i = 0; i < a.size(); ++i) {
    double minv = std::min(a[i], b[i]);
    double maxv = std::max(a[i], b[i]);
    double range = maxv - minv;
    double lo = minv - alpha * range;
    double hi = maxv + alpha * range;
    child[i] = vanta::utils::RandUniform() * (hi - lo) + lo;
  }

  return child;
}

void Mutate(std::vector<double>& genes, double rate, double strength,
            const std::vector<double>& lower,
            const std::vector<double>& upper) {
  for (size_t i = 0; i < genes.size(); ++i) {
    if (vanta::utils::RandUniform() < rate) {
      double range = (upper[i] - lower[i]);
      genes[i] += (vanta::utils::RandUniform() * 2 * strength * range) -
                  strength * range;
      genes[i] = vanta::utils::Clamp(genes[i], lower[i], upper[i]);
    }
  }
}

} // namespace

namespace vanta::optimisers {

vanta::optimisers::Solution GeneticAlgorithm(
    const std::function<double(const std::vector<double>&)>& f,
    const std::vector<double>& lower_bounds,
    const std::vector<double>& upper_bounds, GAOptions opts) {
  // Number of dimensions
  int dim = lower_bounds.size();

  // Initialise population
  std::vector<Individual> population(opts.population_size);

  for (auto& ind : population) {
    ind.genes.resize(dim);
    for (int i = 0; i < dim; ++i) {
      ind.genes[i] =
          vanta::utils::RandUniform() * (upper_bounds[i] - lower_bounds[i]) +
          lower_bounds[i];
    }
    ind.fitness = f(ind.genes);
  }

  Individual best = population[0];

  // Evolution loop
  int gen = 0;
  for (; gen < opts.max_generations; ++gen) {
    // Initialise new population vector
    std::vector<Individual> new_population;

    // Elitism
    best = *std::min_element(population.begin(), population.end(),
                             [](const Individual& a, const Individual& b) {
                               return a.fitness < b.fitness;
                             });

    new_population.push_back(best);

    // Generate rest
    while (static_cast<int>(new_population.size()) < opts.population_size) {
      // Select parents through tournaments
      const Individual& parent1 =
          TournamentSelect(population, opts.tournament_size);
      const Individual& parent2 =
          TournamentSelect(population, opts.tournament_size);

      // Create child
      std::vector<double> child_genes;

      if (vanta::utils::RandUniform() < opts.crossover_rate) {
        child_genes = Crossover(parent1.genes, parent2.genes);
      } else {
        child_genes = parent1.genes;
      }

      Mutate(child_genes, opts.mutation_rate, opts.mutation_strength,
             lower_bounds, upper_bounds);

      Individual child;
      child.genes = child_genes;
      child.fitness = f(child.genes);

      new_population.push_back(child);
    }

    population = std::move(new_population);

    // Convergence check
    if (best.fitness < opts.tolerance) {
      break;
    }
  }

  // Create solution structure
  vanta::optimisers::Solution sol{.f_val = best.fitness,
                                  .x = best.genes,
                                  .converged = best.fitness < opts.tolerance,
                                  .iters = gen};

  return sol;
}

}  // namespace vanta::optimisers
