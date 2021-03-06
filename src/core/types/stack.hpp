#ifndef TYPES_STACK_HPP_
#define TYPES_STACK_HPP_
    
#include <utility>
#include <iostream>
#include <vector>
#include <algorithm>

namespace sword
{

template <class T>
class Stack
{
public:
    friend class Application;
    void push(T&& item) {items.push_back(std::forward<T>(item));}
    void pop() {items.pop_back();}
    T top() const {return items.back();}
    auto topPtr() const {return items.back().get();}
    bool empty() const {return items.empty();}
    void clear() { items.clear(); }
    void print() const {for (const auto& item : items) std::cout << item->getName() << std::endl;}
    size_t size() const {return items.size();}
    void emplace_back(T&& t) { items.emplace_back(std::move(t));}
    void reverse() { std::reverse(items.begin(), items.end()); }
protected:
    std::vector<T> items;
};

template <class T>
class ForwardStack : public Stack<T>
{
public:
    auto begin() {return this->items.begin();}
    auto end() {return this->items.end();}
    auto& at(size_t i) const {return this->items.at(i); }
};

template <class T>
class ReverseStack : public Stack<T>
{
public:
    auto begin() const {return this->items.rbegin();}
    auto end() const {return this->items.rend();}
    auto& at(size_t i) const {return this->items.at(i); }
};

}; //sword

#endif /* end of include guard: STACK_HPP_ */
