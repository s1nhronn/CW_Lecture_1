#include <future>
#include <iostream>
#include <random>
#include <vector>

bool isInside(double x, double y, double r);
size_t calc(double r, size_t tests, size_t seed);
double area(double r, size_t threads, size_t tests);

int main()
{
  double r = 0.0;
  size_t tests = 0;
  std::cin >> r >> tests;
  std::cout << "Методом Монте-Карло: " << area(r, 10, tests / 10) << '\n';
  std::cout << "Формула: " << 3.14 * r * r << '\n';
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

double area(double r, size_t threads, size_t tests)
{
  std::vector< std::future< size_t > > results;
  results.reserve(threads);

  for (size_t i = 0; i < threads; ++i)
  {
    results.push_back(std::async(std::launch::async, calc, r, tests, i));
  }

  size_t count = 0;
  for (size_t i = 0; i < threads; ++i)
  {
    count += results[i].get();
  }

  return 4 * r * r * static_cast< double >(count) / static_cast< double >(threads * tests);
}
