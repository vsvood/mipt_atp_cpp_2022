//
// Created by vsvood on 2/27/22.
//

#ifndef DEQUE__DEQUE_H_
#define DEQUE__DEQUE_H_

template <typename T>
class Deque {
 private:
  static const size_t kSizeOfInnerArray = 512;

  template <bool is_const>
  class CommonIterator;

  T** deque_;
  size_t outer_array_size_;

 public:
  using iterator = CommonIterator<false>;
  using const_iterator = CommonIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

 private:
  iterator very_begin_iterator_;
  iterator very_end_iterator_;
  iterator begin_;
  iterator end_;

 public:
  using size_type = size_t;
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;

  Deque();
  ~Deque();
  Deque(const Deque<value_type>& other);
  explicit Deque(size_type count);
  Deque(size_type count, const_reference value);
  Deque<value_type>& operator=(const Deque<value_type>& other);

  [[nodiscard]] size_type size() const;

  reference operator[](size_type pos);
  const_reference operator[](size_type pos) const;
  reference at(size_type pos);
  const_reference at(size_type pos) const;

  void push_back(const_reference value);
  void pop_back();
  void push_front(const_reference value);
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

  iterator insert(const_iterator pos, const_reference value);
  template< class... Args >
  void emplace_back( Args&&... args );
  iterator erase(const_iterator pos);

 private:
  void resize();
};

template <typename T>
template <bool is_const>
class Deque<T>::CommonIterator {
 public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = T;
  using difference_type = ssize_t;
  using pointer = typename std::conditional<is_const, const T*, T*>::type;
  using reference = typename std::conditional<is_const, const T&, T&>::type;

  CommonIterator();
  CommonIterator(T** outer_pointer, size_t idx);
  CommonIterator(const CommonIterator<is_const>& other);

  CommonIterator<is_const>& operator++();
  CommonIterator<is_const>& operator--();
  CommonIterator<is_const> operator++(int);
  CommonIterator<is_const> operator--(int);

  CommonIterator<is_const>& operator+=(difference_type shift);
  CommonIterator<is_const>& operator-=(difference_type shift);
  CommonIterator<is_const> operator+(difference_type shift) const;
  CommonIterator<is_const> operator-(difference_type shift) const;

  bool operator<(const CommonIterator<true>& other) const;
  bool operator>(const CommonIterator<true>& other) const;
  bool operator<=(const CommonIterator<true>& other) const;
  bool operator>=(const CommonIterator<true>& other) const;
  bool operator==(const CommonIterator<true>& other) const;
  bool operator!=(const CommonIterator<true>& other) const;

  difference_type operator-(const CommonIterator<is_const>& other) const;
  reference operator*();
  pointer operator->();

  operator CommonIterator<true>() const;

  friend class CommonIterator<!is_const>;

 private:
  T** outer_pointer_;
  size_t idx_;
};

template <typename T>
template <bool is_const>
Deque<T>::CommonIterator<is_const>::CommonIterator() = default;

template <typename T>
template <bool is_const>
Deque<T>::CommonIterator<is_const>::CommonIterator(T** outer_pointer,
                                                   size_t idx)
    : outer_pointer_(outer_pointer), idx_(idx) {}

template <typename T>
template <bool is_const>
Deque<T>::CommonIterator<is_const>::CommonIterator(
    const CommonIterator<is_const>& other)
    : outer_pointer_(other.outer_pointer_), idx_(other.idx_) {}

