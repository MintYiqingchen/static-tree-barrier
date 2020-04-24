//
// Created by mintyi on 4/23/20.
//
#ifndef TEST_MYBARRIER_HPP
#define TEST_MYBARRIER_HPP
#include <vector>
#include <stdio.h>
#include <atomic>
#ifdef WITH_CDS
#include <threads.h>
#include <model-assert.h>
#else
#define MODEL_ASSERT(x) x
#endif
class Barrier {
public:
    virtual void await(int thread_id) = 0;
};

class StaticTreeBarrier: public Barrier {
    class Node {
        std::atomic<int> count; // will be changed by other thread and read by local thread
        const int children;
        Node* parent;
        StaticTreeBarrier* tree;
        bool threadSense; // thread local, don't have to be visible to other thread
    public:
        Node(StaticTreeBarrier* tree, int c, Node* p = nullptr):children(c), parent(p){
            this->tree = tree;
            count = c;
            threadSense = true;
        }
        void await() {
            while(count.load(std::memory_order_acquire) > 0) {};
            count.store(children, std::memory_order_relaxed);
            if(parent != nullptr) {
                parent->childDone();
                while(threadSense != (tree->globalSense.load(std::memory_order_acquire))){};
            } else {
                int a = tree->globalSense.load(std::memory_order_relaxed);
                tree->globalSense.store(!a, std::memory_order_release);
            }
            threadSense = !threadSense;
        }
        void childDone() {
            count.fetch_sub(1, std::memory_order_release);
        }
    };

    const int radix, size;
    std::atomic<bool> globalSense;
    std::vector<Node*> nodes;
public:
    StaticTreeBarrier(int size, int radix):radix(radix), size(size) {
        MODEL_ASSERT(size > 0);
        globalSense = false;
        build();
        MODEL_ASSERT(nodes.size() == this->size);
    }

    void build() { // BFS building
        nodes.resize(this->size);
        for(int i = 0; i < nodes.size(); ++ i){
            int childidx = i * radix + radix;
            int count = (size - 1) >= childidx ? radix : std::max(0, radix - childidx + size - 1);
            auto n = new Node(this, count, nodes[(i - 1)/2]);
            nodes[i] = n;
        }
    }

    virtual void await(int thread_id) {
        nodes[thread_id]->await();
    }

    ~StaticTreeBarrier(){
        for(Node* n: nodes)
            delete n;
    }
};
#endif //TEST_MYBARRIER_HPP
