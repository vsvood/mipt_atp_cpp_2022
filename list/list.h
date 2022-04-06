//
// Created by vsvood on 05.04.2022.
//

#include <chrono>
#include <stdexcept>
#include <string>
#include <list>
#include <vector>
#include <deque>
#include <memory>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cassert>
#include <sys/resource.h>

#include <cstddef>

template <size_t N>
class StackStorage {
 public:
  StackStorage();
  StackStorage(const StackStorage& other) = delete;
  StackStorage& operator=(const StackStorage& other) = delete;
  template <typename T>
  T *allocate(size_t n);
  void deallocate([[maybe_unused]]const void* p, [[maybe_unused]]size_t n);
 private:
  size_t size_ = N;
  char storage_[N];
  void* top_ = &storage_;
};

template<size_t N>
StackStorage<N>::StackStorage() {}

template<size_t N>
template<typename T>
T *StackStorage<N>::allocate(size_t n) {
  auto ret = static_cast<T*>(std::align(alignof(T), sizeof(T)*n, top_, size_));
  top_ = static_cast<T*>(top_) + n;
  size_ = N + storage_ - static_cast<char*>(top_);
  return ret;
}

template<size_t N>
void StackStorage<N>::deallocate([[maybe_unused]]const void *p, [[maybe_unused]]size_t n) {
  //if (static_cast<const char*>(p) + n == )
}

template <typename T, size_t N>
class StackAllocator {
 public:
  using value_type = T;
  using pointer = T*;
  template<typename U>
  struct rebind {
    //using other = StackAllocator<U, N>;
    typedef StackAllocator<U, N> other;
  };

  StackAllocator(StackStorage<N>& storage);
  template<typename U>
  StackAllocator(const StackAllocator<U, N>& other);
  ~StackAllocator();
  template<typename U>
  StackAllocator& operator=(const StackAllocator<U, N>& other);

  pointer allocate(size_t n);
  void deallocate(pointer p, size_t n);

  template<typename U, size_t M>
  bool operator==(const StackAllocator<U, M>& other) const;

  template<typename U, size_t M>
  bool operator!=(const StackAllocator<U, M>& other) const;

  private:
   template<typename U, size_t M>
   friend class StackAllocator;

   StackStorage<N>* storage_{};
};

template<typename T, size_t N>
StackAllocator<T, N>::StackAllocator(StackStorage<N> &storage) : storage_(&storage) {
}

template<typename T, size_t N>
template<typename U>
StackAllocator<T, N>::StackAllocator(const StackAllocator<U, N> &other) : storage_(other.storage_) {
}

template<typename T, size_t N>
StackAllocator<T, N>::~StackAllocator() = default;

template<typename T, size_t N>
template<typename U>
StackAllocator<T, N> &StackAllocator<T, N>::operator=(const StackAllocator<U, N> &other) {
  storage_ = other.storage_;
  return *this;
}

template<typename T, size_t N>
typename StackAllocator<T, N>::pointer StackAllocator<T, N>::allocate(size_t n) {
  return storage_->template allocate<T>(n);
}

template<typename T, size_t N>
void StackAllocator<T, N>::deallocate(StackAllocator::pointer p, size_t n) {
  storage_->deallocate(p, n);
}

template<typename T, size_t N>
template<typename U, size_t M>
bool StackAllocator<T, N>::operator==(const StackAllocator<U, M> &other) const {
  return storage_ == other.storage_;
}
template<typename T, size_t N>
template<typename U, size_t M>
bool StackAllocator<T, N>::operator!=(const StackAllocator<U, M> &other) const {
  return storage_ != other.storage_;
}




#ifndef LIST__LIST_H_
#define LIST__LIST_H_

template<typename T, typename Allocator = std::allocator<T>>
class List {
 private:
  template <bool is_const>
  class CommonIterator;

 public:
  using iterator = CommonIterator<false>;
  using const_iterator = CommonIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using allocator_type = Allocator;

  List();
  List(size_t n);
  List(size_t n, const T& value);
  List(Allocator allocator);
  List(size_t n, Allocator allocator);
  List(size_t n, const T& value, Allocator allocator);
  List(const List& other);
  ~List();

  List& operator=(const List& other);

  void push_back(const T& value);
  void push_front(const T& value);
  void pop_back();
  void pop_front();

  iterator begin();
  const_iterator begin() const;
  const_iterator cbegin() const;
  iterator end();
  const_iterator end() const;
  const_iterator cend() const;

  reverse_iterator rbegin();
  const_reverse_iterator rbegin() const;
  const_reverse_iterator crbegin() const;
  reverse_iterator rend();
  const_reverse_iterator rend() const;
  const_reverse_iterator crend() const;

  iterator insert(const_iterator pos, const T& value);
  typename List<T, Allocator>::iterator insert(List::const_iterator pos);
  iterator erase(const_iterator pos);


