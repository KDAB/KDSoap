/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2025 Jonathan Brady <jtjbrady@users.noreply.github.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef BYTEARRAYVIEWSPLITTER_H
#define BYTEARRAYVIEWSPLITTER_H

#include <QByteArrayView>
#include <QDebug>
#include <iterator>
#include <type_traits>
#if defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L
#include <ranges>
#endif

// A zero-copy byte-splitting view over a QByteArrayView.
// Splits by a character or QByteArrayView delimiter, with optional skipping of empty parts.
template<typename Delim>
class ByteArrayViewSplitter
#if defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L
    : public std::ranges::view_interface<ByteArrayViewSplitter<Delim>>
#endif
{
    using DelimType = std::decay_t<Delim>;

    // Enforce supported delimiter types: byte values or QByteArrayView
    static_assert(
        std::is_same_v<DelimType, char> || std::is_same_v<DelimType, signed char> || std::is_same_v<DelimType, unsigned char> || std::is_same_v<DelimType, std::byte> || std::is_same_v<DelimType, QByteArrayView>,
        "Delimiter must be a valid QByteArrayView byte type or QByteArrayView itself");

public:
    ByteArrayViewSplitter(QByteArrayView origin, Delim delim, Qt::SplitBehavior skipEmpty = Qt::SplitBehaviorFlags::KeepEmptyParts)
        : m_origin(origin)
        , m_delim(delim)
        , m_skipEmpty(skipEmpty == Qt::SplitBehaviorFlags::SkipEmptyParts)
    {
    }

    class iterator
    {
        // Returns 1 for char-like delimiters, or size() for QByteArrayView delimiters
        static constexpr qsizetype computeDelimSize(const DelimType &delim)
        {
            if constexpr (std::is_same_v<DelimType, QByteArrayView>)
                return delim.size();
            else
                return 1;
        }

    public:
        using value_type = QByteArrayView;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        iterator() = default;

        iterator(QByteArrayView origin, Delim delim, qsizetype offset, bool skipEmpty)
            : m_origin(origin)
            , m_delim(delim)
            , m_delimSize(computeDelimSize(delim))
            , m_offset(offset)
            , m_skipEmpty(skipEmpty)
        {
            if (m_offset < 0) {
                // Marks end iterator
                m_offset = m_nextOffset = -1;
            } else {
                // Start at offset, advance once to initialize m_nextOffset
                m_nextOffset = m_offset;
                do {
                    advance();
                } while (m_skipEmpty && m_offset >= 0 && m_offset + m_delimSize == m_nextOffset); // Skip empty parts
            }
        }

        // Returns the current part
        QByteArrayView operator*() const
        {
            if (m_offset < 0)
                return {};

            const char *start = m_origin.data() + m_offset;
            const char *end = m_origin.data() + m_nextOffset - m_delimSize;

            return QByteArrayView(start, end - start);
        }

        // Advance to the next part
        iterator &operator++()
        {
            do {
                advance();
            } while (m_skipEmpty && m_offset >= 0 && m_offset + m_delimSize == m_nextOffset); // Skip empty parts

            return *this;
        }

#if defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L
        // C++20 sentinel-based end comparison
        bool operator==(std::default_sentinel_t) const
        {
            return m_offset < 0;
        }
#endif

        // Equality comparison for pre-C++20 use
        bool operator==(const iterator &other) const
        {
            return m_offset == other.m_offset && m_origin.data() == other.m_origin.data() && m_origin.size() == other.m_origin.size();
        }

        bool operator!=(const iterator &other) const
        {
            return !(*this == other);
        }

    private:
        // Advances m_offset and m_nextOffset to the next segment
        void advance()
        {
            m_offset = m_nextOffset;

            if (m_offset > m_origin.size()) {
                m_offset = -1; // Marks end
                return;
            }

            // Find next delimiter
            const qsizetype delimPos = m_origin.indexOf(m_delim, m_offset);
            if (delimPos != -1) {
                m_nextOffset = delimPos + m_delimSize;
            } else {
                // No more delimiters: consume rest of input
                m_nextOffset = m_origin.size() + m_delimSize;
            }
        }

        const QByteArrayView m_origin;
        const DelimType m_delim;
        const qsizetype m_delimSize;
        qsizetype m_offset = 0; // Start of current part
        qsizetype m_nextOffset = 0; // End of current part (includes delimiter)
        const bool m_skipEmpty = false;
    };

    iterator begin() const
    {
        return iterator {m_origin, m_delim, qsizetype(0), m_skipEmpty};
    }

#if defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L
    // C++20 sentinel-based end
    std::default_sentinel_t end() const noexcept
    {
        return {};
    }
#else
    // Classic iterator-based end
    iterator end() const noexcept
    {
        return iterator {m_origin, m_delim, qsizetype(-1), m_skipEmpty};
    }
#endif

private:
    const QByteArrayView m_origin;
    const DelimType m_delim;
    const bool m_skipEmpty;
};

#endif
