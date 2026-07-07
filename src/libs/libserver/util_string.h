#pragma once

/**
 * @file util_string.h
 * @brief 字符串工具函数集合，提供大小写无关比较、格式化、修剪、
 *        替换和分割等常用字符串操作。
 *
 * 所有函数均定义在 strutil 命名空间下，以 inline 方式实现，
 * 适合在头文件中直接包含使用，无需额外的编译单元。
 */

#include <string>
#include <vector>
#include <cstdarg>

#include "common.h"

namespace strutil
{
    /**
     * @brief 大小写无关的字符串比较（C 风格字符串）。
     *
     * 对两个以空字符结尾的 C 字符串进行逐字符比较，忽略英文字母的大小写差异。
     * 底层调用引擎提供的 engine_stricmp 实现，具体行为（如区域设置敏感性）
     * 取决于该引擎函数的实现。
     *
     * @param c1 第一个待比较的 C 字符串指针。
     * @param c2 第二个待比较的 C 字符串指针。
     * @return int 比较结果：
     *             - < 0 表示 c1 小于 c2
     *             - 0 表示 c1 等于 c2
     *             - > 0 表示 c1 大于 c2
     */
    inline int stricmp(char const *c1, char const *c2)
    {
        return engine_stricmp(c1, c2);
    }

    /**
     * @brief 使用 printf 风格的格式化字符串生成 std::string。
     *
     * 接受与 printf 系列函数相同的格式说明符和可变参数，
     * 返回格式化后的 std::string 对象。
     *
     * 实现细节：
     *   1. 先用 std::vsnprintf(nullptr, 0, ...) 计算所需的字符数，
     *      此调用不写入任何数据，仅返回需要的缓冲区大小。
     *   2. 确保目标字符串容量足够后，再通过第二次
     *      std::vsnprintf 调用将格式化结果写入字符串缓冲区。
     *   3. 使用 va_copy 复制可变参数列表，因为同一个 va_list
     *      在 vsnprintf 调用后状态会改变，不能直接复用。
     *
     * @param _format printf 风格的格式字符串。
     * @param ...     与格式字符串中说明符对应的可变参数列表。
     * @return std::string 格式化后的字符串结果。
     */
    inline std::string format(const char *_format, ...)
    {
        std::string str;

        // 初始化可变参数列表（第一次遍历，用于计算长度）
        va_list args1;
        va_start(args1, _format);

        // 复制可变参数列表（第二次遍历，用于实际格式化）
        va_list args2;
        va_copy(args2, args1);

        // 第一次 vsnprintf：仅计算需要的字符数，不写入数据
        const size_t num_of_chars = std::vsnprintf(nullptr, 0, _format, args1);
        if (num_of_chars > str.capacity())
        {
            // +1 为结尾的空字符预留空间
            str.resize(num_of_chars + 1);
        }

        // 第二次 vsnprintf：将格式化结果实际写入字符串缓冲区
        std::vsnprintf(const_cast<char *>(str.data()), str.capacity(), _format, args2);

        // 清理可变参数列表
        va_end(args1);
        va_end(args2);

        return str;
    }

    /**
     * @brief 移除字符串首尾的空白字符。
     *
     * 从字符串的两端分别扫描空白字符（通过 isspace 判断），
     * 返回一个裁剪后的新字符串，原字符串保持不变。
     *
     * @param s 待修剪的源字符串。
     * @return std::string 去除首尾空白后的新字符串。
     *                     如果源字符串全为空白，则返回空字符串。
     */
    inline std::string trim(const std::string &s)
    {
        // 从左向右找到第一个非空白字符
        std::string::const_iterator it = s.begin();
        while (it != s.end() && isspace(*it))
            ++it;

        // 从右向左找到最后一个非空白字符
        std::string::const_reverse_iterator rit = s.rbegin();
        while (rit.base() != it && isspace(*rit))
            ++rit;

        // 构造并返回 [it, rit.base()) 范围内的子串
        // 注意：rit.base() 是反向迭代器对应的正向迭代器
        return std::string(it, rit.base());
    }

    /**
     * @brief 替换字符串中所有出现的指定模式。
     *
     * 在源字符串 str 中原地查找所有 pattern 子串，并用 newpat 替换。
     * 替换操作会修改原字符串。每次替换后，搜索位置从替换后的新内容
     * 末尾继续，避免对刚替换进去的内容进行二次匹配。
     *
     * @note 该函数不会对 pattern 和 newpat 中可能存在的重叠进行特殊处理。
     *       例如，将 "aaa" 中的 "aa" 替换为 "ba" 会产生 "baa" 而非 "bba"。
     *
     * @param str     要进行替换操作的源字符串（原地修改）。
     * @param pattern 需要被替换的子串。
     * @param newpat  用于替换的新子串。
     * @return int    成功替换的次数。
     */
    inline int replace(std::string &str, const std::string &pattern, const std::string &newpat)
    {
        int count = 0;
        const size_t nsize = newpat.size();  // 替换后的子串长度
        const size_t psize = pattern.size(); // 被替换的子串长度

        // 从位置 0 开始查找，每次替换后跳过新内容继续向后查找
        for (size_t pos = str.find(pattern, 0);
             pos != std::string::npos;
             pos = str.find(pattern, pos + nsize))
        {
            str.replace(pos, psize, newpat);
            count++;
        }

        return count;
    }

    /**
     * @brief 按分隔符拆分字符串。
     *
     * 以字符 c 为分隔符，将源字符串 s 拆分为若干子串，
     * 结果追加到输出向量 v 的末尾。连续的分隔符会产生空子串，
     * 空子串不会被添加到输出结果中。
     *
     * 该函数是泛型实现，支持 std::string（char）和 std::wstring（wchar_t）。
     *
     * @tparam T     字符类型（char 或 wchar_t）。
     * @param s      待拆分的源字符串（不修改）。
     * @param c      用作分隔符的字符。
     * @param v      [out] 接收拆分结果的向量，结果追加到现有元素之后。
     */
    template <typename T>
    inline void split(const std::basic_string<T> &s, T c, std::vector<std::basic_string<T>> &v)
    {
        // 空字符串直接返回，不添加任何元素
        if (s.size() == 0)
            return;

        // i: 当前子串的起始位置
        // j: 下一个分隔符的位置
        typename std::basic_string<T>::size_type i = 0;
        typename std::basic_string<T>::size_type j = s.find(c);

        // 循环提取每个分隔符之前的子串
        while (j != std::basic_string<T>::npos)
        {
            // 提取 [i, j) 范围内的子串
            std::basic_string<T> buf = s.substr(i, j - i);

            // 跳过连续分隔符产生的空子串
            if (buf.size() > 0)
                v.push_back(buf);

            // 移动起始位置到分隔符之后，继续查找下一个分隔符
            i = ++j;
            j = s.find(c, j);
        }

        // 处理最后一个分隔符之后的剩余部分（或整个字符串，如果没有分隔符）
        if (j == std::basic_string<T>::npos)
        {
            std::basic_string<T> buf = s.substr(i, s.length() - i);
            if (buf.size() > 0)
                v.push_back(buf);
        }
    }
}
