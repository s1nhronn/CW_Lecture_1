#include <cmath>
#include <exception>
#include <iostream>
#include <pthread.h>
#include <random>
#include <stdexcept>
#include <system_error>
#include <vector>

namespace hometask
{
  bool isInside(double x, double y, double r);
  void* threadAdapter(void* data);
  size_t calc(double r, size_t tests, size_t seed);
  double area(double r, size_t threads, size_t tests);

  struct Task
  {
    double r;
    size_t tests, seed;
    size_t result = 0;
  };

  constexpr size_t THREADS_COUNT = 10;
}

int main()
{
  double r = 0.0;
  size_t tests = 0;
  std::cin >> r >> tests;

  try
  {
    double res = hometask::area(r, hometask::THREADS_COUNT, tests);
    std::cout << "Методом Монте-Карло: " << res << '\n';
    std::cout << "По формуле: " << std::acos(-1.0) * r * r << '\n';
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return 1;
  }
}

bool hometask::isInside(double x, double y, double r)
{
  double dx = r - x, dy = r - y;
  return dx * dx + dy * dy <= r * r;
}

size_t hometask::calc(double r, size_t tests, size_t seed)
{
  std::default_random_engine engine(seed);

  double minVal = 0, maxVal = 2 * r;
  std::uniform_real_distribution< double > dist(minVal, maxVal);

  size_t res = 0;
  for (size_t i = 0; i < tests; ++i)
  {
    double x = dist(engine);
    double y = dist(engine);
    if (isInside(x, y, r))
    {
      ++res;
    }
  }
  return res;
}

void* hometask::threadAdapter(void* data)
{
  Task* task = static_cast< Task* >(data);
  task->result = calc(task->r, task->tests, task->seed);
  return nullptr;
}

double hometask::area(double r, size_t threads, size_t tests)
{
  if (!threads || !tests || r <= 0.0)
  {
    throw std::invalid_argument("All args must be greater than 0");
  }

  size_t base = tests / threads;
  size_t remainder = tests % threads;
  std::vector< pthread_t > ths(threads);
  std::vector< Task > vecOfTasks;
  vecOfTasks.reserve(threads);

  for (size_t i = 0; i < threads; ++i)
  {
    size_t currTestCount = base + ((i < remainder) ? 1 : 0);
    vecOfTasks.push_back({r, currTestCount, i});
  }

  for (size_t i = 0; i < threads; ++i)
  {
    int err = pthread_create(&ths[i], nullptr, threadAdapter, &vecOfTasks[i]);
    if (err)
    {
      for (size_t j = 0; j < i; ++j)
      {
        pthread_join(ths[j], nullptr);
      }
      throw std::system_error(err, std::generic_category(), "pthread_create failed");
    }
  }

  int firstError = 0;
  for (pthread_t th : ths)
  {
    int err = pthread_join(th, nullptr);
    if (err && !firstError)
    {
      firstError = err;
    }
  }

  if (firstError)
  {
    throw std::system_error(firstError, std::generic_category(), "pthread_join failed");
  }

  size_t count = 0;
  for (const Task& task : vecOfTasks)
  {
    count += task.result;
  }

  return 4 * r * r * static_cast< double >(count) / static_cast< double >(tests);
}
