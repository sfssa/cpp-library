# 单例模式

单例模式确保一个类只有一个实例，并提供一个全局访问点。

1. **`GetInstanceX` 函数模板：**

   ```
   template<class T, class X, int N>
   T& GetInstanceX() {
       static T v;
       return v;
   }
   ```

   - `GetInstanceX` 是一个模板函数，用于返回类型 `T` 的静态实例。
   - `X` 是为了创造多个实例对应的标签（Tag），但在代码中未使用。
   - `N` 是同一个 Tag 创造多个实例时的索引，但在代码中未使用。

2. **`GetInstancePtr` 函数模板：**

   ```
   template<class T, class X, int N>
   std::shared_ptr<T> GetInstancePtr() {
       static std::shared_ptr<T> v(new T);
       return v;
   }
   ```

   - `GetInstancePtr` 也是一个模板函数，用于返回类型 `T` 的 `std::shared_ptr`。
   - 类似于 `GetInstanceX`，`X` 和 `N` 在代码中未使用。

3. **`Singleton` 模板类：**

   ```
   cppCopy codetemplate<class T, class X = void, int N = 0>
   class Singleton {
   public:
       static T* GetInstance() {
           static T v;
           return &v;
           // return &GetInstanceX<T, X, N>();
       }
   };
   ```

   - `Singleton` 模板类提供了对 `GetInstanceX` 的简单封装，返回类型 `T` 的指针。
   - 使用静态变量确保了在程序生命周期内只有一个实例。

4. **`SingletonPtr` 模板类：**

   ```
   cppCopy codetemplate<class T, class X = void, int N = 0>
   class SingletonPtr {
   public:
       static std::shared_ptr<T> GetInstance() {
           static std::shared_ptr<T> v(new T);
           return v;
           // return GetInstancePtr<T, X, N>();
       }
   };
   ```

   - `SingletonPtr` 模板类提供了对 `GetInstancePtr` 的简单封装，返回类型 `T` 的 `std::shared_ptr`。
   - 使用静态变量确保了在程序生命周期内只有一个实例。

这种设计允许用户选择使用 `Singleton` 或 `SingletonPtr`，具体取决于他们对实例的引用方式。设计上允许为不同的 `X` 或 `N` 创建不同的实例，但在当前的实现中，它们都未被使用。此外，这个实现没有考虑到线程安全性，因此在多线程环境下使用时可能需要添加额外的线程安全措施。

`X` 和 `N` 是为了在需要同一类型的多个单例实例时提供标识和索引的机制。但在上述代码中，默认情况下它们没有被使用，因此其默认值为 `void` 和 `0`。

```
#include <iostream>
#include "singleton.h"

class MySingleton {
public:
    void print() {
        std::cout << "MySingleton instance: " << this << std::endl;
    }
};

int main() {
    // 使用默认的 Singleton，只会创建一个 MySingleton 实例
    MySingleton* singleton1 = sylar::Singleton<MySingleton>::GetInstance();
    singleton1->print();

    // 使用具有不同 X 标签和 N 索引的 Singleton，可以创建另一个 MySingleton 实例
    MySingleton* singleton2 = sylar::Singleton<MySingleton, struct TagA, 1>::GetInstance();
    singleton2->print();

    // 再次使用默认 Singleton，仍然得到之前创建的实例
    MySingleton* singleton3 = sylar::Singleton<MySingleton>::GetInstance();
    singleton3->print();

    return 0;
}
```

在这个例子中：

- `MySingleton` 是我们想要做成单例的类。
- 我们首先使用了默认的 `Singleton`，创建了一个 `MySingleton` 实例。
- 然后，我们使用具有不同 `X` 标签（`struct TagA`）和 `N` 索引（`1`）的 `Singleton`，这样就创建了另一个 `MySingleton` 实例。
- 最后，再次使用默认的 `Singleton`，得到的是之前创建的实例。

也就是说，单例模式不再只能创建一个了这个实现允许你为同一类型创建多个单例实例，通过使用不同的标签 `X` 和索引 `N` 进行区分。默认情况下，如果没有提供 `X` 和 `N`，则会使用默认值 `void` 和 `0`，此时行为与传统的单例模式相同，只创建一个实例。这样的设计提供了更大的灵活性，使得在某些场景下能够更方便地使用单例模式。