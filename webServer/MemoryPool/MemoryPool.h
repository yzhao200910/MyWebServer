#pragma once
#define BlockSize 4096
#include "../base/MutexLock.h"
#include <assert.h>
#include <utility>
#include <cstddef>

//using namespace std;


    
 struct Slot{
     Slot* next;
};
// union Slot_{
//      Slot * next

// };

class MemoryPool{
        public:
            MemoryPool();
            ~MemoryPool();


            void init(int size);

            Slot* allocate();
            void deAllocate(Slot* p);
        private:
        // union Slot
        // {
           
        // };
        
            int slotSize_;//每个小内存所占的字节

            Slot* currentBlock_;//内存块的首地址
            Slot* currentSlot_;
            Slot* lastSlot_;//可存放元素的最后指针
            Slot* freeSlot_;//用于释放
            MutexLock mutex_freeSlot_;
            MutexLock mutex_other_;

            size_t padPointer(char* p,size_t align);//计算对齐所需空间
            Slot* allocateBlock(); //申请新的内存放进内存池
            Slot* nofree_slove();

        };

        void init_MemoryPool();
        void* use_Memory(size_t size);
        void free_Memory(size_t size,void* p);
        MemoryPool& get_MemoryPool(int id);

        template<typename T,typename... Args>
        T* newElement(Args&&... args){
            T* p;
            if((p=reinterpret_cast<T *>(use_Memory(sizeof(T))))!=nullptr)
                new(p) T(std::forward<Args>(args)...); //完美转发
            return p;
        }

        template<typename T>
        void deleteElement(T* p){
            if(p)
                p->~T();
            free_Memory(sizeof(T),reinterpret_cast<void *>(p));
            printf("deleteElement sucesss\n");
        }