template <typename T>
template <bool is_const>
typename Deque<T>::template CommonIterator<is_const>&
Deque<T>::CommonIterator<is_const>::operator++() {
  ++idx_;
  if (idx_ == kSizeOfInnerArray) {
    ++outer_pointer_;
    idx_ = 0;
  }
  return *this;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template CommonIterator<is_const>&
Deque<T>::CommonIterator<is_const>::operator--() {
  if (idx_ == 0) {
    --outer_pointer_;
    idx_ = kSizeOfInnerArray;
  }
  --idx_;
  return *this;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template CommonIterator<is_const>
Deque<T>::CommonIterator<is_const>::operator++(int) {
  CommonIterator<is_const> tmp = *this;
  ++(*this);
  return tmp;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template CommonIterator<is_const>
Deque<T>::CommonIterator<is_const>::operator--(int) {
  CommonIterator<is_const> tmp = *this;
  --(*this);
  return tmp;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template CommonIterator<is_const>&
Deque<T>::CommonIterator<is_const>::operator+=(difference_type shift) {
  if (shift == 0) {
    return *this;
  }
  if (shift < 0) {
    return *this -= (-shift);
  }
  shift += idx_;

  difference_type outer_shift = shift / kSizeOfInnerArray;
  outer_pointer_ += outer_shift;
  idx_ = shift % kSizeOfInnerArray;

  return *this;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template CommonIterator<is_const>&
Deque<T>::CommonIterator<is_const>::operator-=(difference_type shift) {
  if (shift == 0) {
    return *this;
  }
  if (shift < 0) {
    return *this += (-shift);
  }
  shift = shift + kSizeOfInnerArray - 1 - idx_;

  difference_type outer_shift = shift / kSizeOfInnerArray;
  outer_pointer_ -= outer_shift;
  idx_ = kSizeOfInnerArray - 1 - shift % kSizeOfInnerArray;

  return *this;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template CommonIterator<is_const>
Deque<T>::CommonIterator<is_const>::operator+(difference_type shift) const {
  CommonIterator<is_const> tmp = *this;
  tmp += shift;
  return tmp;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template CommonIterator<is_const>
Deque<T>::CommonIterator<is_const>::operator-(difference_type shift) const {
  CommonIterator<is_const> tmp = *this;
  tmp -= shift;
  return tmp;
}

template <typename T>
template <bool is_const>
bool Deque<T>::CommonIterator<is_const>::operator<(
    const Deque<T>::CommonIterator<true>& other) const {
  return (outer_pointer_ < other.outer_pointer_) ||
         (outer_pointer_ == other.outer_pointer_ && idx_ < other.idx_);
}

template <typename T>
template <bool is_const>
bool Deque<T>::CommonIterator<is_const>::operator>(
    const Deque<T>::CommonIterator<true>& other) const {
  return other < *this;
}

template <typename T>
template <bool is_const>
bool Deque<T>::CommonIterator<is_const>::operator<=(
    const Deque<T>::CommonIterator<true>& other) const {
  return !(*this > other);
}

template <typename T>
template <bool is_const>
bool Deque<T>::CommonIterator<is_const>::operator>=(
    const Deque<T>::CommonIterator<true>& other) const {
  return !(*this < other);
}

template <typename T>
template <bool is_const>
bool Deque<T>::CommonIterator<is_const>::operator==(
    const Deque<T>::CommonIterator<true>& other) const {
  return !((*this < other) || (other < *this));
}

template <typename T>
template <bool is_const>
bool Deque<T>::CommonIterator<is_const>::operator!=(
    const Deque<T>::CommonIterator<true>& other) const {
  return !(*this == other);
}

template <typename T>
template <bool is_const>
typename Deque<T>::template CommonIterator<is_const>::difference_type
Deque<T>::CommonIterator<is_const>::operator-(
    const CommonIterator<is_const>& other) const {
  return (outer_pointer_ - other.outer_pointer_) * kSizeOfInnerArray +
         (idx_ - other.idx_);
}

template <typename T>
template <bool is_const>
typename Deque<T>::template CommonIterator<is_const>::reference
Deque<T>::CommonIterator<is_const>::operator*() {
  return (*outer_pointer_)[idx_];
}

template <typename T>
template <bool is_const>
typename Deque<T>::template CommonIterator<is_const>::pointer
Deque<T>::CommonIterator<is_const>::operator->() {
  return *outer_pointer_ + idx_;
}

template <typename T>
template <bool is_const>
Deque<T>::CommonIterator<is_const>::operator CommonIterator<true>() const {
  return CommonIterator<true>(outer_pointer_, idx_);
}

/////////////////////////////////////////////// DEQUE ////////////////////////

template <typename T>
Deque<T>::Deque()
    : deque_(nullptr),
      outer_array_size_(0),
      very_begin_iterator_(deque_, 0),
      very_end_iterator_(very_begin_iterator_),
      begin_(very_begin_iterator_),
      end_(very_end_iterator_) {}

template <typename T>
Deque<T>::Deque(const Deque<value_type>& other) : Deque() {
  if (other.deque_ == nullptr) {
    return;
  }

  outer_array_size_ = other.outer_array_size_;

  deque_ = new T*[outer_array_size_];
  for (size_t i = 0; i < outer_array_size_; ++i) {
    deque_[i] = static_cast<T*>(operator new(sizeof(T) * kSizeOfInnerArray));
  }

  very_begin_iterator_ = iterator(deque_, 0);
  very_end_iterator_ = iterator(deque_ + outer_array_size_, 0);

  begin_ = very_begin_iterator_ + (other.begin_ - other.very_begin_iterator_);
  end_ = very_begin_iterator_ + (other.end_ - other.very_begin_iterator_);

  auto this_iter = this->begin_;
  auto other_iter = other.begin_;
  while (this_iter < this->end_) {
    *this_iter = *other_iter;
    ++this_iter;
    ++other_iter;
  }
}

template <typename T>
Deque<T>::Deque(size_type count) {
  outer_array_size_ =
      std::max((count + kSizeOfInnerArray - 1) / kSizeOfInnerArray, 2ul);
  deque_ = new T*[outer_array_size_];
  for (size_t i = 0; i < outer_array_size_; ++i) {
    deque_[i] = static_cast<T*>(operator new(sizeof(T) * kSizeOfInnerArray));
  }

  very_begin_iterator_ = iterator(deque_, 0);
  very_end_iterator_ = iterator(deque_ + outer_array_size_, 0);
  begin_ = iterator(deque_, 0);
  end_ = begin_ + count;
  for (auto& elem : *this) {
    elem = T();
  }
}

template <typename T>
Deque<T>::Deque(size_type count, const_reference value) {
  outer_array_size_ =
      std::max((count + kSizeOfInnerArray - 1) / kSizeOfInnerArray, 2ul);
  deque_ = new T*[outer_array_size_];
  for (size_t i = 0; i < outer_array_size_; ++i) {
    deque_[i] = static_cast<T*>(operator new(sizeof(T) * kSizeOfInnerArray));
  }

  very_begin_iterator_ = iterator(deque_, 0);
  very_end_iterator_ = iterator(deque_ + outer_array_size_, 0);
  begin_ = iterator(deque_, 0);
  end_ = begin_ + count;
  for (auto& elem : *this) {
    elem = value;
  }
}

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque<value_type>& other) {
  if (deque_ == other.deque_) {
    return *this;
  }
  if (other.deque_ == nullptr) {
    return *this = Deque<T>();
  }
  T** deque_tmp = deque_;
  auto outer_array_size_tmp = outer_array_size_;
  auto very_begin_iterator_tmp = very_begin_iterator_;
  auto very_end_iterator_tmp = very_end_iterator_;
  auto begin_tmp = begin_;
  auto end_tmp = end_;

  outer_array_size_ = other.outer_array_size_;

  deque_ = new T*[outer_array_size_];
  for (size_t i = 0; i < outer_array_size_; ++i) {
    deque_[i] = static_cast<T*>(operator new(sizeof(T) * kSizeOfInnerArray));
  }

  very_begin_iterator_ = iterator(deque_, 0);
  very_end_iterator_ = iterator(deque_ + outer_array_size_, 0);
  begin_ = very_begin_iterator_ + (other.begin_ - other.very_begin_iterator_);
  end_ = very_begin_iterator_ + (other.end_ - other.very_begin_iterator_);

  auto this_iter = this->begin_;
  auto other_iter = other.begin_;
  while (this_iter < this->end_) {
    try {
      *this_iter = *other_iter;
    } catch (...) {
      for (auto iter = this->begin_; iter < this_iter; ++iter) {
        iter->~T();
      }
      for (size_t i = 0; i < outer_array_size_; ++i) {
        operator delete[](deque_[i]);
      }
      delete[] deque_;

      deque_ = deque_tmp;
      outer_array_size_ = outer_array_size_tmp;
      very_begin_iterator_ = very_begin_iterator_tmp;
      very_end_iterator_ = very_end_iterator_tmp;
      begin_ = begin_tmp;
      end_ = end_tmp;
      throw;
    }
    ++this_iter;
    ++other_iter;
  }

  while (begin_tmp != end_tmp) {
    begin_tmp->~T();
    ++begin_tmp;
  }

  for (size_t i = 0; i < outer_array_size_tmp; ++i) {
    operator delete[](deque_tmp[i]);
  }
  delete[] deque_tmp;

  return *this;
}

template <typename T>
typename Deque<T>::size_type Deque<T>::size() const {
  return end_ - begin_;
}

template <typename T>
typename Deque<T>::reference Deque<T>::operator[](size_type pos) {
  return *(begin_ + pos);
}

template <typename T>
typename Deque<T>::const_reference Deque<T>::operator[](size_type pos) const {
  return *(begin_ + pos);
}

template <typename T>
typename Deque<T>::reference Deque<T>::at(size_type pos) {
  if (size() <= pos) {
    throw std::out_of_range("out of range");
  }

  return *(begin_ + pos);
}

template <typename T>
typename Deque<T>::const_reference Deque<T>::at(size_type pos) const {
  if (size() <= pos) {
    throw std::out_of_range("out of range");
  }

  return *(begin_ + pos);
}

template <typename T>
void Deque<T>::push_back(const_reference value) {
  if (end_ == very_end_iterator_) {
    resize();
  }
  *end_ = value;
  ++end_;
}

template <typename T>
void Deque<T>::pop_back() {
  end_->~T();
  --end_;
}

template <typename T>
void Deque<T>::push_front(const_reference value) {
  if (begin_ == very_begin_iterator_) {
    resize();
  }
  --begin_;
  try {
    *begin_ = value;
  } catch (...) {
    ++begin_;
    throw;
  }
}

template <typename T>
void Deque<T>::pop_front() {
  begin_->~T();
  ++begin_;
}

template <typename T>
typename Deque<T>::iterator Deque<T>::begin() {
  return begin_;
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::begin() const {
  return begin_;
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const {
  return begin_;
}

template <typename T>
typename Deque<T>::iterator Deque<T>::end() {
  return end_;
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::end() const {
  return end_;
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cend() const {
  return end_;
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rbegin() {
  return reverse_iterator(end_);
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rbegin() const {
  return const_reverse_iterator(end_);
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crbegin() const {
  return const_reverse_iterator(end_);
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rend() {
  return reverse_iterator(begin_);
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rend() const {
  return const_reverse_iterator(begin_);
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crend() const {
  return const_reverse_iterator(begin_);
}

template <typename T>
typename Deque<T>::iterator Deque<T>::insert(Deque<T>::const_iterator pos,
                                             const_reference value) {
  Deque<T> deque_tmp = *this;
  auto shift = pos - begin_;
  auto iter = deque_tmp.begin_ + shift;
  T tmp = value;
  while (iter < deque_tmp.end_) {
    std::swap(*iter, tmp);
    ++iter;
  }
  deque_tmp.push_back(tmp);
  *this = deque_tmp;
  return begin_ + shift;
}

template <typename T>
typename Deque<T>::iterator Deque<T>::erase(Deque<T>::const_iterator pos) {
  Deque<T> deque_tmp = *this;
  auto shift = pos - begin_;
  auto iter = deque_tmp.begin_ + shift;
  while (iter < deque_tmp.end_ - 1) {
    *iter = *(iter + 1);
    ++iter;
  }
  --deque_tmp.end_;
  *this = deque_tmp;
  return begin_ + shift;
}

template <typename T>
void Deque<T>::resize() {
  if (deque_ != nullptr) {
    auto old_deque = deque_;
    outer_array_size_ *= 2;
    deque_ = new T*[outer_array_size_];

    auto begin_shift = begin_ - very_begin_iterator_;
    auto end_shift = end_ - very_begin_iterator_;

    auto shift = outer_array_size_ / 4;

    for (size_t i = 0; i < shift; ++i) {
      deque_[i] = static_cast<T*>(operator new(sizeof(T) * kSizeOfInnerArray));
    }
    for (size_t i = shift; i < shift + outer_array_size_ / 2; ++i) {
      deque_[i] = old_deque[i - shift];
    }
    for (size_t i = shift + outer_array_size_ / 2; i < outer_array_size_; ++i) {
      deque_[i] = static_cast<T*>(operator new(sizeof(T) * kSizeOfInnerArray));
    }

    delete[] old_deque;

    very_begin_iterator_ = iterator(deque_, 0);
    very_end_iterator_ = iterator(deque_ + outer_array_size_, 0);

    auto relative_begin = iterator(deque_ + shift, 0);
    begin_ = relative_begin + begin_shift;
    end_ = relative_begin + end_shift;
  } else {
    outer_array_size_ = 2;
    deque_ = new T*[outer_array_size_];
    for (size_t i = 0; i < outer_array_size_; ++i) {
      deque_[i] = static_cast<T*>(operator new(sizeof(T) * kSizeOfInnerArray));
    }

    very_begin_iterator_ = iterator(deque_, 0);
    very_end_iterator_ = iterator(deque_ + outer_array_size_, 0);

    begin_ = iterator(deque_ + 1, 0);
    end_ = begin_;
  }
}

template<typename T>
Deque<T>::~Deque() {
  while (begin_ != end_) {
    begin_->~T();
    ++begin_;
  }
  for (size_t i = 0; i < outer_array_size_; ++i) {
    operator delete[](deque_[i]);
  }
  delete deque_;
}

template <typename T>
template <class... Args>
void Deque<T>::emplace_back(Args &&...args) {

}

#endif  // DEQUE__DEQUE_H_
