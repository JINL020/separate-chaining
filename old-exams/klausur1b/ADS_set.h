#ifndef ADS_SET_H
#define ADS_SET_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>

// type name Key is the typ of keys the set takes
// size_t N is the starting size of the ADS_set
// both these parameters are specified at compile time
template <typename Key, size_t N = 9>
class ADS_set {
   public:
    class Iterator;
    //using alias
    using value_type = Key;
    using key_type = Key;
    using reference = key_type &;
    using const_reference = const key_type &;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = Iterator;
    using const_iterator = Iterator;
    //using key_compare = std::less<key_type>;    // B+-Tree
    using key_equal = std::equal_to<key_type>;  // Hashing
    using hasher = std::hash<key_type>;         // Hashing
   private:
    struct Entry;       //Wieso?
    struct List;        //wieso?
    size_type buckets;  // number of max buckets
    size_type current_size;
    List *values;  //pointer to first List array
    key_type max1;
    key_type max2;
    bool define1 = false;
    bool define2 = false;
    bool ok = false;

   public:
    // constructors
    ADS_set() : buckets{N}, current_size{0}, values{new List[N]} {}  // PH1

    ADS_set(std::initializer_list<key_type> ilist) : ADS_set{std::begin(ilist), std::end(ilist)} {}  // PH1

    template <typename InputIt>
    ADS_set(InputIt first, InputIt last) : ADS_set{} {  // PH1
        this->insert(first, last);
    }

    //copy constructor
    ADS_set(const ADS_set &other) : ADS_set(other.begin(), other.end()) {  //PH2
    }

    // destructor
    ~ADS_set() {
        delete[] values;
    }

    // assignment operators
    ADS_set &operator=(const ADS_set &other) {
        if (this == &other)
            return *this;
        ADS_set temp{other};
        swap(temp);
        if (current_size > 1) {
            ok = true;
        }
        for (auto it{other.begin()}; it != other.end(); ++it) {
            if (!define1) {
                max1 = *it;
                define1 = true;
            } else if (!define2 && !key_equal{}(max1, *it)) {
                max2 = *it;
                define2 = true;
            } else if (define1 && define2) {
                if (std::less<key_type>{}(max2, *it) && !key_equal{}(max1, *it)) {
                    max2 = *it;
                }
                if (std::less<key_type>{}(max1, *it)) {
                    max2 = max1;
                    max1 = *it;
                }
            }
        }
        return *this;
    }

    ADS_set &operator=(std::initializer_list<key_type> ilist) {
        ADS_set tmp{ilist};
        swap(tmp);
        if (current_size > 1) {
            ok = true;
        }
        for (auto it{std::begin(ilist)}; it != std::end(ilist); ++it) {
            if (!define1) {
                max1 = *it;
                define1 = true;
            } else if (!define2 && !key_equal{}(max1, *it)) {
                max2 = *it;
                define2 = true;
            } else if (define1 && define2) {
                if (std::less<key_type>{}(max2, *it) && !key_equal{}(max1, *it)) {
                    max2 = *it;
                }
                if (std::less<key_type>{}(max1, *it)) {
                    max2 = max1;
                    max1 = *it;
                }
            }
        }
        return *this;
    }

    //size and empty
    size_type size() const {  // PH1
        return current_size;
    }

    bool empty() const {  // PH1
        return !current_size;
    }

    // inserts
    void insert(std::initializer_list<key_type> ilist) {  // PH1
        this->insert(std::begin(ilist), std::end(ilist));
    }

    std::pair<iterator, bool> insert(const key_type &key) {
        if (!count(key)) {
            ++current_size;
            expand();
            values[hash(key)].add_Entry(key);
            if (current_size > 1) {
                ok = true;
            }
            if (!define1) {
                max1 = key;
                define1 = true;
            } else if (!define2 && !key_equal{}(max1, key)) {
                max2 = key;
                if (std::less<key_type>{}(max1, key)) {
                    max2 = max1;
                    max1 = key;
                }
                define2 = true;
            } else if (define1 && define2) {
                if (std::less<key_type>{}(max2, key) && !key_equal{}(max1, key)) {
                    max2 = key;
                }
                if (std::less<key_type>{}(max1, key)) {
                    max2 = max1;
                    max1 = key;
                }
            }
            return std::pair<iterator, bool>(find(key), true);
        }
        return std::pair<iterator, bool>(find(key), false);
    }

    template <typename InputIt>
    void insert(InputIt first, InputIt last) {  // PH1
        for (auto elem_it{first}; elem_it != last; ++elem_it) {
            if (!count(*elem_it)) {
                ++current_size;
                expand();
                values[hash(*elem_it)].add_Entry(*elem_it);
                if (current_size > 1) {
                    ok = true;
                }
                if (!define1) {
                    max1 = *elem_it;
                    define1 = true;
                } else if (!define2 && !key_equal{}(max1, *elem_it)) {
                    max2 = *elem_it;
                    if (std::less<key_type>{}(max1, *elem_it)) {
                        max2 = max1;
                        max1 = *elem_it;
                    }
                    define2 = true;
                } else if (define1 && define2) {
                    if (std::less<key_type>{}(max2, *elem_it) && !key_equal{}(max1, *elem_it)) {
                        max2 = *elem_it;
                    }
                    if (std::less<key_type>{}(max1, *elem_it)) {
                        max2 = max1;
                        max1 = *elem_it;
                    }
                }
            }
        }
    }

    // returns hashcode
    size_type
    hash(const key_type &key) const {
        return hasher{}(key) % buckets;
    }

