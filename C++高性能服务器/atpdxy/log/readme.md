<!--
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-07 19:40:58
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-08 10:35:08
 * @FilePath: /C++高性能服务器/log/readme.md
 * @Description: g
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
-->
# 日志文件详解

## LogLevel-日志级别

这个类定义了日志的各个级别，包括DEBUG、INFO、WARN、ERROR和FATAL。每个级别都对应一个整数值，用于表示其严重程度。

1. **`enum Level` 枚举：**
   - `UNKNOW=0`: 未知级别。
   - `DEBUG=1`: 调试级别，用于输出详细的调试信息。
   - `INFO=2`: 信息级别，用于输出一般的程序信息。
   - `WARN=3`: 警告级别，表示可能的问题，但不会影响程序运行。
   - `ERROR=4`: 错误级别，表示出现了错误，但程序仍然可以继续运行。
   - `FATAL=5`: 致命错误级别，表示程序遇到了无法继续运行的严重错误。
2. **`toString` 静态方法：**
   - `static const char* toString(LogLevel::Level level)`: 将给定的日志级别转换为相应的字符串。例如，如果传递 `LogLevel::DEBUG`，则返回字符串 "DEBUG"。
3. **`levelFromString` 静态方法：**
   - `static LogLevel::Level levelFromString(const std::string& str)`: 将给定的字符串表示的日志级别转换为相应的枚举值。例如，如果传递字符串 "DEBUG"，则返回 `LogLevel::DEBUG`。

此类的主要目的是提供一种方便的方式来处理和表示不同的日志级别。通常，在日志系统中，开发人员可以设置日志级别，以便在程序运行时只输出特定级别以上的日志，以方便调试和故障排除。

## LogEvent-日志事件

这个类表示一次日志事件，包括事件的相关信息，如日志级别、发生时间、文件名、行号、线程ID、协程ID等。它还包括一个字符串流（stringstream），可以将日志消息写入其中。

1. **`typedef std::shared_ptr<LogEvent> ptr;`：**
   - 为 `LogEvent` 类创建了一个智能指针类型 `ptr`，使得可以方便地使用共享指针管理 `LogEvent` 对象的生命周期。
2. **构造函数：**
   - `LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time, const std::string& thread_name);`
   - `LogEvent(const char* file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time);`
   - 构造函数用于创建 `LogEvent` 对象。第一个构造函数接受日志器 `Logger` 的共享指针，日志级别 `LogLevel::Level`，以及与日志事件相关的各种参数（文件名、行号、耗时等）。第二个构造函数是一个简化版本，不接受日志器和级别，仅接受与事件相关的参数。
3. **成员函数：**
   - `const char* getFile() const`: 返回文件名。
   - `int32_t getLine() const`: 返回行号。
   - `uint32_t getElapse() const`: 返回程序启动到现在的毫秒数。
   - `uint32_t getThreadId() const`: 返回线程ID。
   - `uint32_t getFiberId() const`: 返回协程ID。
   - `uint64_t getTime() const`: 返回时间戳。
   - `const std::string& getThreadName() const`: 返回线程名称。
   - `std::string getContent() const`: 返回日志内容。
   - `std::shared_ptr<Logger> getLogger() const`: 返回日志器。
   - `LogLevel::Level getLevel() const`: 返回日志等级。
   - `std::stringstream& getSS()`: 返回日志内容字符串流。
4. **格式化写入日志内容的函数：**
   - `void format(const char* fmt, ...)`: 使用可变参数列表将格式化字符串写入日志内容流。
   - `void format(const char* fmt, va_list al)`: 使用可变参数列表将格式化字符串写入日志内容流。
5. **私有成员变量：**
   - `const char* m_file_`: 文件名。
   - `int32_t m_line_`: 行号。
   - `uint32_t m_elapse_`: 程序启动到现在的毫秒数。
   - `uint32_t m_thread_id_`: 线程ID。
   - `uint32_t m_fiber_id_`: 协程ID。
   - `uint64_t m_time_`: 时间戳。
   - `std::string m_thread_name_`: 线程名称。
   - `std::stringstream m_ss_`: 日志内容流。
   - `std::shared_ptr<Logger> m_logger_`: 日志器。
   - `LogLevel::Level m_level_`: 日志等级。

## LogEventWrap-日志封装器

这是一个用于包装 `LogEvent` 对象的辅助类 `LogEventWrap`。

1. **构造函数：**

   - ```
     LogEventWrap(LogEvent::ptr e);
     ```

     - 构造函数接收一个 `LogEvent` 对象的智能指针（`LogEvent::ptr`），并将其保存在 `m_event_` 成员变量中。

