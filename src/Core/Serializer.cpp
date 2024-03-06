#include "Serializer.h"
#include "../Utils/FileUtils.h"
#include "../Core/Editor.h"
#include "../Core/Simulation.h"

static std::string SerializeEditor(Ref<Editor>& editor) {
    YAML::Emitter out;

    return std::string(out.c_str());
}

static std::string SerializeSimulation(Ref<Simulation>& simulation) {
    YAML::Emitter out;

    return std::string(out.c_str());
}

static bool DeserializeEditor(Ref<Editor>& editor, const std::string& data) {

    return true;
}

static bool DeserializeSimulation(Ref<Simulation>& simulation, const std::string& data) {

    return true;
}