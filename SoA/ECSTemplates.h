//
// ECSTemplates.h
//
// Created by Cristian Zaloj on 16 Mar 2015
//

#pragma once

#ifndef ECSTemplates_h__
#define ECSTemplates_h__

#include <Vorb/Event.hpp>
#include <Vorb/IO.h>
#include <Vorb/io/Keg.h>
#include <Vorb/ecs/ECS.h>

class ECSTemplate;
class ECSTemplateLibrary;

class ECSComponentBuilder {
    friend class ECSTemplate;
public:
    virtual ~ECSComponentBuilder() {
        // Empty
    }

    virtual void load(keg::ReadContext& reader, keg::Node node) = 0;
    virtual void build(vecs::ECS& ecs, vecs::EntityID eID) = 0;
    virtual void postBuild(vecs::ECS& ecs VORB_UNUSED, vecs::EntityID eID VORB_UNUSED) { /* Empty */ };
protected:
    vecs::ComponentID m_cID; ///< ID of generated component
};

class ECSTemplate {
    friend class ECSTemplateLibrary;
public:
    virtual ~ECSTemplate() {
        for(auto& kvp : m_components) delete kvp.second;
    }

    vecs::EntityID create(vecs::ECS& ecs);
private:
    std::unordered_map<nString, ECSComponentBuilder*> m_components;
};

class ECSTemplateLibrary {
    typedef Delegate<ECSComponentBuilder*> ComponentBuildFunctionFactory;
public:
    virtual ~ECSTemplateLibrary();

    vecs::EntityID build(vecs::ECS& ecs, const nString& name) const;

    void loadTemplate(const vpath& file);

    template<typename T>
    void registerFactory(const nString& component) {
        auto func=[]() -> ECSComponentBuilder* {
            return new T();
        };

        m_builders[component] = makeDelegate(&func);
    }

    template<typename F>
    void forEachTemplate(F f) {
        for(auto& kvp : m_templates) f(kvp.first);
    }
private:
    std::unordered_map<nString, ECSTemplate*> m_templates;
    std::unordered_map<nString, ComponentBuildFunctionFactory> m_builders;
};

#endif // !ECSTemplates_h__
