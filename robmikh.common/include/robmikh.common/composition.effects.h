#pragma once
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Numerics.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.Graphics.Effects.h>
#include <windows.graphics.effects.interop.h>

#include <map>
#include <tuple>
#include <vector>

// Adapted from Win2D

namespace robmikh::common::uwp
{
    namespace impl
    {
        inline winrt::Windows::Foundation::IPropertyValue CreateProperty(winrt::Windows::Foundation::IInspectable const& value)
        {
            auto inspectable = winrt::Windows::Foundation::PropertyValue::CreateInspectableArray({ value });
            return inspectable.as<winrt::Windows::Foundation::IPropertyValue>();
        }

        template <typename T>
        inline T GetValueOfProperty(winrt::Windows::Foundation::IPropertyValue const& propertyValue)
        {
            static_assert(std::is_base_of<IInspectable, T>::value, "Interface types must be IInspectable");

            winrt::com_array<winrt::Windows::Foundation::IInspectable> array;
            propertyValue.GetInspectableArray(array);

            if (array.size() != 1)
            {
                throw winrt::hresult_not_implemented();
            }

            auto innerValue = array.at(0);
            if (innerValue)
            {
                return innerValue.as<T>();
            }
            return nullptr;
        }

        template<typename TBoxed, typename TPublic, typename Enable = void>
        struct PropertyTypeConverter
        {
            static_assert(std::is_same<TBoxed, TPublic>::value, "Default PropertyTypeConverter should only be used when TBoxed = TPublic");

            static winrt::Windows::Foundation::IPropertyValue Box(TPublic const& value)
            {
                return CreateProperty(value);
            }

            static TPublic Unbox(winrt::Windows::Foundation::IPropertyValue const& value)
            {
                return GetValueOfProperty<TPublic>(value);
            }
        };

        template <int N, typename TPublic>
        struct PropertyTypeConverter<float[N], TPublic>
        {
            static_assert(std::is_same<TPublic, winrt::Windows::Foundation::Numerics::float2>::value ||
                std::is_same<TPublic, winrt::Windows::Foundation::Numerics::float3>::value ||
                std::is_same<TPublic, winrt::Windows::Foundation::Numerics::float4>::value ||
                std::is_same<TPublic, winrt::Windows::Foundation::Numerics::float3x2>::value ||
                std::is_same<TPublic, winrt::Windows::Foundation::Numerics::float4x4>::value, /*||
                std::is_same<TPublic, winrt::Windows::Foundation::float5x4>::value,*/
                "This type cannot be boxed as a float array");

            static_assert(sizeof(TPublic) == sizeof(float[N]), "Wrong array size");

            static winrt::Windows::Foundation::IPropertyValue Box(TPublic const& value)
            {
                auto temp = value;
                return winrt::Windows::Foundation::PropertyValue::CreateSingleArray(reinterpret_cast<std::array<float, N>&>(temp)).as<winrt::Windows::Foundation::IPropertyValue>();
            }

            static TPublic Unbox(winrt::Windows::Foundation::IPropertyValue const& value)
            {
                winrt::com_array<float> array;
                value.GetSingleArray(array);
                if (array.size() != N)
                {
                    throw winrt::hresult_out_of_bounds();
                }
                auto result = *reinterpret_cast<TPublic*>(array.data());
                return result;
            }
        };

        template <>
        struct PropertyTypeConverter<float[4], winrt::Windows::UI::Color>
        {
            using VectorConverter = PropertyTypeConverter<float[4], winrt::Windows::Foundation::Numerics::float4>;

            static winrt::Windows::Foundation::IPropertyValue Box(winrt::Windows::UI::Color const& value)
            {
                winrt::Windows::Foundation::Numerics::float4 input = { value.R / 255.0f, value.G / 255.0f, value.B / 255.0f, value.A / 255.0f };
                return VectorConverter::Box(input);
            }

            static winrt::Windows::UI::Color Unbox(winrt::Windows::Foundation::IPropertyValue const& propertyValue)
            {
                auto value = VectorConverter::Unbox(propertyValue);
                winrt::Windows::UI::Color result = 
                { 
                    static_cast<uint8_t>(value.w * 255.0f), 
                    static_cast<uint8_t>(value.x * 255.0f), 
                    static_cast<uint8_t>(value.y * 255.0f), 
                    static_cast<uint8_t>(value.z * 255.0f) 
                };
                return result;
            }
        };
    }

