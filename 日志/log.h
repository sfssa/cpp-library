#pragma once

/*
    如果编译器是UNIX（包括Linux）或者是macOS，并且正在使用gcc或者clang编译器，定义LOG_MACRO_UNIX
    如果编译环境是Windows，并且正在使用Microsoft Visual C++编译器，那么定义LOG_MACRO_WINDOWS

    __unix__,__APPLE__,__MACH__是编译器提供的预定义宏，不同的系统下定义不同的宏

    这种做通常用于在跨平台的代码中根据不同的平台或编译器采取不同的实现或配置，通过条件编译，在不同
    的编译器或平台下，使用不同的代码逻辑
*/

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define LOG_MACRO_UNIX
#elif defined(_MSC_VER)
#define LOG_MACRO_WINDOWS
#endif

// 没有定义LOG_MACRO_NO_WARNING，编译器输出警告，提示开发者该头文件已被包含在代码中
#ifndef LOG_MACRO_NO_WARNING
#pragma message("WARNING: the 'log.h header is included in our code base")
#endif

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

// 包含Unix平台所需头文件
#ifdef LOG_MACRO_UNIX
#include <unistd.h>
#endif

// 选择不同的C++版本，以便在后续根据不同的版本进行编译
// 对于 C++98,__cplusplus 的值是 199711L。
// 对于 C++11,__cplusplus 的值是 201103L。
// 对于 C++14,__cplusplus 的值是 201402L。
// 对于 C++17,__cplusplus 的值是 201703L。
#if __cplusplus >= 201703L
#define LOG_MACRO_CXX_STANDARD 17
#elif __cplusplus >= 201402L
#define LOG_MACRO_CXX_STANDARD 14
#else
#define LOG_MACRO_CXX_STANDARD 11
#endif

// optional-一个可能包含也可能不包含值的容器
// variant-一个类型安全的联合，可以保存不同类型的值，但一次只能保存一个，适用于变量有多个不同的类型并想使类型信息显式时
#if LOG_MACRO_CXX_STANDARD >= 17
#include <optional>
#include <variant>
#endif

namespace log
{

    // 是否启用了彩色输出
    // fileno-将一个标准I/O流转换成对应的文件描述符,stdin-stdout-stderr:0-1-2
    // isatty-检查文件描述符是否关联到一个终端设备tty
    inline bool isColorrizedOutputEnabled()
    {
#if defined(LOG_MACRO_FORCE_COLOR)
        return true;
#elif defined(LOG_MACRO_UNIX)
        return isatty(fileno(stderr));
#else
        return true; // 默认情况启动彩色输出
#endif
    }

    struct time
    {
    };

    /*
        用于处理编译器特定的__PRETTY_FUNCTION__或者__FUNCSIG__，目的是提取模板参数的类型名称

        在不同的编译器中，__PRETTY_FUNCTION__提供了包含函数签名和其他信息的字符串，这段代码根据编译器的类型定义了
        不同的宏，计算截取类型名称所需的前缀和后缀的长度

        通过定义特殊的宏和常量，在type_name_impl函数中截取函数签名中的模板参数类型名称，从而实现一些调试输出
        或者其他用途，这种方法的目的是保持对不同编译器的兼容性

        不同的编译器适用不同的宏定义，__PRETTY_FUNCTION__是GCC，__FUNCSIG__是MVS，用于获取当前函数的带有
        签名信息的字符串，包含函数的返回类型、函数名、参数类型等信息
    */
    namespace pretty_function
    {
#if defined(__clang__)
#define LOG_MACRO_PRETTY_FUNCTION __PRETTY_FUNCTION__
        static constexpr size_t PREFIX_LENGTH =
            sizeof("const char* log::type_name_impl()[T = ") - 1;
        static constexpr size_t SUFFIX_LENGTH = sizeof("]") - 1;
#elif defined(__GNUC__) && !defined(_clang__)
#define LOG_MACRO_PRETTY_FUNCTION __PRETTY_FUNCTION__
        static constexpr size_t PREFIX_LENGTH =
            sizeof("const char* log::type_name_name_impl()[with T =") - 1;
        static constexpr size_t SUFFIX_LENGTH = sizeof("]") - 1;
#elif defined(_MSC_VER)
#define LOG_MACRO_PRETTY_FUNCTION __FUNCSIG__
        static constexpr size_t PREFIX_LENGTH =
            sizeof("const char* __cdecl log::type_name_impl<") - 1;
        static constexpr size_t SUFFIX_LENGTH = sizeof(">(void)") - 1;
#else
#error "This conplier is currently not supported by log_macro."
#endif
    }

