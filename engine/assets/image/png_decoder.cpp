#include "assets/image/png_decoder.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace spy::assets
{
namespace
{

constexpr std::uint8_t kPngSignature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
constexpr std::size_t kPngSignatureSize = 8;
constexpr std::size_t kMaxDecodedBytes = std::size_t{1} << 30;
constexpr std::size_t kMaxCodeLength = 15;

std::uint32_t readBigEndian32(const std::span<const std::uint8_t> data, const std::size_t offset)
{
    return (static_cast<std::uint32_t>(data[offset]) << 24) |
           (static_cast<std::uint32_t>(data[offset + 1]) << 16) |
           (static_cast<std::uint32_t>(data[offset + 2]) << 8) |
           static_cast<std::uint32_t>(data[offset + 3]);
}

bool chunkTypeEquals(const std::uint8_t* type, const char* expected)
{
    for (std::size_t i = 0; i < 4; ++i)
    {
        if (type[i] != static_cast<std::uint8_t>(expected[i]))
        {
            return false;
        }
    }
    return true;
}

std::size_t channelsForColorType(const std::uint8_t colorType)
{
    switch (colorType)
    {
    case 0:
        return 1;
    case 2:
        return 3;
    case 3:
        return 1;
    case 4:
        return 2;
    case 6:
        return 4;
    default:
        return 0;
    }
}

class BitReader
{
public:
    explicit BitReader(const std::span<const std::uint8_t> data)
        : m_data(data)
    {
    }

    bool readBits(const std::uint32_t count, std::uint32_t& value)
    {
        while (m_bitCount < count)
        {
            if (m_position >= m_data.size())
            {
                return false;
            }
            m_buffer |= static_cast<std::uint64_t>(m_data[m_position]) << m_bitCount;
            ++m_position;
            m_bitCount += 8;
        }

        const std::uint64_t mask = (std::uint64_t{1} << count) - 1;
        value = static_cast<std::uint32_t>(m_buffer & mask);
        m_buffer >>= count;
        m_bitCount -= count;
        return true;
    }

    bool alignToByte()
    {
        const std::uint32_t remainder = m_bitCount % 8;
        m_buffer >>= remainder;
        m_bitCount -= remainder;
        return true;
    }

private:
    std::span<const std::uint8_t> m_data;
    std::size_t m_position = 0;
    std::uint64_t m_buffer = 0;
    std::uint32_t m_bitCount = 0;
};

struct HuffmanTable
{
    std::uint16_t counts[16]{};
    std::uint16_t symbols[288]{};
};

bool buildHuffman(const std::uint8_t* lengths, const std::size_t count, HuffmanTable& table)
{
    for (std::size_t i = 0; i < 16; ++i)
    {
        table.counts[i] = 0;
    }

    for (std::size_t i = 0; i < count; ++i)
    {
        if (lengths[i] > kMaxCodeLength)
        {
            return false;
        }
        ++table.counts[lengths[i]];
    }

    std::int32_t remaining = 1;
    for (std::size_t length = 1; length <= kMaxCodeLength; ++length)
    {
        remaining <<= 1;
        remaining -= table.counts[length];
        if (remaining < 0)
        {
            return false;
        }
    }

    std::uint16_t offsets[16]{};
    offsets[1] = 0;
    for (std::size_t length = 1; length < kMaxCodeLength; ++length)
    {
        offsets[length + 1] = static_cast<std::uint16_t>(offsets[length] + table.counts[length]);
    }

    for (std::size_t i = 0; i < count; ++i)
    {
        if (lengths[i] != 0)
        {
            table.symbols[offsets[lengths[i]]] = static_cast<std::uint16_t>(i);
            ++offsets[lengths[i]];
        }
    }
    return true;
}

bool decodeSymbol(BitReader& reader, const HuffmanTable& table, std::int32_t& symbol)
{
    std::int32_t code = 0;
    std::int32_t first = 0;
    std::int32_t index = 0;

    for (std::int32_t length = 1; length <= 15; ++length)
    {
        std::uint32_t bit = 0;
        if (!reader.readBits(1, bit))
        {
            return false;
        }

        code |= static_cast<std::int32_t>(bit);
        const std::int32_t count = table.counts[length];
        if (code - first < count)
        {
            symbol = table.symbols[static_cast<std::size_t>(index + (code - first))];
            return true;
        }

        index += count;
        first = (first + count) << 1;
        code <<= 1;
    }
    return false;
}

bool buildFixedTables(HuffmanTable& literalTable, HuffmanTable& distanceTable)
{
    std::uint8_t literalLengths[288]{};
    for (std::size_t i = 0; i < 144; ++i)
    {
        literalLengths[i] = 8;
    }
    for (std::size_t i = 144; i < 256; ++i)
    {
        literalLengths[i] = 9;
    }
    for (std::size_t i = 256; i < 280; ++i)
    {
        literalLengths[i] = 7;
    }
    for (std::size_t i = 280; i < 288; ++i)
    {
        literalLengths[i] = 8;
    }

    std::uint8_t distanceLengths[32]{};
    for (std::size_t i = 0; i < 32; ++i)
    {
        distanceLengths[i] = 5;
    }

    if (!buildHuffman(literalLengths, 288, literalTable))
    {
        return false;
    }
    return buildHuffman(distanceLengths, 32, distanceTable);
}

bool readDynamicTables(BitReader& reader, HuffmanTable& literalTable, HuffmanTable& distanceTable)
{
    std::uint32_t literalBits = 0;
    std::uint32_t distanceBits = 0;
    std::uint32_t codeLengthBits = 0;

    if (!reader.readBits(5, literalBits) || !reader.readBits(5, distanceBits) ||
        !reader.readBits(4, codeLengthBits))
    {
        return false;
    }

    const std::size_t literalCount = 257 + literalBits;
    const std::size_t distanceCount = 1 + distanceBits;
    const std::size_t codeLengthCount = 4 + codeLengthBits;

    if (literalCount > 286 || distanceCount > 32)
    {
        return false;
    }

    constexpr std::uint8_t kCodeLengthOrder[19] = {16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
                                                   11, 4,  12, 3, 13, 2, 14, 1, 15};
    std::uint8_t codeLengthLengths[19]{};
    for (std::size_t i = 0; i < codeLengthCount; ++i)
    {
        std::uint32_t value = 0;
        if (!reader.readBits(3, value))
        {
            return false;
        }
        codeLengthLengths[kCodeLengthOrder[i]] = static_cast<std::uint8_t>(value);
    }

    HuffmanTable codeLengthTable{};
    if (!buildHuffman(codeLengthLengths, 19, codeLengthTable))
    {
        return false;
    }

    const std::size_t totalCount = literalCount + distanceCount;
    std::uint8_t lengths[288 + 32]{};
    std::size_t index = 0;

    while (index < totalCount)
    {
        std::int32_t symbol = 0;
        if (!decodeSymbol(reader, codeLengthTable, symbol))
        {
            return false;
        }

        if (symbol < 16)
        {
            lengths[index] = static_cast<std::uint8_t>(symbol);
            ++index;
        }
        else if (symbol == 16)
        {
            if (index == 0)
            {
                return false;
            }
            std::uint32_t repeat = 0;
            if (!reader.readBits(2, repeat))
            {
                return false;
            }
            const std::size_t count = 3 + repeat;
            if (count > totalCount - index)
            {
                return false;
            }
            const std::uint8_t previous = lengths[index - 1];
            for (std::size_t i = 0; i < count; ++i)
            {
                lengths[index] = previous;
                ++index;
            }
        }
        else if (symbol == 17 || symbol == 18)
        {
            const bool shortRepeat = symbol == 17;
            std::uint32_t repeat = 0;
            if (!reader.readBits(shortRepeat ? 3 : 7, repeat))
            {
                return false;
            }
            const std::size_t count = (shortRepeat ? 3 : 11) + repeat;
            if (count > totalCount - index)
            {
                return false;
            }
            for (std::size_t i = 0; i < count; ++i)
            {
                lengths[index] = 0;
                ++index;
            }
        }
        else
        {
            return false;
        }
    }

    if (lengths[256] == 0)
    {
        return false;
    }

    if (!buildHuffman(lengths, literalCount, literalTable))
    {
        return false;
    }
    return buildHuffman(lengths + literalCount, distanceCount, distanceTable);
}

bool inflate(const std::span<const std::uint8_t> input,
             const std::size_t expectedSize,
             std::vector<std::uint8_t>& output)
{
    constexpr std::uint16_t kLengthBase[29] = {3,   4,   5,   6,   7,   8,   9,   10,  11,  13,
                                               15,  17,  19,  23,  27,  31,  35,  43,  51,  59,
                                               67,  83,  99,  115, 131, 163, 195, 227, 258};
    constexpr std::uint8_t kLengthExtra[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
                                               2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    constexpr std::uint16_t kDistanceBase[30] = {1,    2,    3,    4,    5,    7,    9,
                                                 13,   17,   25,   33,   49,   65,   97,
                                                 129,  193,  257,  385,  513,  769,  1025,
                                                 1537, 2049, 3073, 4097, 6145, 8193, 12289,
                                                 16385, 24577};
    constexpr std::uint8_t kDistanceExtra[30] = {0, 0, 0,  0,  1,  1,  2,  2,  3,  3,
                                                 4, 4, 5,  5,  6,  6,  7,  7,  8,  8,
                                                 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

    output.clear();
    output.reserve(expectedSize);

    BitReader reader(input);
    HuffmanTable literalTable{};
    HuffmanTable distanceTable{};
    bool finalBlock = false;

    while (!finalBlock)
    {
        std::uint32_t finalBit = 0;
        std::uint32_t blockType = 0;
        if (!reader.readBits(1, finalBit) || !reader.readBits(2, blockType))
        {
            return false;
        }
        finalBlock = finalBit != 0;

        if (blockType == 0)
        {
            reader.alignToByte();

            std::uint32_t length = 0;
            std::uint32_t inverseLength = 0;
            if (!reader.readBits(16, length) || !reader.readBits(16, inverseLength))
            {
                return false;
            }
            if (static_cast<std::uint16_t>(length) !=
                static_cast<std::uint16_t>(~static_cast<std::uint16_t>(inverseLength)))
            {
                return false;
            }

            for (std::uint32_t i = 0; i < length; ++i)
            {
                std::uint32_t value = 0;
                if (!reader.readBits(8, value))
                {
                    return false;
                }
                if (output.size() >= expectedSize)
                {
                    return false;
                }
                output.push_back(static_cast<std::uint8_t>(value));
            }
            continue;
        }

        if (blockType == 1)
        {
            if (!buildFixedTables(literalTable, distanceTable))
            {
                return false;
            }
        }
        else if (blockType == 2)
        {
            if (!readDynamicTables(reader, literalTable, distanceTable))
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        while (true)
        {
            std::int32_t symbol = 0;
            if (!decodeSymbol(reader, literalTable, symbol))
            {
                return false;
            }

            if (symbol < 256)
            {
                if (output.size() >= expectedSize)
                {
                    return false;
                }
                output.push_back(static_cast<std::uint8_t>(symbol));
                continue;
            }

            if (symbol == 256)
            {
                break;
            }

            if (symbol > 285)
            {
                return false;
            }

            const std::size_t lengthIndex = static_cast<std::size_t>(symbol - 257);
            std::uint32_t lengthExtra = 0;
            if (!reader.readBits(kLengthExtra[lengthIndex], lengthExtra))
            {
                return false;
            }
            const std::size_t length = static_cast<std::size_t>(kLengthBase[lengthIndex]) +
                                       lengthExtra;

            std::int32_t distanceSymbol = 0;
            if (!decodeSymbol(reader, distanceTable, distanceSymbol))
            {
                return false;
            }
            if (distanceSymbol < 0 || distanceSymbol > 29)
            {
                return false;
            }

            std::uint32_t distanceExtra = 0;
            if (!reader.readBits(kDistanceExtra[distanceSymbol], distanceExtra))
            {
                return false;
            }
            const std::size_t distance =
                static_cast<std::size_t>(kDistanceBase[distanceSymbol]) + distanceExtra;

            if (distance > output.size())
            {
                return false;
            }
            if (length > expectedSize - output.size())
            {
                return false;
            }

            for (std::size_t i = 0; i < length; ++i)
            {
                output.push_back(output[output.size() - distance]);
            }
        }
    }

    return true;
}

bool inflateZlib(const std::span<const std::uint8_t> data,
                 const std::size_t expectedSize,
                 std::vector<std::uint8_t>& output)
{
    if (data.size() < 2)
    {
        return false;
    }

    const std::uint8_t compressionMethod = data[0];
    const std::uint8_t flags = data[1];

    if ((compressionMethod & 0x0F) != 8)
    {
        return false;
    }
    if ((compressionMethod >> 4) > 7)
    {
        return false;
    }
    if (((static_cast<std::uint32_t>(compressionMethod) << 8) |
         static_cast<std::uint32_t>(flags)) % 31 != 0)
    {
        return false;
    }
    if ((flags & 0x20) != 0)
    {
        return false;
    }

    return inflate(data.subspan(2), expectedSize, output);
}

std::uint8_t paethPredictor(const std::uint32_t left,
                            const std::uint32_t up,
                            const std::uint32_t upLeft)
{
    const std::int32_t prediction = static_cast<std::int32_t>(left) +
                                    static_cast<std::int32_t>(up) -
                                    static_cast<std::int32_t>(upLeft);
    const std::int32_t distanceLeft = prediction > static_cast<std::int32_t>(left)
                                          ? prediction - static_cast<std::int32_t>(left)
                                          : static_cast<std::int32_t>(left) - prediction;
    const std::int32_t distanceUp = prediction > static_cast<std::int32_t>(up)
                                        ? prediction - static_cast<std::int32_t>(up)
                                        : static_cast<std::int32_t>(up) - prediction;
    const std::int32_t distanceUpLeft = prediction > static_cast<std::int32_t>(upLeft)
                                            ? prediction - static_cast<std::int32_t>(upLeft)
                                            : static_cast<std::int32_t>(upLeft) - prediction;

    if (distanceLeft <= distanceUp && distanceLeft <= distanceUpLeft)
    {
        return static_cast<std::uint8_t>(left);
    }
    if (distanceUp <= distanceUpLeft)
    {
        return static_cast<std::uint8_t>(up);
    }
    return static_cast<std::uint8_t>(upLeft);
}

bool unfilter(std::vector<std::uint8_t>& raw,
              const std::size_t width,
              const std::size_t height,
              const std::size_t bytesPerPixel)
{
    const std::size_t rowBytes = width * bytesPerPixel;
    const std::size_t stride = rowBytes + 1;

    for (std::size_t y = 0; y < height; ++y)
    {
        const std::size_t rowOffset = y * stride;
        const std::uint8_t filterType = raw[rowOffset];
        std::uint8_t* row = raw.data() + rowOffset + 1;
        const std::uint8_t* previous = y == 0 ? nullptr : raw.data() + (y - 1) * stride + 1;

        switch (filterType)
        {
        case 0:
            break;
        case 1:
            for (std::size_t x = bytesPerPixel; x < rowBytes; ++x)
            {
                row[x] = static_cast<std::uint8_t>(row[x] + row[x - bytesPerPixel]);
            }
            break;
        case 2:
            if (previous != nullptr)
            {
                for (std::size_t x = 0; x < rowBytes; ++x)
                {
                    row[x] = static_cast<std::uint8_t>(row[x] + previous[x]);
                }
            }
            break;
        case 3:
            for (std::size_t x = 0; x < rowBytes; ++x)
            {
                const std::uint32_t left = x >= bytesPerPixel ? row[x - bytesPerPixel] : 0;
                const std::uint32_t up = previous != nullptr ? previous[x] : 0;
                row[x] = static_cast<std::uint8_t>(row[x] + ((left + up) / 2));
            }
            break;
        case 4:
            for (std::size_t x = 0; x < rowBytes; ++x)
            {
                const std::uint32_t left = x >= bytesPerPixel ? row[x - bytesPerPixel] : 0;
                const std::uint32_t up = previous != nullptr ? previous[x] : 0;
                const std::uint32_t upLeft =
                    previous != nullptr && x >= bytesPerPixel ? previous[x - bytesPerPixel] : 0;
                row[x] = static_cast<std::uint8_t>(row[x] + paethPredictor(left, up, upLeft));
            }
            break;
        default:
            return false;
        }
    }
    return true;
}

}

bool Image::valid() const noexcept
{
    return width != 0 && height != 0 &&
           pixels.size() ==
               static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4;
}

Image decodePng(const std::span<const std::uint8_t> data)
{
    if (data.size() < kPngSignatureSize)
    {
        return {};
    }

    for (std::size_t i = 0; i < kPngSignatureSize; ++i)
    {
        if (data[i] != kPngSignature[i])
        {
            return {};
        }
    }

    struct Header
    {
        std::uint32_t width = 0;
        std::uint32_t height = 0;
        std::uint8_t bitDepth = 0;
        std::uint8_t colorType = 0;
        std::uint8_t compression = 0;
        std::uint8_t filter = 0;
        std::uint8_t interlace = 0;
    };

    Header header{};
    bool sawHeader = false;
    bool sawPalette = false;
    bool sawTransparency = false;
    bool sawData = false;
    bool sawEnd = false;

    std::vector<std::uint8_t> palette;
    std::vector<std::uint8_t> transparency;
    std::vector<std::uint8_t> compressed;

    std::size_t offset = kPngSignatureSize;

    while (offset < data.size())
    {
        if (data.size() - offset < 12)
        {
            return {};
        }

        const std::uint32_t chunkLength = readBigEndian32(data, offset);
        if (static_cast<std::size_t>(chunkLength) > data.size() - offset - 12)
        {
            return {};
        }

        const std::uint8_t* type = data.data() + offset + 4;
        const std::size_t chunkData = offset + 8;
        const std::size_t chunkEnd = chunkData + chunkLength;

        if (chunkTypeEquals(type, "IHDR"))
        {
            if (sawHeader || chunkLength != 13)
            {
                return {};
            }

            header.width = readBigEndian32(data, chunkData);
            header.height = readBigEndian32(data, chunkData + 4);
            header.bitDepth = data[chunkData + 8];
            header.colorType = data[chunkData + 9];
            header.compression = data[chunkData + 10];
            header.filter = data[chunkData + 11];
            header.interlace = data[chunkData + 12];

            if (header.width == 0 || header.height == 0)
            {
                return {};
            }
            if (header.bitDepth != 8)
            {
                return {};
            }
            if (channelsForColorType(header.colorType) == 0)
            {
                return {};
            }
            if (header.compression != 0 || header.filter != 0 || header.interlace != 0)
            {
                return {};
            }

            sawHeader = true;
        }
        else
        {
            if (!sawHeader)
            {
                return {};
            }

            if (chunkTypeEquals(type, "PLTE"))
            {
                if (sawData || sawPalette || chunkLength == 0 || chunkLength % 3 != 0 ||
                    chunkLength > 256 * 3)
                {
                    return {};
                }
                palette.assign(data.data() + chunkData, data.data() + chunkEnd);
                sawPalette = true;
            }
            else if (chunkTypeEquals(type, "tRNS"))
            {
                if (sawData || sawTransparency)
                {
                    return {};
                }
                transparency.assign(data.data() + chunkData, data.data() + chunkEnd);
                sawTransparency = true;
            }
            else if (chunkTypeEquals(type, "IDAT"))
            {
                if (header.colorType == 3 && !sawPalette)
                {
                    return {};
                }
                compressed.insert(compressed.end(),
                                  data.data() + chunkData,
                                  data.data() + chunkEnd);
                sawData = true;
            }
            else if (chunkTypeEquals(type, "IEND"))
            {
                sawEnd = true;
                break;
            }
        }

        offset = chunkEnd + 4;
    }

    if (!sawHeader || !sawData || !sawEnd)
    {
        return {};
    }

    if (header.colorType == 3)
    {
        if (!sawPalette)
        {
            return {};
        }
        if (transparency.size() > palette.size() / 3)
        {
            return {};
        }
    }
    else if (sawTransparency)
    {
        if (header.colorType == 0 && transparency.size() != 2)
        {
            return {};
        }
        if (header.colorType == 2 && transparency.size() != 6)
        {
            return {};
        }
        if (header.colorType == 4 || header.colorType == 6)
        {
            return {};
        }
    }

    const std::size_t width = header.width;
    const std::size_t height = header.height;
    const std::size_t channels = channelsForColorType(header.colorType);

    if (width > kMaxDecodedBytes / channels)
    {
        return {};
    }

    const std::size_t rowBytes = width * channels;
    const std::size_t stride = rowBytes + 1;

    if (height > kMaxDecodedBytes / stride)
    {
        return {};
    }

    const std::size_t rawSize = height * stride;
    const std::size_t pixelCount = width * height;

    if (pixelCount > kMaxDecodedBytes / 4)
    {
        return {};
    }

    std::vector<std::uint8_t> raw;
    if (!inflateZlib(std::span<const std::uint8_t>(compressed.data(), compressed.size()),
                     rawSize,
                     raw))
    {
        return {};
    }
    if (raw.size() != rawSize)
    {
        return {};
    }

    if (!unfilter(raw, width, height, channels))
    {
        return {};
    }

    const bool hasTransparentGray = transparency.size() == 2;

    Image image;
    image.width = header.width;
    image.height = header.height;
    image.pixels.resize(pixelCount * 4);

    std::size_t destination = 0;

    for (std::size_t y = 0; y < height; ++y)
    {
        const std::uint8_t* source = raw.data() + y * stride + 1;

        for (std::size_t x = 0; x < width; ++x)
        {
            switch (header.colorType)
            {
            case 0:
            {
                const std::uint8_t gray = source[x];
                image.pixels[destination] = gray;
                image.pixels[destination + 1] = gray;
                image.pixels[destination + 2] = gray;
                image.pixels[destination + 3] =
                    hasTransparentGray && gray == transparency[1] ? 0 : 255;
                break;
            }
            case 2:
            {
                const std::uint8_t* pixel = source + x * 3;
                const bool transparent = !transparency.empty() && pixel[0] == transparency[1] &&
                                         pixel[1] == transparency[3] &&
                                         pixel[2] == transparency[5];
                image.pixels[destination] = pixel[0];
                image.pixels[destination + 1] = pixel[1];
                image.pixels[destination + 2] = pixel[2];
                image.pixels[destination + 3] = transparent ? 0 : 255;
                break;
            }
            case 3:
            {
                const std::size_t index = source[x];
                if (index * 3 + 2 >= palette.size())
                {
                    return {};
                }
                image.pixels[destination] = palette[index * 3];
                image.pixels[destination + 1] = palette[index * 3 + 1];
                image.pixels[destination + 2] = palette[index * 3 + 2];
                image.pixels[destination + 3] =
                    index < transparency.size() ? transparency[index] : 255;
                break;
            }
            case 4:
            {
                const std::uint8_t* pixel = source + x * 2;
                image.pixels[destination] = pixel[0];
                image.pixels[destination + 1] = pixel[0];
                image.pixels[destination + 2] = pixel[0];
                image.pixels[destination + 3] = pixel[1];
                break;
            }
            default:
            {
                const std::uint8_t* pixel = source + x * 4;
                image.pixels[destination] = pixel[0];
                image.pixels[destination + 1] = pixel[1];
                image.pixels[destination + 2] = pixel[2];
                image.pixels[destination + 3] = pixel[3];
                break;
            }
            }

            destination += 4;
        }
    }

    return image;
}

Image decodePngFile(const std::string_view path)
{
    std::ifstream file(std::string(path), std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        return {};
    }

    const std::streamoff end = static_cast<std::streamoff>(file.tellg());
    if (end <= 0)
    {
        return {};
    }
    if (static_cast<std::uintmax_t>(end) > std::numeric_limits<std::size_t>::max())
    {
        return {};
    }
    if (static_cast<std::uintmax_t>(end) >
        static_cast<std::uintmax_t>(std::numeric_limits<std::streamsize>::max()))
    {
        return {};
    }

    const std::size_t size = static_cast<std::size_t>(end);
    std::vector<std::uint8_t> data(size);

    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size)))
    {
        return {};
    }

    return decodePng(std::span<const std::uint8_t>(data.data(), data.size()));
}

}