2. **析构函数：**

   - ```
     ~LogEventWrap();
     ```

     - 析构函数在对象销毁时被调用。在这个类中，析构函数的作用是释放 `LogEvent` 对象，因为 `LogEventWrap` 类负责管理 `LogEvent` 对象的生命周期。

3. **成员函数：**

   - `LogEvent::ptr getEvent() const`: 返回保存的 `LogEvent` 对象的智能指针。
   - `std::stringstream& getSS()`: 返回 `LogEvent` 对象的日志内容字符串流。

4. **私有成员变量：**

   - `LogEvent::ptr m_event_`: 保存的 `LogEvent` 对象的智能指针。

这个类的主要目的是通过构造函数接收一个 `LogEvent` 对象，然后在析构函数中负责释放这个对象。这样做的好处是，可以在创建 `LogEventWrap` 对象时执行某些操作，而在 `LogEventWrap` 对象销毁时，确保 `LogEvent` 对象也得到正确释放。这种模式常用于资源管理和生命周期控制，以确保在对象不再需要时进行清理工作。

## LogFormatter-日志格式器

这个类用于将日志事件格式化为特定的字符串。它可以根据配置的模板将日志事件中的信息格式化为特定的格式，例如时间、线程ID、消息内容等。如果出现配置错误，它会标记为错误。

1. **构造函数：**

   - ```
     LogFormatter(const std::string& pattern);
     ```

     - 构造函数接收一个输出的格式模板 `pattern`，用于指定日志的输出格式。

2. **成员函数：**

   - ```
     std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
     ```

     - 根据给定的日志器 (`logger`)、日志级别 (`level`) 和日志事件 (`event`)，返回格式化后的日志字符串。

   - ```
     std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
     ```

     - 将格式化后的日志输出到给定的输出流 (`ofs`)，同时考虑日志器、日志级别和日志事件。

   - ```
     void init();
     ```

     - 初始化解析日志模板，将模板解析成一系列的 `FormatItem` 对象，用于后续的日志格式化。

   - ```
     bool isError() const;
     ```

     - 返回是否在解析日志模板时出现了错误。

   - ```
     const std::string getPattern() const;
     ```

     - 返回日志模板字符串。

3. **私有成员变量：**

   - ```
     std::string m_pattern_;
     ```

     - 存储日志模板。

   - ```
     std::vector<FormatItem::ptr> m_items_;
     ```

     - 存储日志格式解析后的格式项，每个格式项是一个 `FormatItem` 对象。

   - ```
     bool m_error_;
     ```

     - 记录解析日志模板时是否出现了错误。

4. **内部类 `FormatItem`：**

   - ```
     class FormatItem
     ```

      是一个虚基类，定义了日志格式项的接口。

     - ```
       virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
       ```

       - 纯虚函数，派生类需要实现该函数以执行具体的格式化操作。

这个类的主要功能是将日志按照指定的格式模板进行格式化，用户可以通过构造函数提供自定义的格式模板。解析后的模板以 `FormatItem` 对象的形式存储，每个 `FormatItem` 对象负责实际的格式化操作。在实际使用中，用户可以调用 `format` 函数，将日志格式化为字符串或输出到指定的输出流。

### FormatItem-格式项

`FormatItem` 类是 `LogFormatter` 类中用于表示日志格式项的基类。该类定义了一个接口，派生类需要实现该接口，以便将具体的日志格式项添加到输出流中。

1. **公共成员函数：**

   - ```
     typedef std::shared_ptr<FormatItem> ptr;
     ```

     - 使用 `std::shared_ptr` 管理 `FormatItem` 对象的智能指针类型定义。

   - ```
     virtual ~FormatItem(){}
     ```

     - 虚析构函数，确保通过基类指针删除对象时，能够正确调用派生类的析构函数。

   - ```
     virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
     ```

     - 纯虚函数，表示格式化日志项的接口。派生类需要实现这个函数，以将具体的日志格式项添加到输出流中。

2. **说明：**

   - `FormatItem` 类是一个纯虚类，不能被直接实例化，只能通过派生类来创建实例。
   - 派生类需要提供实现 `format` 函数的具体逻辑，以完成将特定格式的日志项添加到输出流的功能。
   - 通过使用智能指针进行管理，确保在适当的时候正确释放资源。

有多种类型的派生类，比如`MessageFormatItem`，`NameFormatItem`等继承这个`FormatItem`类，通过`format`函数实现自己的输出方式。而一个`LogFormatter`有自己的匹配格式，比如%m = 消息，%p = 日志级别，这些格式项都保存在`LogFormatter`内的`m_items_`中，在输出日志时通过遍历这个容器来输出所有信息。

