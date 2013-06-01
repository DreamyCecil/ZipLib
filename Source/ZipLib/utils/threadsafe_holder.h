#pragma once
#include <mutex>

template <typename T, typename LOCK_TYPE = std::mutex>
class threadsafe_holder
{
  public:
    threadsafe_holder(T* instance, LOCK_TYPE& mutex)
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
    LOCK_TYPE& _mtx;
};

template <typename T, typename LOCK_TYPE = std::mutex>
struct enable_threadsafe_from_this
{
  public:
    threadsafe_holder<T, LOCK_TYPE> get_threadsafe_instance() 
    {
      return threadsafe_holder<T, LOCK_TYPE>(static_cast<T*>(this), _mtx);
    }

  protected:
    enable_threadsafe_from_this()
    {

    }

  private:
    LOCK_TYPE _mtx;
};
