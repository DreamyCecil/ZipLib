#pragma once
#include <mutex>

template <typename T>
class threadsafe_holder
{
  public:
    threadsafe_holder(T* instance, std::mutex& mutex)
      : _instance(instance)
      , _mtx(mutex)
    {
      _mtx.lock();
    }

    ~threadsafe_holder()
    {
      _mtx.unlock();
    }

    T* operator -> ()
    {
      return _instance;
    }

  private:
    T* _instance;
    std::mutex& _mtx;
};

template <typename T>
struct enable_threadsafe_from_this
{
  public:
    threadsafe_holder<T> get_threadsafe_instance() 
    {
      return threadsafe_holder<T>(static_cast<T*>(this), _mtx);
    }

  protected:
    enable_threadsafe_from_this()
    {

    }

  private:
    std::mutex _mtx;
};
