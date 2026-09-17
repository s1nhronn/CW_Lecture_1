#include <cstring>
#include <iostream>
#include <pthread.h>
#include <random>
#include <tuple>

bool isInside(double x, double y, double r);
void* calc(void* data);
double area(double r, size_t threads, size_t tests, int& err);

int main()
{
  double r = 0.0;
  size_t tests = 0;
  std::cin >> r >> tests;

  int err = 0;
  double res = area(r, 10, tests / 10, err);
  if (err)
  {
    std::cerr << strerror(err) << '\n';
    return err;
  }

  std::cout << "Методом Монте-Карло: " << res << '\n';
  std::cout << "Формула: " << 3.14 * r * r << '\n';
}

bool isInside(double x, double y, double r)
{
  double dx = r - x, dy = r - y;
  return dx * dx + dy * dy <= r * r;
}

void* calc(void* args)
{
  // double r, size_t tests, size_t seed
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
  return reinterpret_cast< void* >(res);
}

double area(double r, size_t threads, size_t tests, int& err)
{
  std::vector< pthread_t > ths(threads);

  for (size_t i = 0; i < threads; ++i)
  {
    std::tuple< double, size_t, size_t > args = {r, tests, i};
    err = pthread_create(&ths[i], nullptr, calc, &args);
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
    size_t* thResult = nullptr;
    err = pthread_join(ths[i], reinterpret_cast< void** >(&thResult));
    if (err)
    {
      for (size_t j = i + 1; j < threads; ++j)
      {
        pthread_join(ths[j], nullptr);
      }
      return 0;
    }
    count += *thResult;
  }

  return 4 * r * r * static_cast< double >(count) / static_cast< double >(threads * tests);
}