  size_t size() const;
  allocator_type get_allocator() const;

 private:
  struct Node;

  size_t size_ = 0;
  iterator begin_;
  iterator end_;
  using inner_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
  inner_allocator_type allocator_;
};

template<typename T, typename Allocator>
struct List<T, Allocator>::Node {
  Node() = default;
  Node(const T& value) : value(value) {}
  T value;
  Node* prev;
  Node* next;
};

template<typename T, typename Allocator>
template <bool is_const>
class List<T, Allocator>::CommonIterator {
 public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = T;
  using difference_type = ssize_t;
  using pointer = typename std::conditional<is_const, const T*, T*>::type;
  using reference = typename std::conditional<is_const, const T&, T&>::type;

  CommonIterator();
  explicit CommonIterator(Node* node);
  CommonIterator(const CommonIterator<is_const>& other);

  CommonIterator<is_const>& operator++();
  CommonIterator<is_const>& operator--();
  CommonIterator<is_const> operator++(int);
  CommonIterator<is_const> operator--(int);

  bool operator==(const CommonIterator<true>& other) const;
  bool operator!=(const CommonIterator<true>& other) const;

  reference operator*();
  pointer operator->();

  void SetNext(Node* next) const {
    node_->next = next;
  }
  void SetPrev(Node* prev) const {
    node_->prev = prev;
  }
  Node* GetNode() const {
    return node_;
  }

  operator CommonIterator<true>() const;

  friend class CommonIterator<!is_const>;

 private:
  Node* node_;
};
template<typename T, typename Allocator>
template<bool is_const>
List<T, Allocator>::CommonIterator<is_const>::operator CommonIterator<true>() const {
  return const_iterator(this->GetNode());
}

template<typename T, typename Allocator>
template<bool is_const>
List<T, Allocator>::CommonIterator<is_const>::CommonIterator()  = default;

template<typename T, typename Allocator>
template<bool is_const>
List<T, Allocator>::CommonIterator<is_const>::CommonIterator(Node *node) : node_(node) {
}

template<typename T, typename Allocator>
template<bool is_const>
List<T, Allocator>::CommonIterator<is_const>::CommonIterator(const CommonIterator<is_const> &other) :node_(other.node_) {
}

template<typename T, typename Allocator>
template<bool is_const>
typename List<T, Allocator>::template CommonIterator<is_const> &List<T, Allocator>::CommonIterator<is_const>::operator++() {
  node_ = node_->next;
  return *this;
}

template<typename T, typename Allocator>
template<bool is_const>
typename List<T, Allocator>::template CommonIterator<is_const> &List<T, Allocator>::CommonIterator<is_const>::operator--() {
  node_ = node_->prev;
  return *this;
}

template<typename T, typename Allocator>
template<bool is_const>
typename List<T, Allocator>::template CommonIterator<is_const> List<T, Allocator>::CommonIterator<is_const>::operator++(int) {
  auto tmp = *this;
  ++(*this);
  return tmp;
}

template<typename T, typename Allocator>
template<bool is_const>
typename List<T, Allocator>::template CommonIterator<is_const> List<T, Allocator>::CommonIterator<is_const>::operator--(int) {
  auto tmp = *this;
  --(*this);
  return tmp;
}

template<typename T, typename Allocator>
template<bool is_const>
bool List<T, Allocator>::CommonIterator<is_const>::operator==(const CommonIterator<true> &other) const {
  return node_ == other.node_;
}

template<typename T, typename Allocator>
template<bool is_const>
bool List<T, Allocator>::CommonIterator<is_const>::operator!=(const CommonIterator<true> &other) const {
  return node_ != other.node_;
}

template<typename T, typename Allocator>
template<bool is_const>
typename List<T, Allocator>::template CommonIterator<is_const>::reference List<T, Allocator>::CommonIterator<is_const>::operator*() {
  return node_->value;
}

template<typename T, typename Allocator>
template<bool is_const>
typename List<T, Allocator>::template CommonIterator<is_const>::pointer List<T, Allocator>::CommonIterator<is_const>::operator->() {
  return &(node_->value);
}
//FIXME default constructor due to base node
template<typename T, typename Allocator>
List<T, Allocator>::List() {
  auto ptr = allocator_.allocate(1);
  ptr->prev = ptr->next = ptr;
  begin_ = iterator(ptr);
  end_ = begin_;
}

template<typename T, typename Allocator>
List<T, Allocator>::List(size_t n) : List() {
  while (n--) {
    push_back(T());
  }
}

template<typename T, typename Allocator>
List<T, Allocator>::List(size_t n, const T &value) : List() {
  while (n--) {
    push_back(value);
  }
}