    template <typename T>
    struct EffectBase : winrt::implements<T,
        winrt::Windows::Graphics::Effects::IGraphicsEffect,
        winrt::Windows::Graphics::Effects::IGraphicsEffectSource,
        ABI::Windows::Graphics::Effects::IGraphicsEffectD2D1Interop>
    {
        using PropertyMappings = std::map<winrt::hstring, std::pair<D2D1_FLOOD_PROP, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING>>;

        EffectBase(size_t propertyCount, size_t sourceCount) : m_properties(propertyCount), m_sources(sourceCount)
        { }

        // IGraphicsEffect
        winrt::hstring Name() noexcept { return m_name; }
        void Name(winrt::hstring const& value) noexcept { m_name = value; }

        // IGraphicsEffectD2D1Interop
        IFACEMETHODIMP GetEffectId(_Out_ GUID* id) 
        {
            *id = EffectId();
            return S_OK;
        }

        IFACEMETHODIMP GetNamedPropertyMapping(
            LPCWSTR name,
            _Out_ UINT* index,
            _Out_ ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING* mapping)
        {
            auto mappings = GetPropertyMappings();
            auto search = mappings.find(name);
            if (search == mappings.end())
            {
                return E_INVALIDARG;
            }

            *index = search->second.first;
            *mapping = search->second.second;

            return S_OK;
        }

        IFACEMETHODIMP GetPropertyCount(_Out_ UINT* count)
        {
            *count = static_cast<UINT>(m_properties.size());
            return S_OK;
        }

        IFACEMETHODIMP GetProperty(
            UINT index,
            _Outptr_ ABI::Windows::Foundation::IPropertyValue** value)
        {
            try
            {
                auto propertyValue = m_properties[index];
                auto abiValue = propertyValue.as<ABI::Windows::Foundation::IPropertyValue>();
                *value = abiValue.detach();
            }
            catch (...)
            {
                return winrt::to_hresult();
            }
            return S_OK;
        }

        IFACEMETHODIMP GetSource(
            UINT index,
            _Outptr_ ABI::Windows::Graphics::Effects::IGraphicsEffectSource** source)
        {
            try
            {
                auto sourceValue = m_sources[index];
                auto abiValue = sourceValue.as<ABI::Windows::Graphics::Effects::IGraphicsEffectSource>();
                *source = abiValue.detach();
            }
            catch (...)
            {
                return winrt::to_hresult();
            }
            return S_OK;
        }

        IFACEMETHODIMP GetSourceCount(_Out_ UINT* count)
        {
            *count = static_cast<UINT>(m_sources.size());
            return S_OK;
        }

        // Effect virtuals
        virtual winrt::guid EffectId() noexcept = 0;
        virtual PropertyMappings const& GetPropertyMappings() noexcept = 0;

        // Helpers
        template <typename TBoxed, typename TPublic>
        void SetBoxedProperty(uint32_t index, TPublic const& value)
        {
            auto boxedValue = impl::PropertyTypeConverter<TBoxed, TPublic>::Box(value);
            m_properties[index] = boxedValue;
        }

        template <typename TBoxed, typename TPublic>
        TPublic GetBoxedProperty(uint32_t index)
        {
            auto boxedValue = m_properties[index];
            return impl::PropertyTypeConverter<TBoxed, TPublic>::Unbox(boxedValue);
        }

    protected:
        winrt::hstring m_name;
        // TODO: I shouldn't be using the impl namespace from winrt...
        std::vector<winrt::impl::com_ref<winrt::Windows::Foundation::IPropertyValue>> m_properties;
        std::vector<winrt::impl::com_ref<winrt::Windows::Graphics::Effects::IGraphicsEffectSource>> m_sources;
    };

    struct ColorSourceEffect : EffectBase<ColorSourceEffect>
    {
        ColorSourceEffect() : EffectBase(1, 0) 
        {
            Color({ 255, 0, 0, 0 });
        }

        winrt::guid EffectId() noexcept override { return CLSID_D2D1Flood; }
        EffectBase::PropertyMappings const& GetPropertyMappings() noexcept override
        {
            static EffectBase::PropertyMappings mappings =
            {
                { L"Color", { D2D1_FLOOD_PROP_COLOR, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_COLOR_TO_VECTOR4 } },
                { L"ColorHdr", { D2D1_FLOOD_PROP_COLOR, ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_UNKNOWN } },
            };
            return mappings;
        }

        winrt::Windows::UI::Color Color() { return GetBoxedProperty<float[4], winrt::Windows::UI::Color>(D2D1_FLOOD_PROP_COLOR); }
        void Color(winrt::Windows::UI::Color const& value) { SetBoxedProperty<float[4], winrt::Windows::UI::Color>(D2D1_FLOOD_PROP_COLOR, value); }

        winrt::Windows::Foundation::Numerics::float4 ColorHdr() { return GetBoxedProperty<float[4], winrt::Windows::Foundation::Numerics::float4>(D2D1_FLOOD_PROP_COLOR); }
        void ColorHdr(winrt::Windows::Foundation::Numerics::float4 const& value) { SetBoxedProperty<float[4], winrt::Windows::Foundation::Numerics::float4>(D2D1_FLOOD_PROP_COLOR, value); }
    };
}