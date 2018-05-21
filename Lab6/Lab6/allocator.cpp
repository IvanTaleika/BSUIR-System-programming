#include "allocator.h"

Allocator::Allocator() {
  memoryAllocationStart = memoryAllocationEnd = sbrk(0);
}

Allocator::~Allocator() {
  brk(memoryAllocationStart);
}

void Allocator::free(void* ptr) {
  MemoryChunkData* currentChunk = (MemoryChunkData*)((char*)ptr - sizeof(MemoryChunkData));
  currentChunk->isAvalible = true;
  mergeWithNext(currentChunk);
  mergeWithPrevious(currentChunk);
}

void* Allocator::calloc(size_t size) {
  char* ptr = (char*) malloc(size);
  for (size_t i = 0; i < size; i++) {
    ptr[i] = 0;
  }
  return (void*)ptr;
}

void* Allocator::malloc(size_t size) {
  MemoryChunkData* currentChunk;
  size_t newChunkSize = size + additionalMemory;
  char* currentPos = (char*)memoryAllocationStart;
  while (true) {
    if (currentPos == memoryAllocationEnd) {
      if (sbrk(newChunkSize) == (void*) -1) {
        return nullptr;
      }
      memoryAllocationEnd = (void*)((char*)memoryAllocationEnd + newChunkSize);
      currentChunk = (MemoryChunkData*)currentPos;

      currentChunk->size = newChunkSize;
      size_t* currentChunkEndSizeValue = (size_t*)((char*)currentChunk + newChunkSize - sizeof(size_t));
      *currentChunkEndSizeValue = newChunkSize;
      break;
    }
    currentChunk = (MemoryChunkData*)currentPos;
    if (currentChunk->isAvalible && currentChunk->size >= newChunkSize) {
      if (currentChunk->size > newChunkSize) {
        MemoryChunkData* nextChunk = (MemoryChunkData*)(currentPos + currentChunk->size);
        if (nextChunk != memoryAllocationEnd && nextChunk->isAvalible) {
          MemoryChunkData* secondChunk = splitChunk(currentChunk, newChunkSize);
          mergeWithNext(secondChunk);
          break;
        }
        MemoryChunkData* previousChunk = (MemoryChunkData*)(currentPos - (size_t) * (currentPos - sizeof(size_t)));
        if (currentChunk != memoryAllocationStart && previousChunk->isAvalible) {
          MemoryChunkData* secondChunk = splitChunk(currentChunk, newChunkSize);
          mergeWithNext(currentChunk);
          currentChunk = secondChunk;
          break;
        }
        if (currentChunk->size - newChunkSize > additionalMemory) {
          splitChunk(currentChunk, newChunkSize);
        }
      }
      break;
    }
    currentPos += currentChunk->size;
  }
  currentChunk->isAvalible = false;
  return (void*)((char*)currentChunk + sizeof(MemoryChunkData));
}

void* Allocator::realloc(void* ptr, size_t newSize) {
  if (ptr == nullptr) {
    return malloc(newSize);
  }
  if (newSize == 0) {
    free(ptr);
    return nullptr;
  }
  void* newPtr = malloc(newSize);
  MemoryChunkData* oldChunk = (MemoryChunkData*)((char*)ptr - sizeof(MemoryChunkData));
  size_t copySize = (oldChunk->size - sizeof(MemoryChunkData)) > newSize ? newSize : (oldChunk->size - sizeof(MemoryChunkData));
  memcpy(newPtr, ptr, copySize);
  free(ptr);
  return newPtr;
}

void Allocator::mergeWithNext(Allocator::MemoryChunkData* currentChunk) {
  MemoryChunkData* nextChunk = (MemoryChunkData*)((char*)currentChunk + currentChunk->size);
  if (nextChunk != memoryAllocationEnd) {
    if (nextChunk->isAvalible) {
      size_t* nextChunkEndSizeValue = (size_t*)((char*)nextChunk + nextChunk->size - sizeof(size_t));
      *nextChunkEndSizeValue = nextChunk->size + currentChunk->size;
      currentChunk->size = *nextChunkEndSizeValue;
    }
  }
}

void Allocator::mergeWithPrevious(Allocator::MemoryChunkData* currentChunk) {
  if (currentChunk != memoryAllocationStart) {
    MemoryChunkData* previousChunk = (MemoryChunkData*)((char*)currentChunk - (size_t) * ((char*)currentChunk - sizeof(size_t)));
    if (previousChunk->isAvalible) {
      previousChunk->size = currentChunk->size + previousChunk->size;
      size_t* currentChunkEndSizeValue = (size_t*)((char*)currentChunk + currentChunk->size - sizeof(size_t));
      *currentChunkEndSizeValue = previousChunk->size;
    }
  }
}

Allocator::MemoryChunkData* Allocator::splitChunk(Allocator::MemoryChunkData* currentChunk, size_t firstChunkSize) {
  MemoryChunkData* secondChunk = (MemoryChunkData*)((char*)currentChunk + firstChunkSize);
  secondChunk->isAvalible = true;
  secondChunk->size = currentChunk->size - firstChunkSize;
  size_t* reducedChunkEndSizeValue = (size_t*)((char*)secondChunk + secondChunk->size - sizeof(size_t));
  *reducedChunkEndSizeValue = secondChunk->size;

  currentChunk->size = firstChunkSize;
  size_t* currentChunkEndSizeValue = (size_t*)((char*)currentChunk + firstChunkSize - sizeof(size_t));
  *currentChunkEndSizeValue = firstChunkSize;
  return secondChunk;

}