    /*
        提供了一种对整数类型进行格式化输出的方式，支持不同的数值基数，并通过静态断言确保只能用于整数
    */
    template <typename T>
    struct print_formatted
    {
        // is_integral函数判断T是否是整型
        static_assert(std::is_integral<T>::value, "Only integral types are supported.");

        print_formatted(T value, int numeric_base)
            : inner(value), base(numeric_base) {}

        // 允许被隐式转换成T类型
        operator T() const { return inner; }

        // 处理前缀
        const char *prefix() const
        {
            switch (base)
            {
            case 8:
                return "0o";
            case 16:
                return "0x";
            case 2:
                return "0b";
            default:
                return "";
            }
        }

        T inner;  // 要格式化输出的值
        int base; // 格式化后的值
    };

    template <typename T>
    print_formatted<T> hex(T value)
    {
        return print_formatted<T>{value, 16};
    }

    template <typename T>
    print_formatted<T> oct(T values)
    {
        return print_formatted<T>{value, 8};
    }

    template <typename T>
    print_formatted<T> bin(T value)
    {
        return print_formatted<T>{value, 2};
    }

    // 返回当前执行函数带有签名信息的字符串
    template <typename T>
    const char *type_name_impl()
    {
        return LOG_MACRO_PRETTY_FUNCTION;
    }

    // 用来在编译时传递类型信息
    template <typename T>
    struct type_tag
    {
    };

    /*
        ExplicitAgumentBarrier是一个参数包展开，可以接收零个或多个模板参数，每个都是int&类型
        rank获取数组的维度信息，如果不是数组值为0
        enable_if检查std::rank<T>::value是否是数组，是，返回类型被指定为string，否则，该模板被排除在可选的模板实例化列表之外
    */
    template <int &...ExplicitAgumentBarrier, typename T>
    typename std::enable_if<(std::rank<T>::value == 0), std::string>::type
    get_type_name(type_tag<T>)
    {
        namespace pf = pretty_function;
        std::string type = type_name_impl<T>();
        // 从前缀下一个元素开始，截取函数信息-前缀长度-后缀长度，得到类型信息
        // 比如"const char* get_type_name<SomeType>()"，最终截取的就是SomeType，也就是类型信息
        return type.substr(pf::PREFIX_LENGTH, type.size() - pf::PREFIX_LENGTH - pf::SUFFIX_LENGTH);
    }

    // 函数用来保证执行进入get_type_name函数的参数是一个字符串，报吃enable_if条件的成立
    template <typename T>
    std::string type_name()
    {
        // 是否是volatil修饰的变量
        if (std::is_volatile<T>::value)
        {
            // 是否是指针
            if (std::is_pointer<T>::value)
            {
                // 移除volatile并追加到后面，将valatile int  -> int valatile
                return type_name<typename std::remove_volatile<T>::type>::type() + " volatile";
            }
            else
            {
                // 不是指针，返回int valatile
                return "volatile " + type_name<typename std::remove_volatile<T>::type>();
            }
        }
        // 是否const修饰
        if (std::is_const<T>::value)
        {
            if (std::is_pointer<T>::value)
            {
                return type_name < typename std::remove_const<T>::type() + " const";
            }
            else
            {
                return "const " + type_name<typename std::remove_const<T>::type>();
            }
        }
        if (std::is_pointer<T>::value)
        {
            return type_name<typename std::remove_pointer<T>::type>() + "*";
        }
        if (std::is_lvalue_reference<T>::value)
        {
            return type_name<typename std::remove_reference<T>::type>() + "&";
        }
        if (std::is_rvalue_reference<T>::value)
        {
            return type_name<typename std::remove_reference<T>::type>() + "&&";
        }
        // 经过上面的分支语句得到字符串类型，然后进入下面的函数获得类型
        return get_type_name(type_tag<T>{});
    }

// 判断t_std和t_std是否相等，是返回t_bit字符串，否则返回t_std字符串
// 在编译时将类型转换成字符串，在编译时获得数据名称的函数
#define LOG_MACRO_REGISTER_TYPE_ASSOC(t_std, t_bit)                 \
    inline constexpr const char *get_type_name(type_tag<t_std>)     \
    {                                                               \
        return std::is_same<t_std, t_bit>::value ? #t_bit : #t_std; \
    }

    // const char* typeName = get_type_name(type_tag<unsigned char>{});结果为uint8_t
    LOG_MACRO_REGISTER_TYPE_ASSOC(unsigned char, uint8_t)
    LOG_MACRO_REGISTER_TYPE_ASSOC(unsigned short, uint16_t)
    LOG_MACRO_REGISTER_TYPE_ASSOC(unsigned int, uint32_t)
    LOG_MACRO_REGISTER_TYPE_ASSOC(unsigned long, uint64_t)
    LOG_MACRO_REGISTER_TYPE_ASSOC(signed char, int8_t)
    LOG_MACRO_REGISTER_TYPE_ASSOC(short, int16_t)
    LOG_MACRO_REGISTER_TYPE_ASSOC(int, int32_t)
    LOG_MACRO_REGISTER_TYPE_ASSOC(long, int64_t)

