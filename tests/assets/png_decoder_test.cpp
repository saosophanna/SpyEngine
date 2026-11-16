#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string>
#include <vector>

#include "assets/image/png_decoder.hpp"

namespace
{

int g_failures = 0;

void check(const bool condition, const char* name)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAILED: %s\n", name);
        ++g_failures;
    }
}

std::array<std::uint32_t, 256> makeCrcTable()
{
    std::array<std::uint32_t, 256> table{};

    for (std::uint32_t i = 0; i < 256; ++i)
    {
        std::uint32_t value = i;
        for (int bit = 0; bit < 8; ++bit)
        {
            if ((value & 1u) != 0)
            {
                value = 0xEDB88320u ^ (value >> 1);
            }
            else
            {
                value >>= 1;
            }
        }
        table[i] = value;
    }
    return table;
}

const std::array<std::uint32_t, 256>& crcTable()
{
    static const std::array<std::uint32_t, 256> table = makeCrcTable();
    return table;
}

std::uint32_t computeCrc32(const std::uint8_t* data, const std::size_t size)
{
    const std::array<std::uint32_t, 256>& table = crcTable();
    std::uint32_t crc = 0xFFFFFFFFu;

    for (std::size_t i = 0; i < size; ++i)
    {
        crc = table[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFu;
}

void appendU32(std::vector<std::uint8_t>& output, const std::uint32_t value)
{
    output.push_back(static_cast<std::uint8_t>((value >> 24) & 0xFFu));
    output.push_back(static_cast<std::uint8_t>((value >> 16) & 0xFFu));
    output.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFFu));
    output.push_back(static_cast<std::uint8_t>(value & 0xFFu));
}

void appendChunk(std::vector<std::uint8_t>& png,
                 const char* type,
                 const std::vector<std::uint8_t>& data)
{
    appendU32(png, static_cast<std::uint32_t>(data.size()));

    const std::size_t typeOffset = png.size();
    for (std::size_t i = 0; i < 4; ++i)
    {
        png.push_back(static_cast<std::uint8_t>(type[i]));
    }
    png.insert(png.end(), data.begin(), data.end());

    const std::uint32_t crc = computeCrc32(png.data() + typeOffset, 4 + data.size());
    appendU32(png, crc);
}

std::vector<std::uint8_t> zlibStore(const std::vector<std::uint8_t>& raw)
{
    std::vector<std::uint8_t> data;
    data.push_back(0x78);
    data.push_back(0x01);
    data.push_back(0x01);

    const std::uint16_t length = static_cast<std::uint16_t>(raw.size());
    const std::uint16_t inverse = static_cast<std::uint16_t>(~length);

    data.push_back(static_cast<std::uint8_t>(length & 0xFFu));
    data.push_back(static_cast<std::uint8_t>((length >> 8) & 0xFFu));
    data.push_back(static_cast<std::uint8_t>(inverse & 0xFFu));
    data.push_back(static_cast<std::uint8_t>((inverse >> 8) & 0xFFu));
    data.insert(data.end(), raw.begin(), raw.end());
    return data;
}

std::vector<std::uint8_t> makePng(const std::uint32_t width,
                                  const std::uint32_t height,
                                  const std::uint8_t colorType,
                                  const std::vector<std::uint8_t>& raw,
                                  const std::vector<std::uint8_t>& palette = {},
                                  const std::vector<std::uint8_t>& transparency = {})
{
    std::vector<std::uint8_t> png = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

    std::vector<std::uint8_t> header;
    appendU32(header, width);
    appendU32(header, height);
    header.push_back(8);
    header.push_back(colorType);
    header.push_back(0);
    header.push_back(0);
    header.push_back(0);
    appendChunk(png, "IHDR", header);

    if (!palette.empty())
    {
        appendChunk(png, "PLTE", palette);
    }
    if (!transparency.empty())
    {
        appendChunk(png, "tRNS", transparency);
    }

    appendChunk(png, "IDAT", zlibStore(raw));

    const std::vector<std::uint8_t> empty;
    appendChunk(png, "IEND", empty);
    return png;
}

spy::assets::Image decode(const std::vector<std::uint8_t>& png)
{
    return spy::assets::decodePng(std::span<const std::uint8_t>(png.data(), png.size()));
}

void appendRow(std::vector<std::uint8_t>& raw,
               const std::uint8_t filter,
               const std::vector<std::uint8_t>& row)
{
    raw.push_back(filter);
    raw.insert(raw.end(), row.begin(), row.end());
}

std::uint32_t paeth(const std::uint32_t left,
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
        return left;
    }
    if (distanceUp <= distanceUpLeft)
    {
        return up;
    }
    return upLeft;
}

