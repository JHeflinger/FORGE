#pragma once
#include "Safety.h"
#include "../Utils/YamlUtils.h"

class Editor;
class Simulation;

class Serializer {
public:
    static std::string SerializeEditor(Ref<Editor>& editor);
    static std::string SerializeSimulation(Ref<Simulation>& simulation);
    static bool DeserializeEditor(Ref<Editor>& editor, const std::string& data);
    static bool DeserializeSimulation(Ref<Simulation>& simulation, const std::string& data);
};