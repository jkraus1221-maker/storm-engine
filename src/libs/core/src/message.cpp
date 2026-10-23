#pragma once

#include <cstdio>
#include "message.h"

#include <algorithm>

#include <fmt/format.h>

// Read one parameter, tolerating a type mismatch.
//
// The scripts can send an entity a message whose format does not match
// what the receiver expects - a message aimed at a class this engine
// does not have arrives with whatever format the caller used. std::get
// throws bad_variant_access on that, and an uncaught throw ends the
// program. A mismatched parameter now reports and yields a default, so
// one bad message costs a warning instead of the process.
template <typename T>
static T message_param(const storm::MessageParam &value)
{
    if (auto *const typed = std::get_if<T>(&value))
        return *typed;
    std::fprintf(stderr, "message: parameter type does not match the receiver; using a default\n");
    return T{};
}


void MESSAGE::Move2Start()
{
    index = 0;
}

uint8_t MESSAGE::Byte()
{
    ValidateFormat('b');
    return message_param<uint8_t>(params_[index - 1]);
}

uint16_t MESSAGE::Word()
{
    ValidateFormat('w');
    return message_param<uint16_t>(params_[index - 1]);
}

int32_t MESSAGE::Long()
{
    ValidateFormat('l');
    return message_param<int32_t>(params_[index - 1]);
}

int32_t MESSAGE::Dword()
{
    ValidateFormat('u');
    return static_cast<int32_t>(message_param<uint32_t>(params_[index - 1]));
}

float MESSAGE::Float()
{
    ValidateFormat('f');
    return message_param<float>(params_[index - 1]);
}

double MESSAGE::Double()
{
    ValidateFormat('d');
    return message_param<double>(params_[index - 1]);
}

uintptr_t MESSAGE::Pointer()
{
    ValidateFormat('p');
    return static_cast<uintptr_t>(message_param<entid_t>(params_[index - 1]));
}

ATTRIBUTES * MESSAGE::AttributePointer()
{
    ValidateFormat('a');
    return message_param<ATTRIBUTES *>(params_[index - 1]);
}

entid_t MESSAGE::EntityID()
{
    ValidateFormat('i');
    return message_param<entid_t>(params_[index - 1]);
}

VDATA * MESSAGE::ScriptVariablePointer()
{
    ValidateFormat('e');
    return message_param<VDATA *>(params_[index - 1]);
}

CVECTOR MESSAGE::CVector()
{
    ValidateFormat('c');
    return message_param<CVECTOR>(params_[index - 1]);
}

const std::string & MESSAGE::String()
{
    ValidateFormat('s');
    return message_param<std::string>(params_[index - 1]);
}

bool MESSAGE::Set(int32_t value)
{
    ValidateFormat('l');
    params_[index - 1] = value;
    return true;
}

bool MESSAGE::Set(uintptr_t value)
{
    ValidateFormat('p');
    params_[index - 1] = static_cast<entid_t>(value);
    return true;
}

bool MESSAGE::Set(float value)
{
    ValidateFormat('f');
    params_[index - 1] = value;
    return true;
}

bool MESSAGE::Set(std::string value)
{
    ValidateFormat('s');
    params_[index - 1] = std::move(value);
    return true;
}

bool MESSAGE::SetEntity(entid_t value)
{
    ValidateFormat('i');
    params_[index - 1] = value;
    return true;
}

bool MESSAGE::Set(VDATA *value)
{
    ValidateFormat('e');
    params_[index - 1] = value;
    return true;
}

bool MESSAGE::Set(ATTRIBUTES *value)
{
    ValidateFormat('a');
    params_[index - 1] = value;
    return true;
}

void MESSAGE::ValidateFormat(char c)
{
    if (format_.empty())
    {
        // An entity reading a message that was never set up. The
        // scripts can cause this - a message sent to a class this
        // engine does not have arrives with a format the receiver does
        // not expect - and throwing ends the whole program.
        std::fprintf(stderr, "message: read from an empty message\n");
        return;
    }
    if (format_[index] != c)
    {
        std::fprintf(stderr,
                     "message: format mismatch, the message says '%c' and the receiver wanted '%c'"
                     " (format '%s', at %u)\n",
                     format_[index], c, format_.c_str(), static_cast<unsigned>(index));
        index++;
        return;
    }
    index++;
}

void MESSAGE::Reset(const std::string_view &format)
{
    format_ = format;
    params_.resize(format_.size());
    index = 0;
}

void MESSAGE::ResetVA(const std::string_view &format, va_list&args)
{
    index = 0;
    format_ = format;
    params_.resize(format_.size());
    std::transform(format_.begin(), format_.end(), params_.begin(),
                   [&](const char c) { return GetParamValue(c, args); });
}

char MESSAGE::GetCurrentFormatType()
{
    return format_[index];
}

const char * MESSAGE::StringPointer()
{
    return String().c_str();
}

std::string_view MESSAGE::GetFormat() const
{
    return format_;
}

bool MESSAGE::ParamValid() const
{
    return index < format_.length();
}

storm::MessageParam MESSAGE::GetParamValue(const char c, va_list&args)
{
    switch (c)
    {
    case 'b':
        return static_cast<uint8_t>(va_arg(args, int));
    case 'w':
        return static_cast<uint16_t>(va_arg(args, int));
    case 'l':
        return va_arg(args, int32_t);
    case 'u':
        return va_arg(args, uint32_t);
    case 'f':
        return static_cast<float>(va_arg(args, double));
    case 'd':
        return va_arg(args, double);
    case 'p':
        return static_cast<entid_t>(va_arg(args, uintptr_t));
    case 'a':
        return va_arg(args, ATTRIBUTES *);
    case 'i':
        return va_arg(args, entid_t);
    case 'e':
        return va_arg(args, VDATA *);
    case 'c':
        return va_arg(args, CVECTOR);
    case 's': {
        char *ptr = va_arg(args, char *);
        return std::string(ptr);
    }
    default:
        throw std::runtime_error(fmt::format("Unknown message format: '{}'", c));
    }
}
