#ifndef ABM_THREAD_POOL_HPP
#define ABM_THREAD_POOL_HPP

#include <atomic>
#include <mutex>
#include <thread>
#include <future>
#include <vector>
#include <queue>
#include <functional>
#include <type_traits>
#include <cassert>

namespace ABM
{
class ThreadPool
{
private:
  class Task
  {
  private:
    struct ImplBase
    {
      virtual void call() = 0;
      virtual ~ImplBase() { }
    };

    template<typename T>
    struct Impl : public ImplBase
    {
      T cally;

      Impl(T && cally) : cally(std::move(cally)) { }

      void call() override
      {
        cally();
      }
    };

    std::unique_ptr<ImplBase> impl;

  public:
    Task() = default;
    Task(Task && task) : impl(std::move(task.impl)) { }
    template<typename TFunction>
    Task(TFunction && func) : impl(new Impl<TFunction>(std::move(func))) { }
    Task & operator=(Task && task)
    {
      impl = std::move(task.impl);

      return *this;
    }

    Task(const Task &) = delete;
    Task(Task &) = delete;
    Task & operator=(const Task &) = delete;

    void operator()()
    {
      impl->call();
    }
  };

  std::atomic_bool done{false};
  std::mutex tasksMutex;
  std::queue<Task> tasks;
  std::vector<std::thread> threads;

public:
  explicit ThreadPool(std::size_t threadNumber)
  {
    assert(threadNumber != 0);

    for (std::size_t i{0}; i < threadNumber; ++i)
    {
      threads.emplace_back([this]
      {
        while (!done)
        {
          Task task;
          bool hasTask{false};

          {
            std::lock_guard<std::mutex> lock{tasksMutex};

            if (!tasks.empty())
            {
              hasTask = true;
              task = std::move(tasks.front());

              tasks.pop();
            }
          }

          if (hasTask)
          {
            task();
          }
          else
          {
            std::this_thread::yield();
          }
        }
      });
    }
  }

  ~ThreadPool()
  {
    done = true;

    for (auto & thread : threads)
    {
      thread.join();
    }
  }

  /**
   * @brief Adds a new task to the queue
   */
  template<typename TFunction, typename... TArgs>
  auto addTask(TFunction && func, TArgs && ... args)
  {
    using ReturnType = typename std::result_of<TFunction(TArgs...)>::type;

    auto bindedFunc = std::bind(std::forward<TFunction>(func),
                               std::forward<TArgs>(args)...);
    std::packaged_task<ReturnType()> newTask{bindedFunc};
    auto future = newTask.get_future();

    {
      std::lock_guard<std::mutex> lock{tasksMutex};

      tasks.push(std::move(newTask));
    }

    return future;
  }
};
}

#endif
