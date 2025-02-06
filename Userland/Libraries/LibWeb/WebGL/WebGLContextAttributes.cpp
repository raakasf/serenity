/*
 * Copyright (c) 2022, Luke Wilde <lukew@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibJS/Runtime/Completion.h>
#include <LibJS/Runtime/GlobalObject.h>
#include <LibJS/Runtime/ValueInlines.h>
#include <LibWeb/WebGL/WebGLContextAttributes.h>

namespace Web::WebGL {

JS::ThrowCompletionOr<WebGLContextAttributes> convert_value_to_context_attributes_dictionary(JS::VM& vm, JS::Value value)
{
    // NOTE: This code was generated by the IDL code generator and then cleaned up.
    if (!value.is_nullish() && !value.is_object())
        return vm.throw_completion<JS::TypeError>(JS::ErrorType::NotAnObjectOfType, "WebGLContextAttributes");

    WebGLContextAttributes context_attributes {};

    JS::Value alpha;
    if (value.is_nullish())
        alpha = JS::js_undefined();
    else
        alpha = TRY(value.as_object().get("alpha"));

    bool alpha_value;
    if (!alpha.is_undefined())
        alpha_value = alpha.to_boolean();
    else
        alpha_value = true;

    context_attributes.alpha = alpha_value;

    JS::Value antialias;
    if (value.is_nullish())
        antialias = JS::js_undefined();
    else
        antialias = TRY(value.as_object().get("antialias"));

    bool antialias_value;
    if (!antialias.is_undefined())
        antialias_value = antialias.to_boolean();
    else
        antialias_value = true;

    context_attributes.antialias = antialias_value;

    JS::Value depth;
    if (value.is_nullish())
        depth = JS::js_undefined();
    else
        depth = TRY(value.as_object().get("depth"));

    bool depth_value;
    if (!depth.is_undefined())
        depth_value = depth.to_boolean();
    else
        depth_value = true;

    context_attributes.depth = depth_value;

    JS::Value desynchronized;
    if (value.is_nullish())
        desynchronized = JS::js_undefined();
    else
        desynchronized = TRY(value.as_object().get("desynchronized"));

    bool desynchronized_value;

    if (!desynchronized.is_undefined())
        desynchronized_value = desynchronized.to_boolean();
    else
        desynchronized_value = false;

    context_attributes.desynchronized = desynchronized_value;

    JS::Value fail_if_major_performance_caveat;
    if (value.is_nullish())
        fail_if_major_performance_caveat = JS::js_undefined();
    else
        fail_if_major_performance_caveat = TRY(value.as_object().get("failIfMajorPerformanceCaveat"));

    bool fail_if_major_performance_caveat_value;
    if (!fail_if_major_performance_caveat.is_undefined())
        fail_if_major_performance_caveat_value = fail_if_major_performance_caveat.to_boolean();
    else
        fail_if_major_performance_caveat_value = false;

    context_attributes.fail_if_major_performance_caveat = fail_if_major_performance_caveat_value;

    JS::Value power_preference;
    if (value.is_nullish())
        power_preference = JS::js_undefined();
    else
        power_preference = TRY(value.as_object().get("powerPreference"));

    WebGLPowerPreference power_preference_value { WebGLPowerPreference::Default };

    if (!power_preference.is_undefined()) {
        auto power_preference_string = TRY(power_preference.to_string(vm));

        if (power_preference_string == "high-performance"sv)
            power_preference_value = WebGLPowerPreference::HighPerformance;
        else if (power_preference_string == "low-power"sv)
            power_preference_value = WebGLPowerPreference::LowPower;
        else if (power_preference_string == "default"sv)
            power_preference_value = WebGLPowerPreference::Default;
        else
            return vm.throw_completion<JS::TypeError>(JS::ErrorType::InvalidEnumerationValue, power_preference_string, "WebGLPowerPreference");
    }

    context_attributes.power_preference = power_preference_value;

    JS::Value premultiplied_alpha;
    if (value.is_nullish())
        premultiplied_alpha = JS::js_undefined();
    else
        premultiplied_alpha = TRY(value.as_object().get("premultipliedAlpha"));

    bool premultiplied_alpha_value;

    if (!premultiplied_alpha.is_undefined())
        premultiplied_alpha_value = premultiplied_alpha.to_boolean();
    else
        premultiplied_alpha_value = true;

    context_attributes.premultiplied_alpha = premultiplied_alpha_value;

    JS::Value preserve_drawing_buffer;
    if (value.is_nullish())
        preserve_drawing_buffer = JS::js_undefined();
    else
        preserve_drawing_buffer = TRY(value.as_object().get("preserveDrawingBuffer"));

    bool preserve_drawing_buffer_value;
    if (!preserve_drawing_buffer.is_undefined())
        preserve_drawing_buffer_value = preserve_drawing_buffer.to_boolean();
    else
        preserve_drawing_buffer_value = false;

    context_attributes.preserve_drawing_buffer = preserve_drawing_buffer_value;

    JS::Value stencil;
    if (value.is_nullish())
        stencil = JS::js_undefined();
    else
        stencil = TRY(value.as_object().get("stencil"));

    bool stencil_value;

    if (!stencil.is_undefined())
        stencil_value = stencil.to_boolean();
    else
        stencil_value = false;

    context_attributes.stencil = stencil_value;

    return context_attributes;
}

}