    void expand() {
        if (current_size > buckets) {
            buckets *= 3;
            List *expanded{new List[buckets]};
            //divide becuase we need the old bucket number
            for (size_type i{0}; i < buckets / 3; ++i) {
                Entry *temp{values[i].head};
                while (temp != nullptr) {
                    expanded[hash(temp->data)].add_Entry(temp->data);
                    temp = temp->next;
                }
            }
            delete[] values;
            values = expanded;
        }
    }

    // clear and erase
    void clear() {
        ADS_set temp;
        swap(temp);
    }

    size_type erase(const key_type &key) {
        size_type hashKey{hash(key)};
        Entry *current{values[hashKey].head};
        Entry *previous{nullptr};
        if (current == nullptr)
            return false;
        if (key_equal{}(current->data, key)) {
            values[hashKey].head = current->next;
            delete current;
            --current_size;
            return true;
        }
        while (!key_equal{}(current->data, key)) {
            previous = current;
            current = current->next;
            if (current == nullptr)
                return false;
        }
        previous->next = current->next;
        delete current;
        --current_size;
        return true;
    }

    size_type count(const key_type &key) const {  // PH1
        Entry *temp{values[hash(key)].head};
        while (temp != nullptr) {
            if (key_equal{}(temp->data, key)) {
                return 1;
            }
            temp = temp->next;
        }
        return 0;
    }

    iterator find(const key_type &key) const {
        size_type hashKey{hash(key)};
        Entry *temp{values[hashKey].head};
        while (temp != nullptr) {
            if (key_equal{}(temp->data, key)) {
                return Iterator(values + hashKey, values + buckets, temp);
            }
            temp = temp->next;
        }
        return end();
    }

    void swap(ADS_set &other) {
        using std::swap;
        swap(values, other.values);
        swap(current_size, other.current_size);
        swap(buckets, other.buckets);
    }

    const_iterator begin() const {
        for (size_type i{0}; i < buckets; ++i) {
            if (values[i].head != nullptr) {
                return const_iterator{values + i, values + buckets, values[i].head};
            }
        }
        return const_iterator{values + buckets, values + buckets};
    }

    const_iterator end() const {
        return const_iterator{values + buckets, values + buckets};
    }

    void dump(std::ostream &o = std::cerr) const {
        o << "\n\nSEPARATE CHAINING\n\n";
        o << "max1 is " << max1 << "\n";
        o << "max2 is " << max2 << "\n";
        o << "define1 is " << define1 << "\n";
        o << "define2 is " << define2 << "\n";
        for (size_type i{0}; i < buckets; ++i) {
            o << "Bucket " << i << "\n";
            Entry *temp{values[i].head};
            while (temp != nullptr) {
                o << " --> " << temp->data;
                temp = temp->next;
            }
            o << "\n\n\n\n";
        }
    }

    friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
        if (lhs.current_size != rhs.current_size)
            return false;
        for (const auto &key : rhs) {
            if (!lhs.count(key))
                return false;
        }
        return true;
    }

    friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) {
        return !(lhs == rhs);
    }

    key_type y() const {
        if (!ok) {
            throw std::runtime_error("less than 2 elem");
        }
        return max2;
    }
};  // end of ADS_set class

template <typename Key, size_t N>
struct ADS_set<Key, N>::Entry {
    key_type data;
    Entry *next;
    Entry(key_type data, Entry *next) : data{data}, next{next} {}
};

template <typename Key, size_t N>
struct ADS_set<Key, N>::List {
    Entry *head;
    Entry *tail;

    List() : head{nullptr}, tail{nullptr} {}
    ~List() {
        Entry *toDelete{head};
        while (toDelete != nullptr) {
            Entry *next{toDelete->next};
            delete toDelete;
            toDelete = next;
        }
    }

    // adds an Entry to the front of the List
    void add_Entry(const key_type &value) {
        Entry *temp{new Entry(value, nullptr)};
        if (head == nullptr) {
            head = temp;
            tail = temp;
        } else {
            temp->next = head;
            head = temp;
        }
    }
};

template <typename Key, size_t N>
class ADS_set<Key, N>::Iterator {
   public:
    using value_type = Key;
    using difference_type = std::ptrdiff_t;
    using reference = const value_type &;
    using pointer = const value_type *;
    using iterator_category = std::forward_iterator_tag;

   private:
    List *current;
    List *end;
    Entry *entry;

   public:
    explicit Iterator(List *current, List *end, Entry *entry = nullptr) : current{current}, end{end}, entry{entry} {
    }

    Iterator() : current{nullptr}, end{nullptr}, entry{nullptr} {
    }

    reference operator*() const {
        return entry->data;
    }

    pointer operator->() const {
        return &(entry->data);
    }

    //präfix!!!!!!!!!!!!!!!!!!!!!!
    Iterator &operator++() {
        if (current != end && entry->next != nullptr) {
            entry = entry->next;
        } else {
            ++current;
            while (current != end && current->head == nullptr) {
                ++current;
            }
            entry = (current == end ? nullptr : current->head);
        }
        return *this;
    }

    //postfix
    Iterator operator++(int) {
        auto temp(*this);
        ++*this;
        return temp;
    }

    friend bool operator==(const Iterator &lhs, const Iterator &rhs) {
        return lhs.operator->() == rhs.operator->();
    }

    friend bool operator!=(const Iterator &lhs, const Iterator &rhs) {
        return !(lhs == rhs);
    }
};  // end of Iterator class

template <typename Key, size_t N>
void swap(ADS_set<Key, N> &lhs, ADS_set<Key, N> &rhs) { lhs.swap(rhs); }

#endif  // ADS_SET_H
