#pragma once

/*
MIT License
Copyright (c) 2017 Arlen Keshabyan (arlen.albert@gmail.com)
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <functional>
#include <unordered_map>

namespace nstd::utilities
{

class at_scope_exit
{
public:

    at_scope_exit(const std::function<void(void)> &functor) : _functor(functor) {}

    ~at_scope_exit()
    {
        _functor();
    }

    at_scope_exit() = delete;
    at_scope_exit(const at_scope_exit&) = delete;
    at_scope_exit(at_scope_exit&&) = delete;
    at_scope_exit &operator =(const at_scope_exit&) = delete;
    at_scope_exit &operator =(at_scope_exit&&) = delete;

private:
    std::function<void(void)> _functor;
};

namespace fibonacci
{

uint64_t recursive_fibonacci(const uint64_t n)
{
    return n < 2 ? n : recursive_fibonacci(n - 1) + recursive_fibonacci(n - 2);
}

uint64_t non_recursive_fibonacci(const uint64_t n)
{
    if (n < 2) return n;
    if (n == 2) return 1;

    uint64_t prev_prev { 1 }, prev { 1 }, current { 0 };

    for (uint64_t idx { 3 }; idx <= n; ++idx)
    {
        current = prev_prev + prev;
        prev_prev = prev;
        prev = current;
    }

    return current;
}

class optimized_fibonacci
{
public:
    optimized_fibonacci() = default;
    uint64_t operator ()(const uint64_t n)
    {
        if (_already_calculated.find(n) == std::end(_already_calculated))
        {
            for (auto idx { _high_water_mark + 1 }; idx <= n; ++idx)
                _already_calculated[idx] = _already_calculated[idx - 1] + _already_calculated[idx - 2];

            _high_water_mark = n;
        }

        return _already_calculated[n];
    }

private:
    inline static std::unordered_map<uint64_t, uint64_t> _already_calculated { {0, 0}, {1, 1}, {2, 1} };
    inline static uint64_t _high_water_mark { 2 };
};

template<uint64_t N>
struct compile_time_fibonacci
{
    enum : uint64_t
    {
        value = compile_time_fibonacci<N - 1>::value + compile_time_fibonacci<N - 2>::value
    };
};

template<>
struct compile_time_fibonacci<0>
{
    enum : uint64_t
    {
        value = 0
    };
};

template<>
struct compile_time_fibonacci<1>
{
    enum : uint64_t
    {
        value = 1
    };
};

template<>
struct compile_time_fibonacci<2>
{
    enum : uint64_t
    {
        value = 1
    };
};

}

}
