#include "MemoryPool.h"
#include <stdlib.h>



MemoryPool::MemoryPool()
     :mutex_freeSlot_(),
      mutex_other_()
    {}
MemoryPool::~MemoryPool(){
    Slot* cur=currentBlock_;
    while(cur){
        Slot* next=cur->next;
        operator delete(reinterpret_cast<void *>(cur));
        cur=next;
    }
}

void MemoryPool::init(int size){
    assert(size>0);
    slotSize_=size;
    currentBlock_=NULL;
    currentSlot_=NULL;
    lastSlot_=NULL;
    freeSlot_=NULL;
}
//计算对齐所需补的空间
inline size_t MemoryPool::padPointer(char* p,size_t align){
    // if(align==0){
    //     perror("slot size is wrong:");
    //     printf("%d",align);
    //     exit(0);
    // }
    size_t result=reinterpret_cast<size_t>(p);
    return ((align-result)%align);
}

Slot* MemoryPool::allocateBlock(){
    char* newBlock=reinterpret_cast<char*>(operator new(BlockSize));

    char* body = newBlock +sizeof(Slot*);
    //补齐所需要的内存
   // slotSize_=4;
    size_t bodyPadding = padPointer(body,static_cast<size_t>(alignof(slotSize_)));
    Slot* useSlot;
    {
        MutexLockGuard lock(mutex_other_);
        //newBlock接到Block链表的头部
        reinterpret_cast<Slot *>(newBlock)->next=currentBlock_;
        currentBlock_=reinterpret_cast<Slot *>(newBlock);
        //为该Block开始的地方加上bodyPadding个char* 空间
        currentSlot_=reinterpret_cast<Slot *>(body+bodyPadding);
        lastSlot_=reinterpret_cast<Slot*>(newBlock+BlockSize-slotSize_+1);
        useSlot=currentSlot_;

        currentSlot_+=(slotSize_>>3);
    
    }
    return useSlot;

}

Slot* MemoryPool::nofree_slove(){
    if(currentSlot_>=lastSlot_)
        return allocateBlock();
    Slot* useSlot;
    {
        MutexLockGuard lock(mutex_other_);
        useSlot=currentSlot_;
        currentSlot_+=(slotSize_>>3);

    }
    return useSlot;

}

Slot * MemoryPool::allocate(){
    if(freeSlot_){
        {
        MutexLockGuard lock(mutex_freeSlot_);
        if(freeSlot_){
            Slot* result=freeSlot_;
            freeSlot_=freeSlot_->next;
            return result;
            }
        }

    }
    return nofree_slove();
}

inline void MemoryPool::deAllocate(Slot* p){
    if(p){
        //将slot加入释放队列
        //MutexLockGuard lock(mutex_freeSlot_);
        p->next=freeSlot_;
        freeSlot_=p;
    }
}

MemoryPool& get_MemoryPool(int id){
    static MemoryPool memorypool_[64];
    return memorypool_[id];
}

void init_MemoryPool(){
    for(int i=0;i<64;i++){
        get_MemoryPool(i).init((i + 1) << 3);
    }
}

void* use_Memory(size_t size){
    if(!size)
        return nullptr;
    if(size>512)
        return operator new(size);
    return reinterpret_cast<void *>(get_MemoryPool(((size + 7) >> 3) - 1).allocate());
}

void free_Memory(size_t size,void* p){
    if(!p) return;
    if(size>512){
        operator delete(p);
        return;
    }
    get_MemoryPool(((size + 7) >> 3) - 1).deAllocate(reinterpret_cast<Slot *>(p));
}