std::vector<std::uint8_t> encodeRow(const std::uint8_t filter,
                                    const std::vector<std::uint8_t>& row,
                                    const std::vector<std::uint8_t>& previous,
                                    const std::size_t bytesPerPixel)
{
    std::vector<std::uint8_t> encoded(row.size());

    for (std::size_t x = 0; x < row.size(); ++x)
    {
        const std::uint32_t left = x >= bytesPerPixel ? row[x - bytesPerPixel] : 0;
        const std::uint32_t up = previous.empty() ? 0 : previous[x];
        const std::uint32_t upLeft =
            previous.empty() || x < bytesPerPixel ? 0 : previous[x - bytesPerPixel];

        std::uint32_t predictor = 0;
        switch (filter)
        {
        case 0:
            predictor = 0;
            break;
        case 1:
            predictor = left;
            break;
        case 2:
            predictor = up;
            break;
        case 3:
            predictor = (left + up) / 2;
            break;
        case 4:
            predictor = paeth(left, up, upLeft);
            break;
        default:
            break;
        }

        encoded[x] = static_cast<std::uint8_t>((row[x] - predictor) & 0xFFu);
    }
    return encoded;
}

std::size_t findChunkDataOffset(const std::vector<std::uint8_t>& png, const char* type)
{
    std::size_t offset = 8;

    while (offset + 12 <= png.size())
    {
        const std::uint32_t length = (static_cast<std::uint32_t>(png[offset]) << 24) |
                                     (static_cast<std::uint32_t>(png[offset + 1]) << 16) |
                                     (static_cast<std::uint32_t>(png[offset + 2]) << 8) |
                                     static_cast<std::uint32_t>(png[offset + 3]);

        bool matches = true;
        for (std::size_t i = 0; i < 4; ++i)
        {
            if (png[offset + 4 + i] != static_cast<std::uint8_t>(type[i]))
            {
                matches = false;
            }
        }

        if (matches)
        {
            return offset + 8;
        }
        offset += 12 + length;
    }
    return png.size();
}

void testRgba()
{
    std::vector<std::uint8_t> raw;
    appendRow(raw, 0, {255, 0, 0, 255, 0, 255, 0, 255});
    appendRow(raw, 0, {0, 0, 255, 255, 255, 255, 0, 128});

    const spy::assets::Image image = decode(makePng(2, 2, 6, raw));

    check(image.valid(), "rgba image is valid");
    check(image.width == 2 && image.height == 2, "rgba dimensions are 2x2");

    const std::vector<std::uint8_t> expected = {255,
                                                0,
                                                0,
                                                255,
                                                0,
                                                255,
                                                0,
                                                255,
                                                0,
                                                0,
                                                255,
                                                255,
                                                255,
                                                255,
                                                0,
                                                128};
    check(image.pixels == expected, "rgba pixels match");
}

void testRgb()
{
    std::vector<std::uint8_t> raw;
    appendRow(raw, 0, {10, 20, 30, 40, 50, 60});

    const spy::assets::Image image = decode(makePng(2, 1, 2, raw));

    check(image.valid(), "rgb image is valid");
    check(image.width == 2 && image.height == 1, "rgb dimensions are 2x1");

    const std::vector<std::uint8_t> expected = {10, 20, 30, 255, 40, 50, 60, 255};
    check(image.pixels == expected, "rgb pixels expand with opaque alpha");
}

void testGrayscale()
{
    std::vector<std::uint8_t> raw;
    appendRow(raw, 0, {0, 64});
    appendRow(raw, 0, {128, 255});

    const spy::assets::Image image = decode(makePng(2, 2, 0, raw));

    check(image.valid(), "grayscale image is valid");

    const std::vector<std::uint8_t> expected = {0,   0,   0,   255, 64,  64,  64,  255,
                                                128, 128, 128, 255, 255, 255, 255, 255};
    check(image.pixels == expected, "grayscale pixels expand to rgba");
}

void testPalette()
{
    std::vector<std::uint8_t> raw;
    appendRow(raw, 0, {0, 1});

    const std::vector<std::uint8_t> palette = {255, 0, 0, 0, 0, 255};
    const std::vector<std::uint8_t> transparency = {128};

    const spy::assets::Image image = decode(makePng(2, 1, 3, raw, palette, transparency));

    check(image.valid(), "palette image is valid");

    const std::vector<std::uint8_t> expected = {255, 0, 0, 128, 0, 0, 255, 255};
    check(image.pixels == expected, "palette pixels use trns alpha");
}

