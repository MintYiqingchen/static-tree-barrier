//
// Created by mintyi on 4/23/20.
//
#ifndef TEST_MYBARRIER_HPP
#define TEST_MYBARRIER_HPP
#include <vector>
#include <stdio.h>
#include <atomic>
class Barrier {
public:
    virtual void await(int thread_id) = 0;
};

class StaticTreeBarrier: public Barrier {
    class Node {
        StaticTreeBarrier* tree;
        std::atomic<int> count;
        int children;
        Node* parent;
        bool threadSense; // volatile?
    public:
        Node(StaticTreeBarrier* tree, int c, Node* p = nullptr): parent(p){
            this->tree = tree;
            count = c;
            children = c;
            threadSense = !tree->globalSense;
        }
        void await() {
            while(count.load(std::memory_order_acquire) > 0) {};
            count.store(children, std::memory_order_relaxed);
            if(parent != nullptr) {
                parent->childDone();
                while(threadSense != tree->globalSense){};
            } else {
                tree->globalSense = !tree->globalSense;
            }
            threadSense = !threadSense;
        }
        void childDone() {
            count.fetch_sub(1, std::memory_order_relaxed);
        }
    };

    int radix, size;
    bool globalSense;
    std::vector<Node*> nodes;
public:
    StaticTreeBarrier(int size, int radix) {
        // MODEL_ASSERT(size > 0);
        this->radix = radix;
        globalSense = false;
        this->size = size;
        build();
        // MODEL_ASSERT(nodes.size() == this->size);
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
