/**
* The MIT License (MIT)
*
* Copyright (c) 2016 Tim Severeijns
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include <cassert>
#include <cstddef>
#include <iostream>

/**
* @brief The basic building block in this whole allocator scheme. This struct wraps an allocated
* block of memory and its associated size.
*/
struct MemoryBlock
{
   using pointer_type = unsigned char*;
   using const_pointer_type = const unsigned char*;

   pointer_type pointer;
   std::size_t length;
};

/**
* @brief A wrapper around a few basic functions to allocate and free MemoryBlocks from the heap.
*/
struct Mallocator
{
   inline static MemoryBlock Allocate(std::size_t requestedSize)
   {
      std::cout << "Allocating " << requestedSize << " bytes on the heap\n";

      MemoryBlock block
      {
         static_cast<MemoryBlock::pointer_type>(std::malloc(requestedSize * sizeof(std::size_t))),
         requestedSize
      };

      return block;
   }

   inline static void Deallocate(const MemoryBlock& block) noexcept
   {
      std::cout << "Deallocating " << block.length << " bytes on the heap\n";

      assert(block.length);
      assert(block.pointer);

      std::free(block.pointer);
   }
};

/**
* @brief Linear memory arena of user specified size and alignment.
*/
template<
   std::size_t ArenaSize,
   std::size_t Alignment = alignof(std::max_align_t)
>
class MemoryArena
{
public:

   MemoryArena() noexcept :
      m_pointer{ GetBufferPointer() }
   {
   }

   ~MemoryArena()
   {
      m_pointer = nullptr;
   }

   MemoryArena(const MemoryArena&) = delete;

   MemoryArena& operator=(const MemoryArena&) = delete;

   MemoryBlock Allocate(std::size_t requestedSize)
   {
      const auto alignedSize = RoundUpToNextAlignment(requestedSize);
      if (static_cast<decltype(alignedSize)>(ArenaSize - Used()) >= alignedSize)
      {
         MemoryBlock block
         {
            m_pointer,
            requestedSize
         };

         m_pointer += alignedSize;
         return block;
      }

      return Mallocator::Allocate(requestedSize);
   }

   void Deallocate(const MemoryBlock& block) noexcept
   {
      if (Owns(block))
      {
         // Since we're working out of a single linear buffer, the only time we can actually
         // deallocate an object is if that object is the last one to have been allocated in
         // our buffer.

         const auto alignedSize = RoundUpToNextAlignment(reinterpret_cast<std::size_t>(block.pointer));
         if (block.pointer + alignedSize == m_pointer)
         {
            m_pointer = block.pointer;
         }

         return;
      }

      Mallocator::Deallocate(block);
   }

   static constexpr auto Size() noexcept
   {
      return ArenaSize;
   }

   auto Used() const noexcept
   {
      return static_cast<std::size_t>(std::distance(GetBufferPointer(), m_pointer));
   }

   void Reset() noexcept
   {
      m_pointer = m_buffer;
   }

private:

   constexpr auto GetBufferPointer() const
   {
      return const_cast<MemoryBlock::pointer_type>(
         reinterpret_cast<MemoryBlock::const_pointer_type>(&m_buffer));
   }

   static constexpr auto RoundUpToNextAlignment(std::size_t requestedSize) noexcept
   {
      return (requestedSize + (Alignment - 1)) & ~(Alignment - 1);
   }

   bool Owns(const MemoryBlock& block) noexcept
   {
      return (block.pointer >= GetBufferPointer()) && (block.pointer < GetBufferPointer() + ArenaSize);
   }

   std::aligned_storage_t<ArenaSize, Alignment> m_buffer;
   MemoryBlock::pointer_type m_pointer;
};

/**
* @brief An `Allocator` concept compliant arena allocator.
*/
template<
   typename DataType,
   std::size_t ArenaSize,
   std::size_t Alignment = alignof(std::max_align_t)
>
class ArenaAllocator
{
public:
   using value_type = DataType;
   using pointer = value_type*;
   using const_pointer = const value_type*;
   using arena_type = MemoryArena<ArenaSize, Alignment>;

   ArenaAllocator(MemoryArena<ArenaSize, Alignment>& arena)
      : m_arena{ arena }
   {
   }

   ArenaAllocator(const ArenaAllocator& other) = default;

   // Needed for the templated copy-constructor:
   template<
      typename OtherDataType,
      std::size_t OtherArenaSize,
      std::size_t OtherAlignment
   >
   friend class ArenaAllocator;

   template<typename OtherDataType>
   ArenaAllocator(const ArenaAllocator<OtherDataType, ArenaSize, Alignment>& other)
      : m_arena{ other.m_arena }
   {
   }

   ArenaAllocator& operator=(const ArenaAllocator&) = delete;

   template<
      typename DataTypeLHS,
      std::size_t ArenaSizeLHS,
      std::size_t AlignmentLHS,
      typename DataTypeRHS,
      std::size_t ArenaSizeRHS,
      std::size_t AlignmentRHS
   >
   friend bool operator==(
      const ArenaAllocator<DataTypeLHS, ArenaSizeLHS, AlignmentLHS>& lhs,
      const ArenaAllocator<DataTypeRHS, ArenaSizeRHS, AlignmentRHS>& rhs);

   template<class OtherDataType>
   struct rebind
   {
      using other = ArenaAllocator<OtherDataType, ArenaSize, Alignment>;
   };

   pointer allocate(std::size_t count)
   {
      const MemoryBlock block = m_arena.Allocate(count * sizeof(DataType));
      return reinterpret_cast<pointer>(block.pointer);
   }

   void deallocate(
      pointer data,
      std::size_t count) noexcept
   {
      const MemoryBlock block
      {
         reinterpret_cast<MemoryBlock::pointer_type>(data),
         count * sizeof(DataType)
      };

      m_arena.Deallocate(block);
   }

private:

   MemoryArena<ArenaSize, Alignment>& m_arena;
};

template<
   typename DataTypeLHS,
   std::size_t ArenaSizeLHS,
   std::size_t AlignmentLHS,
   typename DataTypeRHS,
   std::size_t ArenaSizeRHS,
   std::size_t AlignmentRHS
>
inline bool operator==(
   const ArenaAllocator<DataTypeLHS, ArenaSizeLHS, AlignmentLHS>& lhs,
   const ArenaAllocator<DataTypeRHS, ArenaSizeRHS, AlignmentRHS>& rhs)
{
   return ArenaSizeLHS == ArenaSizeRHS
      && AlignmentLHS == AlignmentRHS
      && &lhs.m_arena == &rhs.m_arena;
}

template<
   typename DataTypeLHS,
   std::size_t ArenaSizeLHS,
   std::size_t AlignmentLHS,
   typename DataTypeRHS,
   std::size_t ArenaSizeRHS,
   std::size_t AlignmentRHS
>
inline bool operator!=(
   const ArenaAllocator<DataTypeLHS, ArenaSizeLHS, AlignmentLHS>& lhs,
   const ArenaAllocator<DataTypeRHS, ArenaSizeRHS, AlignmentRHS>& rhs)
{
   return !(lhs == rhs);
}
