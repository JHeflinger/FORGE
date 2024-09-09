#pragma once
#include "Core/Safety.h"
#include "Utils/YamlUtils.h"

class Editor;
class Simulation;

class Serializer {
public:
    static std::string SerializeEditor(const Ref<Editor> editor);
    static std::string SerializeSimulation(const Ref<Simulation> simulation);
    static bool DeserializeEditor(Ref<Editor> editor, const std::string& data);
    static bool DeserializeSimulation(Ref<Simulation> simulation, const std::string& filepath);
    static bool SimulationSaved(const Ref<Simulation> simulation);
};