`FormatItem` 类的设计使得 `LogFormatter` 类可以通过组合不同的格式项来构建复杂的日志格式。每个具体的格式项都是一个派生类，负责处理不同的日志信息部分（如时间、日志级别、线程ID等）。这种设计使得日志格式的灵活性得到了增强，用户可以根据自己的需求选择或扩展不同的格式项。

## LogAppender-日志输出目标

这个类表示日志输出的目标，可以是控制台、文件、网络等。它定义了将日志事件输出到目标的接口，并可以设置日志级别和格式化器。不同的输出目标可以派生自这个类。

1. **类型定义：**

   - ```
     typedef std::shared_ptr<LogAppender> ptr;
     ```

     - 使用 `std::shared_ptr` 管理 `LogAppender` 对象的智能指针。

2. **析构函数：**

   - ```
     virtual ~LogAppender(){}
     ```

     - 虚析构函数，确保派生类的析构函数能够被正确调用。

3. **纯虚函数：**

   - ```
     virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
     ```

     - 纯虚函数，需要在派生类中实现。用于将日志输出到具体的目标。

4. **虚函数：**

   - ```
     virtual std::string toYamlString() = 0;
     ```

     - 将日志输出目标的配置转换成 YAML 格式的字符串。虚函数，需要在派生类中实现。

5. **成员函数：**

   - ```
     void setFormatter(LogFormatter::ptr val);
     ```

     - 设置日志格式器。

   - ```
     LogFormatter::ptr getFormatter();
     ```

     - 获取日志格式器。

   - ```
     LogLevel::Level getLevel() const;
     ```

     - 获取日志级别。

   - ```
     void setLevel(LogLevel::Level val);
     ```

     - 设置日志级别。

6. **保护成员变量：**

   - ```
     LogLevel::Level m_level_ = LogLevel::DEBUG;
     ```

     - 日志级别，默认为 `DEBUG`。

   - ```
     bool m_hasFormatter_ = false;
     ```

     - 记录是否有自己的日志格式器。

   - ```
     LogFormatter::ptr m_formatter_;
     ```

     - 日志格式器，用于格式化输出的日志内容。

7. **友元类：**

   - ```
     friend class Logger;
     ```

     - 将 `Logger` 类声明为友元类，使得 `Logger` 类可以访问 `LogAppender` 类的私有成员。

该类定义了一系列接口，要求派生类实现具体的日志输出操作。它也提供了一些设置和获取日志格式器、级别的方法。这样的设计使得用户可以通过不同的日志输出目标来扩展日志系统，例如，可以有文件输出、控制台输出等。

### StdoutLogAppender-控制台日志输出目标

`StdoutLogAppender` 类是一个日志输出到标准输出（stdout）的日志目标（Appender）。以下是对该类的详细解释：

1. **公共成员函数：**

   - ```
     void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
     ```

     - 重写基类的虚函数，将日志信息输出到标准输出。

   - ```
     std::string toYamlString() override;
     ```

     - 重写基类的虚函数，将配置信息输出成 YAML 格式的字符串。

2. **特有成员类型：**

   - ```
     typedef std::shared_ptr<StdoutLogAppender> ptr;
     ```

     - 类型别名，方便使用智能指针管理对象。

3. **特性说明：**

   - 将日志信息输出到标准输出。

### FileLogAppender-文件日志输出目标

`FileLogAppender` 类是一个将日志输出到文件的日志目标（Appender）。以下是对该类的详细解释：

1. **公共成员函数：**

   - ```
     FileLogAppender(const std::string& filename);
     ```

     - 构造函数，接受文件名作为参数，用于初始化日志文件名。

   - ```
     void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
     ```

     - 重写基类的虚函数，将日志信息输出到文件。

   - ```
     std::string toYamlString() override;
     ```

     - 重写基类的虚函数，将配置信息输出成 YAML 格式的字符串。

   - ```
     bool reopen();
     ```

     - 重新打开日志文件。用于在日志文件被删除或者日志滚动时重新打开文件。

2. **特有成员类型：**

   - ```
     typedef std::shared_ptr<FileLogAppender> ptr;
     ```

     - 类型别名，方便使用智能指针管理对象。

3. **特有成员变量：**

   - ```
     std::string m_filename_;
     ```

     - 文件名，表示日志输出的目标文件。

   - ```
     std::ofstream m_filestream_;
     ```

     - 文件流，用于写入日志信息到文件。

   - ```
     uint64_t m_last_time_;
     ```

     - 最后一次打开文件的时间戳，用于判断是否需要重新打开文件。

