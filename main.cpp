#include <cmath>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <random>
#include <vector>

bool isInside(double x, double y, double r);
void* threadAdapter(void* data);
size_t calc(double r, size_t tests, size_t seed);
double area(double r, size_t threads, size_t tests, int& err);

struct args_t
{
  double r;
  size_t tests, seed;
};

constexpr size_t THREADS_COUNT = 10;

int main()
{
  double r = 0.0;
  size_t tests = 0;
  std::cin >> r >> tests;
  if (tests < THREADS_COUNT)
  {
    tests = THREADS_COUNT;
  }

  int err = 0;
  double res = area(r, THREADS_COUNT, tests / THREADS_COUNT, err);
  if (err)
  {
    std::cerr << strerror(err) << '\n';
    return err;
  }

  std::cout << "Методом Монте-Карло: " << res << '\n';
  std::cout << "Формула: " << std::acos(-1.0) * r * r << '\n';
}

bool isInside(double x, double y, double r)
{
  double dx = r - x, dy = r - y;
  return dx * dx + dy * dy <= r * r;
}

size_t calc(double r, size_t tests, size_t seed)
{
  std::default_random_engine engine(seed);

  double minVal = 0, maxVal = 2 * r;
  std::uniform_real_distribution< double > dist(minVal, maxVal);

  size_t res = 0;
  for (size_t i = 0; i < tests; ++i)
  {
    if (isInside(dist(engine), dist(engine), r))
    {
      ++res;
    }
  }
  return res;
}

void* threadAdapter(void* data)
{
  auto args = *static_cast< args_t* >(data);
  return reinterpret_cast< void* >(calc(args.r, args.tests, args.seed));
}

double area(double r, size_t threads, size_t tests, int& err)
{
  std::vector< pthread_t > ths(threads);
  std::vector< args_t > vecOfArgs;
  vecOfArgs.reserve(threads);

  for (size_t i = 0; i < threads; ++i)
  {
    vecOfArgs.push_back({r, tests, i});
  }

  for (size_t i = 0; i < threads; ++i)
  {
    err = pthread_create(&ths[i], nullptr, threadAdapter, &vecOfArgs[i]);
    if (err)
    {
      for (size_t j = 0; j < i; ++j)
      {
        pthread_join(ths[j], nullptr);
      }
      return 0;
    }
  }

  size_t count = 0;
  for (size_t i = 0; i < threads; ++i)
  {
    void* thResult = nullptr;
    err = pthread_join(ths[i], &thResult);
    if (err)
    {
      for (size_t j = i + 1; j < threads; ++j)
      {
        pthread_join(ths[j], nullptr);
      }
      return 0;
    }
    count += reinterpret_cast< size_t >(thResult);
  }

  return 4 * r * r * static_cast< double >(count) / static_cast< double >(threads * tests);
}
