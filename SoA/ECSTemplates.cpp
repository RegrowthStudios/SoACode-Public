#include "stdafx.h"
#include "ECSTemplates.h"

#include <Vorb/io/IOManager.h>
#include <Vorb/io/Keg.h>

vecs::EntityID ECSTemplate::create(vecs::ECS& ecs) {
    // Create entity.
    auto e = ecs.addEntity();

    // Add all components.
    for(auto& kvp : m_components) kvp.second->m_cID = ecs.addComponent(kvp.first, e);

    // Build all components.
    for(auto& kvp : m_components) kvp.second->build(ecs, e);

    // Run post-build for dependencies and such.
    for (auto& kvp : m_components) kvp.second->postBuild(ecs, e);

    return e;
}

void ECSTemplateLibrary::loadTemplate(const vpath& file) {
    ECSTemplate* t = new ECSTemplate();

    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    { // Parse YAML file
        vio::IOManager iom;
        const cString s = iom.readFileToString(file);
        context.reader.init(s);
        delete[] s;
    }

    {
        vfile f;
        file.asFile(&f);
        nString fileName = file.getLeaf();
        nString templateName = fileName.substr(0, fileName.length() - 4);
        m_templates[templateName] = t;
    }

    auto node = context.reader.getFirst();
    auto f = makeFunctor([&](Sender s, const nString& component, keg::Node node) {
        auto& bb = m_builders.find(component);
        if(bb != m_builders.end()) {
            ECSComponentBuilder* builder = bb->second();
            builder->load(context, node);
            t->m_components[component] = builder;
        }
        context.reader.free(node);
    });
    context.reader.forAllInMap(node, f);
    delete f;

    context.reader.dispose();
}

vecs::EntityID ECSTemplateLibrary::build(vecs::ECS& ecs, const nString& name) const {
    auto& tmpl = m_templates.find(name);
    if(tmpl == m_templates.end()) return ID_GENERATOR_NULL_ID;
    return tmpl->second->create(ecs);
}

ECSTemplateLibrary::~ECSTemplateLibrary() {
    for(auto& kvp : m_templates) {
        delete kvp.second;
    }
}