4. **特性说明：**

   - 将日志信息输出到指定的文件。
   - 具备重新打开日志文件的功能，以应对文件滚动等情况。

## Logger-日志器

这个类表示一个日志记录器，它可以添加多个日志输出目标（Appender），设置日志级别，配置日志格式化器。当日志事件被记录时，它将事件传递给所有已添加的输出目标。

1. **类型定义：**

   - ```
     typedef std::shared_ptr<Logger> ptr;
     ```

     - 使用 `std::shared_ptr` 管理 `Logger` 对象的智能指针。

2. **继承关系：**

   - ```
     public std::enable_shared_from_this<Logger>
     ```

     - 继承自 `std::enable_shared_from_this`，用于在成员函数中获取指向当前对象的 `shared_ptr`。

3. **友元类：**

   - ```
     friend class LoggerManager;
     ```

     - 将 `LoggerManager` 类声明为友元类，使其可以访问 `Logger` 类的私有成员。

4. **构造函数：**

   - ```
     Logger(const std::string& name = "root");
     ```

     - 构造函数，接受一个字符串作为日志器名称，默认为 "root"。

5. **日志写入函数：**

   - ```
     void log(LogLevel::Level level, LogEvent::ptr event);
     ```

     - 写入指定级别的日志。

   - ```
     void debug(LogEvent::ptr event);
     ```

     - 写入 `DEBUG` 级别的日志。

   - ```
     void info(LogEvent::ptr event);
     ```

     - 写入 `INFO` 级别的日志。

   - ```
     void warn(LogEvent::ptr event);
     ```

     - 写入 `WARN` 级别的日志。

   - ```
     void error(LogEvent::ptr event);
     ```

     - 写入 `ERROR` 级别的日志。

   - ```
     void fatal(LogEvent::ptr event);
     ```

     - 写入 `FATAL` 级别的日志。

6. **日志目标操作：**

   - ```
     void addAppender(LogAppender::ptr appender);
     ```

     - 添加日志目标。

   - ```
     void delAppender(LogAppender::ptr appender);
     ```

     - 删除日志目标。

   - ```
     void clearAppenders();
     ```

     - 清空日志目标集合。

7. **日志级别操作：**

   - ```
     LogLevel::Level getLevel() const;
     ```

     - 获取日志级别。

   - ```
     void setLevel(LogLevel::Level level);
     ```

     - 设置日志级别。

8. **日志名称操作：**

   - ```
     const std::string& getName() const;
     ```

     - 获取日志器名称。

9. **日志格式器操作：**

   - ```
     void setFormatter(LogFormatter::ptr val);
     ```

     - 设置日志格式器。

   - ```
     void setFormatter(const std::string& val);
     ```

     - 设置日志格式模板。

   - ```
     LogFormatter::ptr getFormatter();
     ```

     - 获取日志格式器。

10. **日志配置操作：**

- ```
  std::string toYamlString();
  ```

  - 将日志器的配置信息转换成 YAML 格式的字符串。

1. **私有成员变量：**

- ```
  std::string m_name_;
  ```

  - 日志器名称。

- ```
  LogLevel::Level m_level_;
  ```

  - 日志级别。

- ```
  std::list<LogAppender::ptr> m_appenders_;
  ```

  - 日志目标集合。

- ```
  LogFormatter::ptr m_formatter_;
  ```

  - 日志格式器。

- ```
  Logger::ptr m_root_;
  ```

  - 主日志器，用于在获取根日志器时的优化。

该类提供了一系列对日志进行管理和配置的接口，用户可以通过该类实现对日志的灵活控制。

## LoggerManager-日志管理器

这个类用于管理所有的日志器（Logger）对象。它可以根据日志器的名称获取日志器对象，也负责初始化和管理所有的日志器。主要包括获取日志器、初始化、获取主日志器以及将所有的日志器配置转化为YAML字符串等功能。

1. **公共成员函数：**

   - ```
     LoggerManager();
     ```

     - 构造函数，用于初始化 `LoggerManager` 对象。

   - ```
     Logger::ptr getLogger(const std::string& name);
     ```

     - 根据日志器名称获取日志器对象。如果不存在该名称的日志器，则创建一个新的并返回。

   - ```
     void init() {}
     ```

     - 空函数，可能是为了保持接口一致性而存在。在实际中并未实现具体逻辑。

   - ```
     Logger::ptr getRoot() const;
     ```

     - 返回主日志器，即根日志器。主要用于获取整个日志系统的根节点。

   - ```
     std::string toYamlString();
     ```

     - 将日志器配置信息转换成 YAML 格式的字符串。用于输出当前日志系统的配置信息。

