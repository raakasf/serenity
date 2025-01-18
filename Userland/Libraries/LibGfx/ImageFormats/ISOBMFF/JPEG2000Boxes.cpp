/*
 * Copyright (c) 2024, Nico Weber <thakis@chromium.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "JPEG2000Boxes.h"
#include <AK/Function.h>

// Core coding system spec (.jp2 format): T-REC-T.800-201511-S!!PDF-E.pdf available here:
// https://www.itu.int/rec/dologin_pub.asp?lang=e&id=T-REC-T.800-201511-S!!PDF-E&type=items

namespace Gfx::ISOBMFF {

ErrorOr<void> JPEG2000HeaderBox::read_from_stream(BoxStream& stream)
{
    // I.5.3 JP2 Header box (superbox)
    auto make_subbox = [](BoxType type, BoxStream& stream) -> ErrorOr<Optional<NonnullOwnPtr<Box>>> {
        switch (type) {
        case BoxType::JPEG2000ChannelDefinitionBox:
            return TRY(JPEG2000ChannelDefinitionBox::create_from_stream(stream));
        case BoxType::JPEG2000ColorSpecificationBox:
            return TRY(JPEG2000ColorSpecificationBox::create_from_stream(stream));
        case BoxType::JPEG2000ImageHeaderBox:
            return TRY(JPEG2000ImageHeaderBox::create_from_stream(stream));
        case BoxType::JPEG2000PaletteBox:
            return TRY(JPEG2000PaletteBox::create_from_stream(stream));
        case BoxType::JPEG2000ResolutionBox:
            return TRY(JPEG2000ResolutionBox::create_from_stream(stream));
        default:
            return OptionalNone {};
        }
    };

    TRY(SuperBox::read_from_stream(stream, move(make_subbox)));
    return {};
}

void JPEG2000HeaderBox::dump(String const& prepend) const
{
    SuperBox::dump(prepend);
}

ErrorOr<void> JPEG2000ImageHeaderBox::read_from_stream(BoxStream& stream)
{
    // I.5.3.1 Image Header box
    height = TRY(stream.read_value<BigEndian<u32>>());
    width = TRY(stream.read_value<BigEndian<u32>>());
    num_components = TRY(stream.read_value<BigEndian<u16>>());
    bits_per_component = TRY(stream.read_value<u8>());
    compression_type = TRY(stream.read_value<u8>());
    is_colorspace_unknown = TRY(stream.read_value<u8>());
    contains_intellectual_property_rights = TRY(stream.read_value<u8>());
    return {};
}

void JPEG2000ImageHeaderBox::dump(String const& prepend) const
{
    Box::dump(prepend);
    outln("{}- height = {}", prepend, height);
    outln("{}- width = {}", prepend, width);
    outln("{}- num_components = {}", prepend, num_components);
    if (bits_per_component == 0xFF) {
        outln("{}- components vary in bit depth", prepend);
    } else {
        outln("{}- are_components_signed = {}", prepend, (bits_per_component & 0x80) != 0);
        outln("{}- bits_per_component = {}", prepend, (bits_per_component & 0x7f) + 1);
    }
    outln("{}- compression_type = {}", prepend, compression_type);
    outln("{}- is_colorspace_unknown = {}", prepend, is_colorspace_unknown);
    outln("{}- contains_intellectual_property_rights = {}", prepend, contains_intellectual_property_rights);
}

ErrorOr<void> JPEG2000ColorSpecificationBox::read_from_stream(BoxStream& stream)
{
    // I.5.3.3 Colour Specification box
    method = TRY(stream.read_value<u8>());
    precedence = TRY(stream.read_value<i8>());
    approximation = TRY(stream.read_value<u8>());
    if (method == 1)
        enumerated_color_space = TRY(stream.read_value<BigEndian<u32>>());
    if (method == 2) {
        ByteBuffer local_icc_data = TRY(ByteBuffer::create_uninitialized(stream.remaining()));
        TRY(stream.read_until_filled(local_icc_data));
        icc_data = move(local_icc_data);
    }

    // T.801 JPX extended file format syntax,
    // Table M.22 – Legal METH values
    if (method == 3) {
        ByteBuffer local_icc_data = TRY(ByteBuffer::create_uninitialized(stream.remaining()));
        TRY(stream.read_until_filled(local_icc_data));
        icc_data = move(local_icc_data);
    }
    if (method == 4)
        return Error::from_string_literal("Method 4 is not yet implemented");
    if (method == 5)
        return Error::from_string_literal("Method 5 is not yet implemented");

    return {};
}

void JPEG2000ColorSpecificationBox::dump(String const& prepend) const
{
    Box::dump(prepend);
    outln("{}- method = {}", prepend, method);
    outln("{}- precedence = {}", prepend, precedence);
    outln("{}- approximation = {}", prepend, approximation);
    if (method == 1)
        outln("{}- enumerated_color_space = {}", prepend, enumerated_color_space);
    if (method == 2 || method == 3)
        outln("{}- icc_data = {} bytes", prepend, icc_data.size());
}

ErrorOr<void> JPEG2000PaletteBox::read_from_stream(BoxStream& stream)
{
    // I.5.3.4 Palette box
    u16 number_of_entries = TRY(stream.read_value<BigEndian<u16>>());
    u8 number_of_palette_columns = TRY(stream.read_value<u8>());

    for (u8 i = 0; i < number_of_palette_columns; ++i) {
        // Table I.13 – B^i values
        BitDepth bit_depth;
        u8 depth = TRY(stream.read_value<u8>());
        bit_depth.depth = (depth & 0x7f) + 1;
        bit_depth.is_signed = (depth & 0x80) != 0;
        bit_depths.append(bit_depth);
    }

    for (u16 i = 0; i < number_of_entries; ++i) {
        Color color;
        for (auto const& bit_depth : bit_depths) {
            i64 value = 0;
            for (u8 j = 0; j < ceil_div(bit_depth.depth, static_cast<u8>(8)); ++j) {
                u8 byte = TRY(stream.read_value<u8>());
                value = (value << 8) | byte;
            }

            // Sign extend if needed.
            if (bit_depth.is_signed && (value >> (bit_depth.depth - 1)))
                value |= ~((1LL << bit_depth.depth) - 1);

            color.append(value);
        }
        palette_entries.append(color);
    }

    return {};
}

void JPEG2000PaletteBox::dump(String const& prepend) const
{
    Box::dump(prepend);
    outln("{}- number_of_entries = {}", prepend, palette_entries.size());
    outln("{}- number_of_palette_columns = {}", prepend, bit_depths.size());

    outln("{}- bit_depths", prepend);
    for (auto const& bit_depth : bit_depths) {
        outln("{}  - {}, {}", prepend, bit_depth.depth, bit_depth.is_signed ? "signed" : "unsigned");
    }

    outln("{}- palette_entries", prepend);
    for (auto const& color : palette_entries) {
        out("{}  - ", prepend);
        for (auto value : color) {
            out("{:#x} ", value);
        }
        outln();
    }
}

ErrorOr<void> JPEG2000ChannelDefinitionBox::read_from_stream(BoxStream& stream)
{
    // I.5.3.6 Channel Definition box
    u16 count = TRY(stream.read_value<BigEndian<u16>>());
    for (u32 i = 0; i < count; ++i) {
        Channel channel;
        channel.channel_index = TRY(stream.read_value<BigEndian<u16>>());
        channel.channel_type = TRY(stream.read_value<BigEndian<u16>>());
        channel.channel_association = TRY(stream.read_value<BigEndian<u16>>());
        channels.append(channel);
    }
    return {};
}

void JPEG2000ChannelDefinitionBox::dump(String const& prepend) const
{
    Box::dump(prepend);
    for (auto const& channel : channels) {
        outln("{}- channel_index = {}", prepend, channel.channel_index);

        out("{}- channel_type = {}", prepend, channel.channel_type);
        if (channel.channel_type == 0)
            outln(" (color)");
        else if (channel.channel_type == 1)
            outln(" (opacity)");
        else if (channel.channel_type == 2)
            outln(" (premultiplied opacity)");
        else
            outln(" (unknown)");

        outln("{}- channel_association = {}", prepend, channel.channel_association);
    }
}

ErrorOr<void> JPEG2000ResolutionBox::read_from_stream(BoxStream& stream)
{
    // I.5.3.7 Resolution box (superbox)
    auto make_subbox = [](BoxType type, BoxStream& stream) -> ErrorOr<Optional<NonnullOwnPtr<Box>>> {
        switch (type) {
        case BoxType::JPEG2000CaptureResolutionBox:
            return TRY(JPEG2000CaptureResolutionBox::create_from_stream(stream));
        case BoxType::JPEG2000DefaultDisplayResolutionBox:
            return TRY(JPEG2000DefaultDisplayResolutionBox::create_from_stream(stream));
        default:
            return OptionalNone {};
        }
    };

    TRY(SuperBox::read_from_stream(stream, move(make_subbox)));
    return {};
}

void JPEG2000ResolutionBox::dump(String const& prepend) const
{
    SuperBox::dump(prepend);
}

ErrorOr<void> JPEG2000ResolutionSubboxBase::read_from_stream(BoxStream& stream)
{
    vertical_capture_grid_resolution_numerator = TRY(stream.read_value<BigEndian<u16>>());
    vertical_capture_grid_resolution_denominator = TRY(stream.read_value<BigEndian<u16>>());
    horizontal_capture_grid_resolution_numerator = TRY(stream.read_value<BigEndian<u16>>());
    horizontal_capture_grid_resolution_denominator = TRY(stream.read_value<BigEndian<u16>>());
    vertical_capture_grid_resolution_exponent = TRY(stream.read_value<i8>());
    horizontal_capture_grid_resolution_exponent = TRY(stream.read_value<i8>());
    return {};
}

void JPEG2000ResolutionSubboxBase::dump(String const& prepend) const
{
    Box::dump(prepend);
    outln("{}- vertical_capture_grid_resolution = {}/{} * 10^{}", prepend, vertical_capture_grid_resolution_numerator, vertical_capture_grid_resolution_denominator, vertical_capture_grid_resolution_exponent);
    outln("{}- horizontal_capture_grid_resolution = {}/{} * 10^{}", prepend, horizontal_capture_grid_resolution_numerator, horizontal_capture_grid_resolution_denominator, horizontal_capture_grid_resolution_exponent);
}

ErrorOr<void> JPEG2000CaptureResolutionBox::read_from_stream(BoxStream& stream)
{
    // I.5.3.7.1 Capture Resolution box
    return JPEG2000ResolutionSubboxBase::read_from_stream(stream);
}

void JPEG2000CaptureResolutionBox::dump(String const& prepend) const
{
    JPEG2000ResolutionSubboxBase::dump(prepend);
}

ErrorOr<void> JPEG2000DefaultDisplayResolutionBox::read_from_stream(BoxStream& stream)
{
    // I.5.3.7.2 Default Display Resolution box
    return JPEG2000ResolutionSubboxBase::read_from_stream(stream);
}

void JPEG2000DefaultDisplayResolutionBox::dump(String const& prepend) const
{
    JPEG2000ResolutionSubboxBase::dump(prepend);
}

ErrorOr<void> JPEG2000ContiguousCodestreamBox::read_from_stream(BoxStream& stream)
{
    // I.5.4 Contiguous Codestream box
    // FIXME: It's wasteful to make a copy of all the image data here. Having just a ReadonlyBytes
    // or streaming it into the jpeg2000 decoder would be nicer.
    ByteBuffer local_codestream = TRY(ByteBuffer::create_uninitialized(stream.remaining()));
    TRY(stream.read_until_filled(local_codestream));
    codestream = move(local_codestream);
    return {};
}

void JPEG2000ContiguousCodestreamBox::dump(String const& prepend) const
{
    Box::dump(prepend);
    outln("{}- codestream = {} bytes", prepend, codestream.size());
}

ErrorOr<void> JPEG2000SignatureBox::read_from_stream(BoxStream& stream)
{
    // I.5.1 JPEG 2000 Signature box
    signature = TRY(stream.read_value<BigEndian<u32>>());
    return {};
}

void JPEG2000SignatureBox::dump(String const& prepend) const
{
    Box::dump(prepend);
    outln("{}- signature = {:#08x}", prepend, signature);
}

ErrorOr<void> JPEG2000UUIDInfoBox::read_from_stream(BoxStream& stream)
{
    // I.7.3 UUID Info boxes (superbox)
    auto make_subbox = [](BoxType type, BoxStream& stream) -> ErrorOr<Optional<NonnullOwnPtr<Box>>> {
        switch (type) {
        case BoxType::JPEG2000UUIDListBox:
            return TRY(JPEG2000UUIDListBox::create_from_stream(stream));
        case BoxType::JPEG2000URLBox:
            return TRY(JPEG2000URLBox::create_from_stream(stream));
        default:
            return OptionalNone {};
        }
    };

    TRY(SuperBox::read_from_stream(stream, move(make_subbox)));
    return {};
}

void JPEG2000UUIDInfoBox::dump(String const& prepend) const
{
    SuperBox::dump(prepend);
}

ErrorOr<void> JPEG2000UUIDListBox::read_from_stream(BoxStream& stream)
{
    // I.7.3.1 UUID List box
    u16 count = TRY(stream.read_value<BigEndian<u16>>());
    for (u32 i = 0; i < count; ++i) {
        Array<u8, 16> uuid;
        TRY(stream.read_until_filled(uuid));
        uuids.append(uuid);
    }
    return {};
}

void JPEG2000UUIDListBox::dump(String const& prepend) const
{
    Box::dump(prepend);
    for (auto const& uuid : uuids) {
        out("{}- ", prepend);
        for (auto byte : uuid) {
            out("{:02x}", byte);
        }
        outln();
    }
}

ErrorOr<String> JPEG2000URLBox::url_as_string() const
{
    // Zero-terminated UTF-8 per spec.
    if (url_bytes.is_empty() || url_bytes.bytes().last() != '\0')
        return Error::from_string_literal("URL not zero-terminated");
    return String::from_utf8(StringView { url_bytes.bytes().trim(url_bytes.size() - 1) });
}

ErrorOr<void> JPEG2000URLBox::read_from_stream(BoxStream& stream)
{
    // I.7.3.2 Data Entry URL box
    version_number = TRY(stream.read_value<u8>());
    flag = TRY(stream.read_value<u8>()) << 16;
    flag |= TRY(stream.read_value<BigEndian<u16>>());

    url_bytes = TRY(ByteBuffer::create_uninitialized(stream.remaining()));
    TRY(stream.read_until_filled(url_bytes));

    return {};
}

void JPEG2000URLBox::dump(String const& prepend) const
{
    Box::dump(prepend);
    outln("{}- version_number = {}", prepend, version_number);
    outln("{}- flag = {:#06x}", prepend, flag);

    auto url_or_err = url_as_string();
    if (url_or_err.is_error())
        outln("{}- url = <invalid {}; {} bytes>", prepend, url_or_err.release_error(), url_bytes.size());
    else
        outln("{}- url = {}", prepend, url_or_err.release_value());
}

}