    inline std::string get_type_name(type_tag<std::string>)
    {
        return "std::string";
    }

    // 返回一维数组的大小，例如：[10]
    template <typename T>
    typename std::enable_if<(std::rank<T>::value == 1), std::string>::type
    get_array_dim()
    {
        // std::extent函数获得数组的大小
        return "[" + std::to_string(std::extent<T>::value) + "]";
    }

    /*
        多维数组的大小
        递归调用自身，获取当前维度的大小，移除当前维度，继续获取下一维度的信息
        remove_extent用来移除当前维度
    */
    template <typename T>
    typename std::enable_if<(std::rank<T>::value > 1), std::string>::type
    get_array_dim()
    {
        return "[" + std::to_string(std::extent<T>::value) + "]" +
               get_array_dim<typename std::remove_extent<T>::type>();
    }

    /*
        是数组返回值为string类型
        remove_all_extents用来移除给定类型的所有数组维度，返回数组元素的类型
        获取类型后拼接维度信息，例如：int[10]
    */
    template <typename T>
    typename std::enable_if<(std::rank<T>::value > 0), std::string>::type
    get_type_name(type_tag<T>)
    {
        return type_name < typename std::remove_all_extents<T>::type() + get_array_dim<T>();
    }

    // array特化版本
    // 返回数组的类型和数组的大小
    template <typename T, size_t N>
    std::string get_type_name(type_tag<std::array<T, N>>)
    {
        return "std::array<" + type_name<T>() + ", " + std::to_string(N) + ">";
    }

    // vector特化版本
    // 返回vector类型信息
    template <typename T>
    std::string get_type_name(type_tag<std::vector<T, std::allocator<T>>>)
    {
        return "std::vector<" + type_name<T>() + ">";
    }

    // pair特化版本
    // 返回类型
    template <typename T1, typename T2>
    std::string get_type_name(type_tag<std::pair<T1, T2>>)
    {
        return "std::pair<" + type_name<T1>() + ", " + type_name<T2>() + ">";
    }

    /*
        使用折叠表达式和逗号运算符将每个类型的字符串添加到result中，每个字符串之间加上逗号和空格
        检查类型列表是否为空，如果为空，取出末尾多余的逗号和空格
    */
    template <typename... T>
    std::string type_list_to_string()
    {
        std::string result;
        auto unused = {(result += type_name<T>() + ", ", 0)..., 0};
        static_cast<void>(unused);
#if LOG_MACRO_CXX_STANDARD >= 17
        if constexpr (sizeof...(T) > 0)
        {
#else
        if (sizeof...(T) > 0)
        {
#endif
            result.pop_back();
            result.pop_back();
        }
        return result;
    }

    // tuple特化版本
    template <typename... T>
    std::string get_type_name(type_tag<std::tuple<T...>>)
    {
        return "std::tuple<" + type_list_to_string<T...>() + ">";
    }

    // print_formatted<T>特化版本
    template <typename T>
    inline std::string get_type_name(type_tag<print_formatted<T>>)
    {
        return type_name<T>();
    }
    
    // 是否可以使用给定的操作Op<Args...>
    namespace detail_detector
    {
        // 无效类型，表示没有匹配的情况
        struct nonesuch
        {
            nonesuch()=delete;
            ~nonesuch()=delete;
            nonesuch(nonesuch const&)=delete;
            void operator=(nonesuch const&)=delete;
        };

        // void_t被定义为别名，表示void
        // 利用SFINAE和模板优先找特化去匹配的特性
        template <typename...>
        using void_t=void;
        
        /*
            Default是默认模板参数，AlwaysVoid是启用SFINAE的void_t，Op是要检测的操作符，Args...是要传递给操作符的参数
            使用false_type表示无法匹配
            type默认是Default
        */
        template<class Default,class AlwaysVoid,template<class...> class Op,class... Args>
        struct detector
        {
            using value_t=std::false_type;
            using type=Default;
        };

        /*
            detector模板的偏特化版本，检测是否可以使用给定的Op<Args...>代替默认Default
            在传递给Op的类型参数Args...下，检测是否可以使用Op替代默认类型Default
            如果不满足就会找通用版本，放弃这个版本的实例化        
        */
        template <class Default,template <class...> class Op,class... Args>
        struct detector<Default,void_t<Op<Args...>>,Op,Args...>
        {
            using value_t=std::std::true_type;
            using type=Op<Args...>;
        };
    }

    
}