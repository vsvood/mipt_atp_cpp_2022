//
// Created by vsvood on 04.05.22.
//

#ifndef SHARED_PTR__SMART_POINTERS_H_
#define SHARED_PTR__SMART_POINTERS_H_

template<typename T>
class SharedPtr;

class SharedBase {
 protected:
  struct BaseControlBlock {
    BaseControlBlock() : shared_counter(1), weak_counter(0) {};
    BaseControlBlock(size_t shared_cnt, size_t weak_cnt) : shared_counter(shared_cnt), weak_counter(weak_cnt) {};

    virtual void *get_ptr() const {
      return nullptr;
    };

    virtual void delete_object() {};

    virtual void deallocate_control_block() {};

    size_t shared_counter;
    size_t weak_counter;
  };

  template<typename T, typename Alloc, typename... Args>
  friend SharedPtr<T> allocateShared(const Alloc alloc, Args &&...args);
};

template<typename T>
class SharedPtr : public SharedBase {
 public:
  SharedPtr() {
    control_block_ = nullptr;
    ptr_ = nullptr;
  }

  template<typename Y>
  SharedPtr(Y *ptr) {
    if (ptr == nullptr) {
      control_block_ = nullptr;
      ptr_ = nullptr;
    }
    control_block_ = new ControlBlock<Y>(ptr);
    ptr_ = ptr;
  };

  template<typename Y, typename Deleter>
  SharedPtr(Y *ptr, const Deleter &deleter) {
    control_block_ = new ControlBlock<Y, Deleter>(ptr, deleter);
    ptr_ = ptr;
  };

  template<typename Y, typename Deleter, typename Alloc>
  SharedPtr(Y *ptr, const Deleter &deleter, const Alloc &alloc) {
    using cb_type = ControlBlock<Y, Deleter, Alloc>;
    using cb_alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<cb_type>;

    cb_alloc_type cb_allocator = alloc;

    control_block_ = cb_allocator.allocate(1);
    new (control_block_) cb_type(ptr, deleter, alloc);
    ptr_ = ptr;
  };

  template<typename Y>
  SharedPtr(const SharedPtr<Y> &other) {
    control_block_ = other.control_block_;
    if (control_block_ != nullptr) {
      ++(control_block_->shared_counter);
    }
    ptr_ = other.ptr_;
  };

  SharedPtr(const SharedPtr<T> &other) {
    control_block_ = other.control_block_;
    if (control_block_ != nullptr) {
      ++(control_block_->shared_counter);
    }
    ptr_ = other.ptr_;
  };

  template<typename Y>
  SharedPtr(SharedPtr<Y> &&other) {
    control_block_ = other.control_block_;
    ptr_ = other.ptr_;

    other.control_block_ = nullptr;
    other.ptr_ = nullptr;
  };

  template<typename Y>
  SharedPtr &operator=(const SharedPtr<Y> &other) {
    if (*this == other) {
      return *this;
    }

    this->~SharedPtr();
    control_block_ = other.control_block_;
    if (control_block_ != nullptr) {
      ++(control_block_->shared_counter);
    }
    ptr_ = other.ptr_;

    return *this;
  };

  SharedPtr &operator=(const SharedPtr<T> &other) {
    if (*this == other) {
      return *this;
    }

    this->~SharedPtr();
    control_block_ = other.control_block_;
    if (control_block_ != nullptr) {
      ++(control_block_->shared_counter);
    }
    ptr_ = other.ptr_;

    return *this;
  }

  template<typename Y>
  SharedPtr &operator=(SharedPtr<Y> &&other) {
    if (*this == other) {
      if (control_block_ && control_block_->shared_counter > 1) {
        --(control_block_->shared_counter);
      }
    } else {
      this->~SharedPtr();
    }

    control_block_ = other.control_block_;
    ptr_ = other.ptr_;
    other.control_block_ = nullptr;
    other.ptr_ = nullptr;

    return *this;
  };

  template<typename Y>
  bool operator==(const SharedPtr<Y> &other) {
    return (this->get() == other.get());
  }

  void swap(SharedPtr &other) {
    std::swap(control_block_, other.control_block_);
    std::swap(ptr_, other.ptr_);
  };

  ~SharedPtr() {
    if (control_block_ == nullptr) {
      return;
    }

    bool cb_alive = true;
    if (control_block_->shared_counter == 1) {
      control_block_->delete_object();
      if (control_block_->weak_counter == 0) {
        control_block_->deallocate_control_block();
        cb_alive = false;
      }
    }
    if (cb_alive) {
      --(control_block_->shared_counter);
    }
  };

  size_t use_count() const {
    if (control_block_ == nullptr) {
      return 0;
    }
    return control_block_->shared_counter;
  };

  void reset() {
    SharedPtr().swap(*this);
  };

  template<typename Y>
  void reset(Y *ptr) {
    SharedPtr(ptr).swap(*this);
  };

  T *get() {
    return ptr_;
  };

  const T *get() const {
    return ptr_;
  };

  T &operator*() {
    return *ptr_;
  };

  const T &operator*() const {
    return *ptr_;
  };

  T *operator->() {
    return ptr_;
  }

  const T *operator->() const {
    return ptr_;
  };

 private:
  template<typename Y>
  friend class SharedPtr;

  template<typename Y>
  friend class WeakPtr;

  template<typename Y, typename... Args>
  friend SharedPtr<Y> makeShared(Args &&...args);

  template<typename Y, typename Alloc, typename... Args>
  friend SharedPtr<Y> allocateShared(const Alloc alloc, Args &&...args);

