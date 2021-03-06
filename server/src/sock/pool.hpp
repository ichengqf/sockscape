#ifndef SOSC_POOL_H
#define SOSC_POOL_H

#include <vector>
#include <list>
#include <thread>
#include <mutex>

namespace sosc {
typedef struct {
    // amount of threads to start with (never close)
    int initial_count = 3;
    // starting limit of clients per thread
    int initial_size = 3;
    // amount the limit scales when threshold is passed
    int size_growth = 1;
    
    // any values below marked -1 indicates no limit
    // max amount of clients in a scaled thread
    int max_size = -1;
    // maximum number of threads
    int max_count = -1;
    // maximum number of connections in the pool
    int max_total = -1;
    // excess deviance from threshold necessary to rebalance
    int tolerance = -1;
} poolinfo_t;
    
template<class T>
class Pool {
public:
    Pool();
    Pool(const Pool&) = delete;
    void Configure(const poolinfo_t& info);
    
    void Start();
    bool AddClient(T client);
    int ClientCount();
    inline bool IsOpen() const {
        return this->is_open;
    }
    
    virtual void Stop();
protected:
    virtual bool ProcessClient(T& client) = 0;
private:
    bool IsStackFull(int stackCount) const;
    bool CanAddStack() const;
    
    class Stack {
    public:
        Stack(Pool<T>* pool);
        Stack(const Stack&) = delete;
        
        void Start();
        
        void AddClient(T client);
        int ClientCount();
        inline bool IsOpen() const {
            return this->is_open;
        }
        
        void Stop();
    private:
        void StackThread();
        
        std::thread* thread;
        Pool<T>* pool;
        bool is_open;
        bool is_running;
        
        std::list<T> clients;
        std::mutex clients_mtx;
    };
    
    poolinfo_t info;
    bool is_open;
    
    std::vector<Stack*> stacks;
    
    friend class Stack;
};

template<class T>
Pool<T>::Pool() {
    this->info = poolinfo_t();
}

template<class T>
void Pool<T>::Configure(const poolinfo_t& info) {
    this->info = info;
    this->is_open = false;
}

template<class T>
void Pool<T>::Start() {
    if(this->is_open)
        return;
    
    for(int i = 0; i < this->info.initial_count; ++i) {
        this->stacks.push_back(new Stack(this));
        this->stacks.back()->Start();
    }
    
    this->is_open = true;
}

template<class T>
bool Pool<T>::IsStackFull(int stackCount) const {
    const poolinfo_t *info = &this->info;
    return info->max_size != -1 
        && stackCount < 
            info->initial_size 
                + (info->size_growth 
                    * (this->stacks.size() - info->initial_count))
        && stackCount < info->max_size;
}

template<class T>
bool Pool<T>::CanAddStack() const {
    return this->info.max_count == -1 
        || this->stacks.size() < this->info.max_count;
}

template<class T>
bool Pool<T>::AddClient(T client) {
    if(!this->is_open)
        return false;
    
    if(this->info.max_total != -1)
       if(this->ClientCount() >= this->info.max_total)
           return false;
    
    int lowestCount = -1;
    Stack* lowestStack = nullptr;
    for(const auto& stack : this->stacks) {
        int thisCount;
        if((thisCount = stack->ClientCount()) > lowestCount) {
            lowestCount = thisCount;
            lowestStack = stack;
        }
    }
    
    if(lowestStack != nullptr && !this->IsStackFull(lowestCount))
        lowestStack->AddClient(client);
    else if(this->CanAddStack()) {
        this->stacks.push_back(new Stack(this));
        this->stacks.back()->AddClient(client);
    } else
        return false;
    
    return true;
}

template<class T>
int Pool<T>::ClientCount() {
    if(!this->is_open)
        return 0;
    
    int count = 0;
    for(const auto& stack : this->stacks)
        count += stack->ClientCount();
    return count;
}

template<class T>
void Pool<T>::Stop() {
    if(!this->is_open)
        return;

    for(const auto& stack : this->stacks) {
        stack->Stop();
        delete stack;
    }
    
    this->stacks.clear();
    this->is_open = false;
}

template<class T>
Pool<T>::Stack::Stack(Pool<T>* pool) {
    this->pool = pool;
    this->is_open = false;
    this->is_running = false;
}

template<class T>
void Pool<T>::Stack::Start() {
    if(this->is_open || this->is_running)
        return;
    
    this->is_open = true;
    this->is_running = true;
    this->thread = new std::thread([&]() {
        this->StackThread();
    });
}

template<class T>
void Pool<T>::Stack::AddClient(T client) {
    if(!this->is_open || !this->is_running)
        return;
    
    this->clients_mtx.lock();
    this->clients.push_back(client);
    this->clients_mtx.unlock();
}

template<class T>
int Pool<T>::Stack::ClientCount() {
    if(!this->is_open || !this->is_running)
        return 0;
    
    this->clients_mtx.lock();
    int count = this->clients.size();
    this->clients_mtx.unlock();
    
    return count;
}

template<class T>
void Pool<T>::Stack::StackThread() {
    while(this->is_running) {
        for(auto client  = this->clients.begin();
                 client != this->clients.end();
                 ++client)
        {
            if(!this->is_running)
                break;
            
            this->clients_mtx.lock();
            if(!this->pool->ProcessClient(*client))
                this->clients.erase(client);
            this->clients_mtx.unlock();
        }
    }
}

template<class T>
void Pool<T>::Stack::Stop() {
    if(!this->is_open || !this->is_running)
        return;
    
    this->is_running = false;
    this->thread->join();
    
    delete this->thread;
    this->is_open = false;
}
}

#endif