template<typename T, typename Allocator>
List<T, Allocator>::List(Allocator allocator) : allocator_(allocator) {
  auto ptr = allocator_.allocate(1);
  ptr->prev = ptr->next = ptr;
  begin_ = iterator(ptr);
  end_ = begin_;
}

template<typename T, typename Allocator>
List<T, Allocator>::List(size_t n, Allocator allocator) : List(allocator) {
  while (n--) {
    insert(end_);
  }
}

template<typename T, typename Allocator>
List<T, Allocator>::List(size_t n, const T &value, Allocator allocator) : List(allocator) {
  while (n--) {
    push_back(value);
  }
}

template<typename T, typename Allocator>
List<T, Allocator>::List(const List &other) : List(std::allocator_traits<inner_allocator_type>::select_on_container_copy_construction(other.allocator_)) {
  for (const auto& elem : other)  {
    push_back(elem);
  }
}

template<typename T, typename Allocator>
List<T, Allocator>::~List() {
  while (size_) {
    pop_back();
  }
}

template<typename T, typename Allocator>
List<T, Allocator> &List<T, Allocator>::operator=(const List &other) {
  auto size_tmp = size_;
  auto begin_tmp = begin_;
  auto end_tmp = end_;
  auto allocator_tmp = allocator_;
  if (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value)
    allocator_ = other.get_allocator();
  size_ = 0;
  auto ptr = allocator_.allocate(1);
  ptr->prev = ptr->next = ptr;
  begin_ = iterator(ptr);
  end_ = begin_;
  try {
    for (const auto& elem : other) {
      push_back(elem);
    }
  } catch (...) {
    this->~List();
    size_ = size_tmp;
    begin_ = begin_tmp;
    end_ = end_tmp;
    allocator_ = allocator_tmp;
    throw;
  }
  while (begin_tmp != end_tmp) {
    std::allocator_traits<inner_allocator_type>::destroy(allocator_tmp, begin_tmp.GetNode());
    ++begin_tmp;
  }
  return *this;
}

template<typename T, typename Allocator>
void List<T, Allocator>::push_back(const T& value) {
  insert(end_, value);
}

template<typename T, typename Allocator>
void List<T, Allocator>::push_front(const T& value) {
  insert(begin_, value);
}

template<typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
  //auto tmp = end_;
  erase(std::prev(end_));
}
template<typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
  erase(begin_);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::begin() {
  return begin_;
}
//Ah, shit and here we go again
template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::begin() const {
  return begin_;
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cbegin() const {
  return begin_;
}

template<typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::end() {
  return end_;
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::end() const {
  return end_;
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cend() const {
  return end();
}

template<typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rbegin() {
  return reverse_iterator(end_);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rbegin() const {
  return List::const_reverse_iterator(end_);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::crbegin() const {
  return List::const_reverse_iterator(end_);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rend() {
  return List::reverse_iterator(begin_);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rend() const {
  return List::const_reverse_iterator(begin_);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::crend() const {
  return List::const_reverse_iterator(begin_);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::insert(List::const_iterator pos, const T &value) {
  auto ptr = allocator_.allocate(1);
  std::allocator_traits<inner_allocator_type>::construct(allocator_, ptr, value);
  //ptr->value = value;
  ++size_;
  ptr->next = pos.GetNode();
  ptr->prev = std::prev(pos).GetNode();
  std::prev(pos).SetNext(ptr);
  pos.SetPrev(ptr);
  if (pos == begin_) {
    begin_ = iterator(ptr);
  }
  return iterator(ptr);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::insert(List::const_iterator pos) {
  auto ptr = allocator_.allocate(1);
  std::allocator_traits<inner_allocator_type>::construct(allocator_, ptr);
  ++size_;
  ptr->next = pos.GetNode();
  ptr->prev = std::prev(pos).GetNode();
  std::prev(pos).SetNext(ptr);
  pos.SetPrev(ptr);
  if (pos == begin_) {
    begin_ = iterator(ptr);
  }
  return iterator(ptr);
}

template<typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::erase(List::const_iterator pos) {
  --size_;
  auto next = std::next(pos);
  auto prev = std::prev(pos);
  next.SetPrev(prev.GetNode());
  prev.SetNext(next.GetNode());
  std::allocator_traits<inner_allocator_type>::destroy(allocator_, pos.GetNode());
  allocator_.deallocate(pos.GetNode(), 1);
  if (pos == begin_) {
    begin_ = iterator(next.GetNode());
  }
  //allocator_.destroy(pos.GetNode());
  return iterator(next.GetNode());
}

template<typename T, typename Allocator>
size_t List<T, Allocator>::size() const {
  return size_;
}

template<typename T, typename Allocator>
typename List<T, Allocator>::allocator_type List<T, Allocator>::get_allocator() const {
  return allocator_;
}

#endif//LIST__LIST_H_