  SharedPtr(BaseControlBlock *control_block) {
    control_block_ = control_block;
    ptr_ = static_cast<T *>(control_block_->get_ptr());
  };

  template<typename U, typename Deleter = std::default_delete<U>, typename Allocator = std::allocator<U>>
  struct ControlBlock : BaseControlBlock {
    using cb_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<ControlBlock<U, Deleter, Allocator>>;

    ControlBlock(U *ptr, Deleter deleter = std::default_delete<U>(), Allocator allocator = std::allocator<U>()) : ptr(ptr), deleter(deleter), allocator(allocator){};

    void *get_ptr() const final {
      return static_cast<T *>(ptr);
    }

    void delete_object() final {
      deleter(ptr);
    }

    void deallocate_control_block() final {
      cb_allocator cb_alloc = allocator;
      std::allocator_traits<cb_allocator>::deallocate(cb_alloc, this, 1);
    }

    U *ptr;
    Deleter deleter;
    Allocator allocator;
  };

  template<typename U, typename Allocator = std::allocator<U>>
  struct SharedControlBlock : BaseControlBlock {
    using scb_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<SharedControlBlock<U, Allocator>>;

    template<typename... Args>
    SharedControlBlock(Allocator alloc, Args &&...args) : BaseControlBlock(), allocator(alloc) {
      ptr = new (object) U(std::forward<Args>(args)...);
    }

    void *get_ptr() const final {
      return static_cast<T *>(ptr);
    }

    void delete_object() final {
      ptr->~T();
    }

    void deallocate_control_block() final {
      scb_allocator scb_alloc = allocator;
      scb_alloc.destroy(this);
      std::allocator_traits<scb_allocator>::deallocate(scb_alloc, this, 1);
    }

    alignas(U) char object[sizeof(U)];
    U *ptr;
    Allocator allocator;
  };

  mutable BaseControlBlock *control_block_;
  mutable T *ptr_;
};

template<typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc alloc, Args &&...args) {
  using cb_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<typename SharedPtr<T>::template SharedControlBlock<T, Alloc>>;
  cb_allocator cb_alloc = alloc;
  auto ptr = cb_alloc.allocate(1);
  std::allocator_traits<cb_allocator>::construct(cb_alloc, ptr, alloc, std::forward<Args>(args)...);
  return SharedPtr<T>(static_cast<SharedBase::BaseControlBlock *>(ptr));
}

template<typename T, typename... Args>
SharedPtr<T> makeShared(Args &&...args) {
  return allocateShared<T, std::allocator<T>, Args...>(std::allocator<T>(), std::forward<Args>(args)...);
}

template<typename T>
class WeakPtr {
 public:
  WeakPtr() {
    control_block_ = nullptr;
  };

  template<typename U>
  WeakPtr(const WeakPtr<U> &other) {
    control_block_ = other.control_block_;
    if (control_block_ != nullptr) {
      ++(control_block_->weak_counter);
    }
  };

  WeakPtr(const WeakPtr<T> &other) {
    control_block_ = other.control_block_;
    if (control_block_ != nullptr) {
      ++(control_block_->weak_counter);
    }
  }

  template<typename U>
  WeakPtr(WeakPtr<U> &&other) {
    control_block_ = other.control_block_;
    other.control_block_ = nullptr;
  };

  template<typename U>
  WeakPtr(const SharedPtr<U> &other) {
    control_block_ = other.control_block_;
    if (control_block_ != nullptr) {
      ++(control_block_->weak_counter);
    }
  };

  template<typename U>
  WeakPtr<T> &operator=(const WeakPtr<U> &other) {
    if (*this == other) {
      return *this;
    }

    ~WeakPtr();
    control_block_ = other.control_block_;
    if (control_block_ != nullptr) {
      ++(control_block_->weak_counter);
    }

    return *this;
  };

  WeakPtr<T> &operator=(const WeakPtr<T> &other) {
    if (*this == other) {
      return *this;
    }

    this->~WeakPtr();
    control_block_ = other.control_block_;
    if (control_block_ != nullptr) {
      ++(control_block_->weak_counter);
    }

    return *this;
  }

  template<typename U>
  WeakPtr<T> &operator=(WeakPtr<U> &&other) {
    if (*this == other) {
      if (control_block_->weak_counter > 1) {
        --(control_block_->weak_counter);
      }
    } else {
      ~WeakPtr();
    }

    control_block_ = other.control_block_;
    other.control_block_ = nullptr;

    return *this;
  };

  template<typename Y>
  bool operator==(const WeakPtr<Y> &other) {
    return (control_block_ == other.control_block_);
  }

  ~WeakPtr() {
    if (control_block_ == nullptr) {
      return;
    }

    bool cb_alive = true;
    if (control_block_->shared_counter == 0 && control_block_->weak_counter == 1) {
      control_block_->deallocate_control_block();
      cb_alive = false;
    }
    if (cb_alive) {
      --(control_block_->weak_counter);
    }
  };

  size_t use_count() const {
    if (control_block_ == nullptr) {
      return 0;
    }
    return control_block_->shared_counter;
  };

  bool expired() const {
    if (control_block_ == nullptr) {
      return true;
    }
    return control_block_->shared_counter == 0;
  };

  SharedPtr<T> lock() const {
    ++(control_block_->shared_counter);
    return SharedPtr<T>(control_block_);
  };

 private:
  template<typename Y>
  friend class WeakPtr;

  typename SharedPtr<T>::BaseControlBlock *control_block_;
};

#endif//SHARED_PTR__SMART_POINTERS_H_
