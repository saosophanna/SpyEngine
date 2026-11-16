#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace spy {

namespace detail {

constexpr char asciiLower(char character) noexcept
{
    if (character >= 'A' && character <= 'Z')
    {
        return static_cast<char>(character - 'A' + 'a');
    }

    return character;
}

constexpr char asciiUpper(char character) noexcept
{
    if (character >= 'a' && character <= 'z')
    {
        return static_cast<char>(character - 'a' + 'A');
    }

    return character;
}

constexpr bool asciiSpace(char character) noexcept
{
    return character == ' ' || character == '\t' || character == '\n' || character == '\r' ||
           character == '\f' || character == '\v';
}

}

[[nodiscard]] constexpr bool startsWith(std::string_view text, std::string_view prefix) noexcept
{
    return text.size() >= prefix.size() && text.substr(0, prefix.size()) == prefix;
}

[[nodiscard]] constexpr bool endsWith(std::string_view text, std::string_view suffix) noexcept
{
    return text.size() >= suffix.size() && text.substr(text.size() - suffix.size()) == suffix;
}

[[nodiscard]] constexpr bool contains(std::string_view text, std::string_view needle) noexcept
{
    return text.find(needle) != std::string_view::npos;
}

[[nodiscard]] constexpr std::string_view trimLeft(std::string_view text) noexcept
{
    std::size_t offset = 0;

    while (offset < text.size() && detail::asciiSpace(text[offset]))
    {
        ++offset;
    }

    return text.substr(offset);
}

[[nodiscard]] constexpr std::string_view trimRight(std::string_view text) noexcept
{
    std::size_t size = text.size();

    while (size > 0 && detail::asciiSpace(text[size - 1]))
    {
        --size;
    }

    return text.substr(0, size);
}

[[nodiscard]] constexpr std::string_view trim(std::string_view text) noexcept
{
    return trimRight(trimLeft(text));
}

[[nodiscard]] inline std::string toLowerAscii(std::string_view text)
{
    std::string result(text);

    for (char& character : result)
    {
        character = detail::asciiLower(character);
    }

    return result;
}

[[nodiscard]] inline std::string toUpperAscii(std::string_view text)
{
    std::string result(text);

    for (char& character : result)
    {
        character = detail::asciiUpper(character);
    }

    return result;
}

[[nodiscard]] constexpr bool equalsIgnoreCaseAscii(std::string_view lhs,
                                                   std::string_view rhs) noexcept
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }

    for (std::size_t index = 0; index < lhs.size(); ++index)
    {
        if (detail::asciiLower(lhs[index]) != detail::asciiLower(rhs[index]))
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] inline std::vector<std::string_view> split(std::string_view text, char delimiter)
{
    std::vector<std::string_view> parts;

    if (text.empty())
    {
        return parts;
    }

    std::size_t start = 0;

    while (true)
    {
        const std::size_t end = text.find(delimiter, start);

        if (end == std::string_view::npos)
        {
            parts.push_back(text.substr(start));
            break;
        }

        parts.push_back(text.substr(start, end - start));
        start = end + 1;
    }

    return parts;
}

[[nodiscard]] inline std::vector<std::string_view> split(std::string_view text,
                                                         std::string_view delimiter)
{
    std::vector<std::string_view> parts;

    if (text.empty())
    {
        return parts;
    }

    if (delimiter.empty())
    {
        parts.push_back(text);
        return parts;
    }

    std::size_t start = 0;

    while (true)
    {
        const std::size_t end = text.find(delimiter, start);

        if (end == std::string_view::npos)
        {
            parts.push_back(text.substr(start));
            break;
        }

        parts.push_back(text.substr(start, end - start));
        start = end + delimiter.size();
    }

    return parts;
}

[[nodiscard]] inline std::string join(std::span<const std::string_view> parts,
                                      std::string_view separator)
{
    if (parts.empty())
    {
        return {};
    }

    std::size_t total = 0;

    for (std::string_view part : parts)
    {
        total += part.size();
    }

    total += separator.size() * (parts.size() - 1);

    std::string result;
    result.reserve(total);

    for (std::size_t index = 0; index < parts.size(); ++index)
    {
        if (index > 0)
        {
            result.append(separator);
        }

        result.append(parts[index]);
    }

    return result;
}

inline bool replaceAll(std::string& text, std::string_view from, std::string_view to)
{
    if (from.empty())
    {
        return false;
    }

    bool replaced = false;
    std::size_t position = 0;

    while ((position = text.find(from, position)) != std::string::npos)
    {
        text.replace(position, from.size(), to);
        position += to.size();
        replaced = true;
    }

    return replaced;
}

}
