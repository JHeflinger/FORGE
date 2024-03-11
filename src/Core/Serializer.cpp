#include "Serializer.h"
#include "../Utils/FileUtils.h"
#include "../Core/Editor.h"
#include "../Core/Simulation.h"

std::string Serializer::SerializeEditor(Ref<Editor>& editor) {
    YAML::Emitter out;
    out << YAML::BeginMap;

    out << YAML::Key << "Camera Settings" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "Type" << YAML::Value << static_cast<int>(editor->GetCamera()->GetProperties().Type);
    out << YAML::Key << "Position" << YAML::Value << editor->GetCamera()->GetProperties().Position;
    out << YAML::Key << "Focal Point" << YAML::Value << editor->GetCamera()->GetProperties().FocalPoint;
    out << YAML::Key << "FOV" << YAML::Value << editor->GetCamera()->GetProperties().FOV;
    out << YAML::Key << "NearClip" << YAML::Value << editor->GetCamera()->GetProperties().NearClip;
    out << YAML::Key << "FarClip" << YAML::Value << editor->GetCamera()->GetProperties().FarClip;
    out << YAML::Key << "OrthographicSize" << YAML::Value << editor->GetCamera()->GetProperties().OrthographicSize;
    out << YAML::Key << "OrthographicNear" << YAML::Value << editor->GetCamera()->GetProperties().OrthographicNear;
    out << YAML::Key << "OrthographicFar" << YAML::Value << editor->GetCamera()->GetProperties().OrthographicFar;
    out << YAML::Key << "Pitch" << YAML::Value << editor->GetCamera()->GetProperties().Pitch;
    out << YAML::Key << "Yaw" << YAML::Value << editor->GetCamera()->GetProperties().Yaw;
    out << YAML::Key << "Distance" << YAML::Value << editor->GetCamera()->GetProperties().Distance;
    out << YAML::EndMap;

    out << YAML::Key << "Panels" << YAML::Value;
    out << YAML::BeginMap;
    std::vector<Ref<Panel>> panels = editor->GetPanels();
    for (auto panel : panels)
        out << YAML::Key << panel->Name() << YAML::Value << panel->m_Enabled;
    out << YAML::EndMap;

    out << YAML::EndMap;
    return std::string(out.c_str());
}

std::string Serializer::SerializeSimulation(Ref<Simulation>& simulation) {
    YAML::Emitter out;

    return std::string(out.c_str());
}

bool Serializer::DeserializeEditor(Ref<Editor>& editor, const std::string& data) {
    YAML::Node yamldata = YAML::Load(data);
    
    // Camera deserialization
    if (yamldata["Camera Settings"]) {
        CameraProperties cameraprops = editor->GetCamera()->GetProperties();
        if (yamldata["Camera Settings"]["Type"])
            cameraprops.Type = static_cast<CameraTypes>(yamldata["Camera Settings"]["Type"].as<int>());
        else WARN("Camera type not found in editor settings!");
        if (yamldata["Camera Settings"]["Position"])
            cameraprops.Position = yamldata["Camera Settings"]["Position"].as<glm::vec3>();
        else WARN("Camera position not found in editor settings!");
        if (yamldata["Camera Settings"]["Focal Point"])
            cameraprops.FocalPoint = yamldata["Camera Settings"]["Focal Point"].as<glm::vec3>();
        else WARN("Camera focal point not found in editor settings!");
        if (yamldata["Camera Settings"]["FOV"])
            cameraprops.FOV = yamldata["Camera Settings"]["FOV"].as<float>();
        else WARN("Camera FOV not found in editor settings!");
        if (yamldata["Camera Settings"]["NearClip"])
            cameraprops.NearClip = yamldata["Camera Settings"]["NearClip"].as<float>();
        else WARN("Camera NearClip not found in editor settings!");
        if (yamldata["Camera Settings"]["FarClip"])
            cameraprops.FarClip = yamldata["Camera Settings"]["FarClip"].as<float>();
        else WARN("Camera FarClip not found in editor settings!");
        if (yamldata["Camera Settings"]["OrthographicSize"])
            cameraprops.OrthographicSize = yamldata["Camera Settings"]["OrthographicSize"].as<float>();
        else WARN("Camera OrthographicSize not found in editor settings!");
        if (yamldata["Camera Settings"]["OrthographicNear"])
            cameraprops.OrthographicNear = yamldata["Camera Settings"]["OrthographicNear"].as<float>();
        else WARN("Camera OrthographicNear not found in editor settings!");
        if (yamldata["Camera Settings"]["OrthographicFar"])
            cameraprops.OrthographicFar = yamldata["Camera Settings"]["OrthographicFar"].as<float>();
        else WARN("Camera OrthographicFar not found in editor settings!");
        if (yamldata["Camera Settings"]["Pitch"])
            cameraprops.Pitch = yamldata["Camera Settings"]["Pitch"].as<float>();
        else WARN("Camera Pitch not found in editor settings!");
        if (yamldata["Camera Settings"]["Yaw"])
            cameraprops.Yaw = yamldata["Camera Settings"]["Yaw"].as<float>();
        else WARN("Camera Yaw not found in editor settings!");
        if (yamldata["Camera Settings"]["Distance"])
            cameraprops.Distance = yamldata["Camera Settings"]["Distance"].as<float>();
        else WARN("Camera Distance not found in editor settings!");
        editor->GetCamera()->SetProperties(cameraprops);
    } else WARN("Camera properties not found in editor settings!");

    // Panel deserialization
    if (yamldata["Panels"]) {
        std::vector<Ref<Panel>> panels = editor->GetPanels();
        for (auto panel : panels) {
            if (yamldata["Panels"][panel->Name()]) {
                panel->m_Enabled =yamldata["Panels"][panel->Name()].as<bool>();
            } else WARN("\"{}\" panel setting not detected!", panel->Name());
        }
    } else WARN("panel settings not found in editor settings!");

    return true;
}

bool Serializer::DeserializeSimulation(Ref<Simulation>& simulation, const std::string& data) {

    return true;
}