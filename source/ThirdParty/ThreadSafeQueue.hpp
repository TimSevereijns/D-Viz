#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <type_traits>
#include <queue>

template<typename Type>
class ThreadSafeQueue
{
   static_assert(std::is_move_assignable<Type>::value, "Type has to be move-assignable.");
   static_assert(std::is_move_constructible<Type>::value, "Type has to be move-constructible.");

public:

   void Push(Type data)
   {
      std::lock_guard<decltype(m_mutex)> lock{ m_mutex };

      m_queue.push(std::move(data));

      m_conditionVariable.notify_one();
   }

   template<typename... Args>
   void Emplace(Args&&... args)
   {
      std::lock_guard<decltype(m_mutex)> lock{ m_mutex };

      m_queue.emplace(std::forward<Args>(args)...);

      m_conditionVariable.notify_one();
   }

   void WaitAndPop(Type& data)
   {
      std::unique_lock<decltype(m_mutex)> lock{ m_mutex };
      m_conditionVariable.wait(lock, [this] { return !m_queue.empty(); });

      data = std::move(m_queue.front());

      m_queue.pop();
   }

   std::shared_ptr<Type> WaitAndPop()
   {
      std::unique_lock<decltype(m_mutex)> lock{ m_mutex };
      m_conditionVariable.wait(lock, [this] { return !m_queue.empty(); });

      const auto result = std::make_shared<Type>(std::move(m_queue.front()));
      m_queue.pop();

      return result;
   }

   bool TryPop(Type& data)
   {
      std::lock_guard<decltype(m_mutex)> lock{ m_mutex };

      if (m_queue.empty())
      {
         return false;
      }

      data = std::move(m_queue.front());
      m_queue.pop();

      return true;
   }

   std::shared_ptr<Type> TryPop()
   {
      std::lock_guard<decltype(m_mutex)> lock{ m_mutex };

      if (m_queue.empty())
      {
         return{};
      }

      const auto result = std::make_shared<Type>(std::move(m_queue.front()));
      m_queue.pop();

      return result;
   }

   bool IsEmpty() const
   {
      std::lock_guard<decltype(m_mutex)> lock{ m_mutex };
      return m_queue.empty();
   }

private:
   mutable std::mutex m_mutex;

   std::queue<Type> m_queue;
   std::condition_variable m_conditionVariable;
};
