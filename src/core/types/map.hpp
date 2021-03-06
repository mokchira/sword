#ifndef TYPES_MAP_HPP
#define TYPES_MAP_HPP

#include <optional>
#include <vector>
#include <bitset>


namespace sword
{

template <typename S, typename T>
class SmallMap
{
public:
    using Element = std::pair<S, T>;
    SmallMap() = default;
    SmallMap(std::initializer_list<Element> avail) : options{avail} {}
//    inline std::optional<T> findOption(CommandLineEvent* event) const
//    {
//        std::string input = event->getInput();
//        std::stringstream instream{input};
//        instream >> input;
//        return findOption(input);
//    }
    std::optional<T> findValue(const S& s) const 
    {
        for (const auto& item : options) 
            if (item.first == s)
                return item.second;
        return {};
    }

    template <size_t N>
    std::optional<T> findValue(const S& s, std::bitset<N> mask) const 
    {
        for (int i = 0; i < options.size(); i++) 
            if (mask[options[i].second] && options[i].first == s)
                return options[i].second;
        return {};
    }

    std::optional<Element> findElement(const S& s, const T& t)
    {
        for (const auto& item : options) 
            if (item.first == s && item.second == t)
                return item;
        return {};
    }

    void remove(T t)
    {
        size_t size = options.size();
        for (int i = 0; i < size; i++) 
           if (options[i].second == t)
               options.erase(options.begin() + i); 
    }

    //TODO: we need to extend the map to the specific case of <string, option> 
    //      and make sure that this function in that setting returns the strings in order
    std::vector<S> getKeys() const
    {
        std::vector<S> vec;
        vec.reserve(options.size());
        for (const auto& item : options) 
            vec.push_back(item.first);
        return vec;
    }

    template <size_t N>
    std::vector<S> getKeys(std::bitset<N> mask) const
    {
        std::vector<S> vec;
        vec.reserve(options.size());
        for (int i = 0; i < options.size(); i++) 
        {
            if (mask[i])
                vec.push_back(options[i].first);
        }
        return vec;
    }

    S keyAt(int i) const { return options.at(i).first; }

    T valueAt(int i) const { return options.at(i).second; }
    
    void push(Element element)
    {
        options.push_back(element);
    }

    auto begin()
    {
        return options.begin();
    }

    auto end()
    {
        return options.end();
    }

    size_t size() const { return options.size(); }

private:
    std::vector<Element> options;
};



}; //sword


#endif /* end of include guard: TYPES_MAP_HPP */
