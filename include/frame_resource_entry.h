#pragma once

#include "pch.h"

// http://www.cplusplus.com/articles/oz18T05o/
// https://www.modernescpp.com/index.php/c-core-guidelines-type-erasure-with-templates

// Wrapper around a virtual resource
class ResourceEntry final {
    friend class FrameGraph;

public:
    ResourceEntry() = delete;
    ResourceEntry(const ResourceEntry&) = delete;
    ResourceEntry(ResourceEntry&&) noexcept = default;

    ResourceEntry& operator=(const ResourceEntry&) = delete;
    ResourceEntry& operator=(ResourceEntry&&) noexcept = default;

    [[nodiscard]] std::string toString() const;

    void create(void* allocator);
    void destroy(void* allocator);

    [[nodiscard]] uint32_t getVersion() const;
    [[nodiscard]] bool isImported() const;
    [[nodiscard]] bool isTransient() const;

    template <typename T> [[nodiscard]] T& get();
    template <typename T>
    [[nodiscard]] typename const T::Desc& getDescriptor() const;

private:
    template <typename T>
    ResourceEntry(uint32_t id, typename T::Desc&&, T&&, uint32_t version,
        bool imported = false);

    struct Concept {
        virtual ~Concept() = default;

        virtual void create(void*) = 0;
        virtual void destroy(void*) = 0;

        virtual std::string toString() const = 0;
    };
    template <typename T> struct Model : Concept {
        Model(typename T::Desc&&, T&&);

        void create(void* allocator) final;
        void destroy(void* allocator) final;

        std::string toString() const final;

        typename const T::Desc descriptor;
        T resource;
    };

    template <typename T> [[nodiscard]] auto* _getModel() const;

private:
    const uint32_t m_id;
    std::unique_ptr<Concept> m_concept;
    uint32_t m_version;    // Incremented on each (unique) write declaration
    const bool m_imported; // Imported or transient

    PassNode* m_producer{ nullptr };
    PassNode* m_last{ nullptr };
};