void testFilters()
{
    const std::vector<std::vector<std::uint8_t>> rows = {
        {10, 20, 30, 40, 15, 25, 35, 45},
        {100, 110, 120, 130, 5, 6, 7, 8},
    };

    std::vector<std::uint8_t> expected;
    for (const std::vector<std::uint8_t>& row : rows)
    {
        expected.insert(expected.end(), row.begin(), row.end());
    }

    const char* const names[5] = {
        "none filter",
        "sub filter",
        "up filter",
        "average filter",
        "paeth filter",
    };

    for (std::uint8_t filter = 0; filter <= 4; ++filter)
    {
        std::vector<std::uint8_t> raw;

        for (std::size_t y = 0; y < rows.size(); ++y)
        {
            const std::vector<std::uint8_t> previous =
                y == 0 ? std::vector<std::uint8_t>{} : rows[y - 1];
            raw.push_back(filter);
            const std::vector<std::uint8_t> encoded = encodeRow(filter, rows[y], previous, 4);
            raw.insert(raw.end(), encoded.begin(), encoded.end());
        }

        const spy::assets::Image image = decode(makePng(2, 2, 6, raw));
        const std::size_t nameIndex = static_cast<std::size_t>(filter);

        check(image.valid(), names[nameIndex]);
        check(image.pixels == expected, names[nameIndex]);
    }
}

void testOnePixel()
{
    std::vector<std::uint8_t> raw;
    appendRow(raw, 0, {1, 2, 3, 4});

    const spy::assets::Image image = decode(makePng(1, 1, 6, raw));

    check(image.valid(), "one pixel image is valid");

    const std::vector<std::uint8_t> expected = {1, 2, 3, 4};
    check(image.pixels == expected, "one pixel values match");
}

void testCorrupt()
{
    std::vector<std::uint8_t> raw;
    appendRow(raw, 0, {9, 8, 7, 6});
    const std::vector<std::uint8_t> valid = makePng(1, 1, 6, raw);

    check(!decode(std::vector<std::uint8_t>{}).valid(), "empty data is rejected");

    std::vector<std::uint8_t> wrongSignature = valid;
    wrongSignature[0] = 0;
    check(!decode(wrongSignature).valid(), "wrong signature is rejected");

    std::vector<std::uint8_t> truncated = valid;
    truncated.resize(valid.size() / 2);
    check(!decode(truncated).valid(), "truncated data is rejected");

    std::vector<std::uint8_t> badLength = valid;
    badLength[8] = 0xFF;
    badLength[9] = 0xFF;
    badLength[10] = 0xFF;
    badLength[11] = 0xFF;
    check(!decode(badLength).valid(), "oversized chunk is rejected");

    std::vector<std::uint8_t> badDeflate = makePng(1, 1, 6, raw);
    const std::size_t idat = findChunkDataOffset(badDeflate, "IDAT");
    check(idat + 7 <= badDeflate.size(), "idat chunk is located");

    if (idat + 7 <= badDeflate.size())
    {
        badDeflate[idat + 6] ^= 0xFF;
        check(!decode(badDeflate).valid(), "corrupt deflate data is rejected");
    }

    std::vector<std::uint8_t> badDepth = valid;
    badDepth[24] = 16;
    check(!decode(badDepth).valid(), "unsupported bit depth is rejected");

    std::vector<std::uint8_t> interlaced = valid;
    interlaced[28] = 1;
    check(!decode(interlaced).valid(), "interlaced images are rejected");

    std::vector<std::uint8_t> badColorType = valid;
    badColorType[25] = 1;
    check(!decode(badColorType).valid(), "unsupported color type is rejected");

    std::vector<std::uint8_t> missingEnd = valid;
    missingEnd.resize(valid.size() - 12);
    check(!decode(missingEnd).valid(), "missing iend is rejected");
}

#if defined(SPY_SOURCE_DIR)
void testApplePng()
{
    const std::string path =
        std::string(SPY_SOURCE_DIR) + "/games/fruit_spin/assets/textures/fruit/apple.png";
    const spy::assets::Image image = spy::assets::decodePngFile(path);

    check(image.valid(), "apple.png decodes");
    check(image.width == 280, "apple.png width is 280");
    check(image.height == 280, "apple.png height is 280");
    check(image.pixels.size() == 280u * 280u * 4u, "apple.png pixel byte count");
}
#endif

}

int main()
{
    testRgba();
    testRgb();
    testGrayscale();
    testPalette();
    testFilters();
    testOnePixel();
    testCorrupt();

#if defined(SPY_SOURCE_DIR)
    testApplePng();
#endif

    std::printf("%s\n",
                g_failures == 0 ? "png decoder tests passed" : "png decoder tests failed");
    return g_failures == 0 ? 0 : 1;
}