2. **保护成员：**

   - ```
     std::map<std::string, Logger::ptr> m_loggers_;
     ```

     - 保存日志器的容器，使用日志器名称作为键值。通过这个容器，可以根据日志器的名称快速查找和管理各个日志器对象。

   - ```
     Logger::ptr m_root_;
     ```

     - 主日志器，即整个日志系统的根节点。通过根日志器，可以获取整个日志系统的配置和状态。

3. **互斥锁：**

   - ```
     // MutexType m_mutex_;
     ```

     - 互斥锁，可能是为了保护日志器容器的并发访问而存在。在注释掉的情况下，可能没有实际使用互斥锁。

`LoggerManager` 类的设计是为了实现对日志器的集中管理，通过日志器的名称来区分和查找不同的日志器对象。通过管理器，用户可以更方便地配置和获取日志器，实现对整个日志系统的统一管理。

# 为什么给日志事件加一层包装？

名为 `LogEventWrap` 的类，其目的是为了包装 `LogEvent` 对象。这种设计通常被称为“装饰器模式”（Decorator Pattern）。下面这样设计的原因和优势：

1. **功能扩展：** 包装器可以用于向 `LogEvent` 中添加额外的功能或元数据。通过创建一个包装类，你可以在不修改原始类的情况下，动态地为对象添加新的行为。
2. **链式调用：** 包装器的使用允许你通过链式调用来构建对象。这种模式使代码更加清晰，易于理解，可以动态地组合不同的功能。
3. **解耦合：** 将功能添加到包装器中可以帮助解耦系统的各个部分。这样，你可以更容易地修改、扩展和维护系统，而不必直接修改核心类。
4. **灵活性：** 如果你的应用程序需要动态地在运行时为对象添加功能，装饰器模式是一种非常灵活的方式。你可以根据需要选择性地使用包装器。

# 为什么需要有一个主日志器？

在 `Logger` 类中，有一个成员变量 `m_root_` 被称为主日志器。主要的作用可能是为了实现日志器的层次结构和管理。

1. **层次结构：** 通过使用主日志器，你可以建立一个层次结构，其中每个日志器都可以有一个父级。这使得日志器之间可以形成树状结构，其中根日志器可能是主应用程序日志，而子日志器可能是特定模块或组件的日志。
2. **配置继承：** 如果一个日志器没有指定特定的日志级别或格式器，它可以从其父级继承这些配置。这样，你可以在整个日志体系结构中共享一些通用的配置，而不必在每个日志器中都进行单独配置。
3. **统一管理：** 主日志器可以用作整个应用程序的日志管理中心，通过它可以统一管理日志的级别、格式等配置。这样的设计使得在整个应用程序中进行全局的日志配置变得更加方便。
4. **YAML 转换：** 在你的代码中，`toYamlString` 方法可能会使用主日志器作为起点，将整个日志体系结构的配置转换成 YAML 字符串。这样，你可以更方便地导出和管理整个日志配置。

总体来说，主日志器在日志系统中的设计可能是为了提供更灵活的配置、更好的层次结构和更统一的管理。

# 为什么要根据日志级别来选择是否输出日志？

选择是否输出日志基于日志级别的主要目的是让开发者有更灵活的控制，以满足不同场景和需求。

1. **过滤和聚焦：** 日志级别允许开发者过滤掉那些不重要的日志信息，只关注当前关注的问题。在调试过程中，你可能更关心 DEBUG 和 INFO 级别的信息，而在生产环境中可能更关心 WARN、ERROR 和 FATAL 级别的信息。
2. **性能优化：** 输出日志可能会对应用程序的性能产生一定的影响，尤其是在高并发或性能敏感的场景下。通过选择性地输出日志，你可以根据当前环境和需求来平衡日志的详细程度和性能开销。
3. **问题排查：** 不同级别的日志适用于不同的问题排查场景。DEBUG 级别的日志用于详细的调试信息，而 ERROR 和 FATAL 级别的日志则用于标识潜在的问题或错误。
4. **灵活配置：** 日志级别允许在运行时动态配置，而无需修改代码。这样，你可以在不同的运行环境中采用不同的日志级别，从而更好地满足运维和开发需求。
5. **日志存储：** 不同级别的日志可能需要不同的存储策略。例如，你可能希望将 DEBUG 和 INFO 级别的日志存储在本地文件中，而将 WARN、ERROR 和 FATAL 级别的日志发送到集中的日志服务器。

总体来说，通过基于日志级别选择是否输出日志，你可以在日志记录过程中实现更细粒度的控制和优化，以适应各种应用场景。

