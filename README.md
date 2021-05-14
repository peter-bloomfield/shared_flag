# shared_flag
A one-shot waitable boolean flag to facilitate safe and cooperative thread interruption in C++.

## Introduction
### What problem is this trying to solve?
It's important to be able to interrupt threads cooperatively (or "politely"), allowing them to
finish their work and free resources in a safe manner. There are several existing approaches, using
features such as `std::atomic&lt;bool&gt;`, `std::condition_variable`, `std::future`, and
`std::stop_token`.

However, in my experience, these either have some annoying limitations, or aren't particularly easy
to work with (especially for beginners). This library aims to provide a safe and versatile
alternative, which is more readble and easier to use.

### What do you mean by "shared" flag?
Think of it like `std::shared_ptr<>` in the standard library: multiple instances of the class can
refer to the same data in memory. (In fact, `shared_flag` currently uses a shared pointer
internally, although that may change in future.)

When you construct a totally new instance of `shared_flag`, it creates a flag structure in memory.
You can then make copies of that `shared_flag` instance, and they will all refer to the same flag
structure internally. If you use one instance to set the flag, then all related instances will see
the result. The original instance can even be destroyed, and the others will continue to work. The
internal flag data isn't deleted until no more instances refer to it.

In practical terms, this means you can have any number of threads waiting on the same flag. You can
even allow any number of threads to write to the same flag, although that situation is perhaps less
common.

It's worth highlighting that unrelated instances of `shared_flag` will not refer to the same flag
structure internally. For example:

```cpp
prb::shared_flag flag1;

// Refers to the same data as flag1 internally:
prb::shared_flag flag2{ flag1 };

// Totally independent of flag1 and flag2:
prb::shared_flag flag3;

```

### Example usage
In this example, the main thread constructs a shared flag and passes a read-only view of it to a
task running in a worker thread. The main thread sets the flag when it wants the worker thread to
stop.

```cpp
void task(prb::shared_flag_reader flag_reader)
{
    while (!flag_reader.wait_for(1s))
    {
        // Do regular work in the background here.
    }
}

int main()
{
    prb::shared_flag flag_writer;
    std::thread task_thread{ task, flag_writer };

    // Do some other long-running task here.

    flag_writer.set();
    task_thread.join();
    return 0;
}
```

Note:

* `shared_flag` is readable and writeable.
* `shared_flag_reader` is read-only.
* You can convert `shared_flag` to `shared_flag_reader`, but not the other way around.

## Build instructions
Prerequisites:
* A C++ compiler for your platform (must support C++17 or later).
* [CMake](https://cmake.org/) (version 3.20 or later).

To make a debug build, run the following commands from the root of the repository:

```bash
mkdir -p build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

To make a release build, run the following commands from the root of the repository:

```bash
mkdir -p build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Documentation
TODO

## Repository structure
TODO

## TODO list
- Expand documentation in readme
- Add CMake package/install info.
- Add static analysis / linting.
- Add a continuous integration pipeline to run unit tests.
- Add a script to generate doxygen documentation automatically.
- Add a Conan recipe?
